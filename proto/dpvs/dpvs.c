/*
 * Copyright 2016 yubo. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "nest/bird.h"
#include "nest/iface.h"
#include "nest/protocol.h"
#include "nest/iface.h"
#include "nest/route.h"
#include "lib/krt.h"
#include "lib/timer.h"
#include "conf/conf.h"
#include "filter/filter.h"
#include "lib/string.h"
#include "lib/alloca.h"
#include "nest/rt-dev.h"

#include "dpvs.h"



struct dpvs_config *dpvs_conf; 
static struct timer *dpvs_timer;

static void dpvs_dump(struct proto *p);
static void dpvs_dump_rt(struct dpvs_route *r);

static struct iface *dpvs_install_link(struct dpvs_iface *i)
{
	struct iface f, *ifi;
	unsigned index = i->port;

	ifi = if_find_by_index(index);
	if (!ifi) {
		bzero(&f, sizeof(f));
		bsnprintf(f.name, sizeof(f.name) - 1, "port%d", i->port);
		f.mtu = 1500;
		f.index = index;
		// IF_MULTICAST for PtP 
		f.flags = IF_LINK_UP | IF_ADMIN_UP | IF_MULTICAST | IF_BROADCAST | IF_MULTIACCESS;
		ifi = if_update(&f);
		debug("install dpvs ifaces: %s mtu: %d\n", f.name, f.mtu);
	}
	return ifi;
}

static void dpvs_install_addr(struct proto *p,
		struct dpvs_iface *i, struct iface *ifi)
{
	struct ifa ifa;
	int scope;
	int new = 1;

	bzero(&ifa, sizeof(ifa));
	ifa.iface = ifi;
	/*if (i->ifa_flags & IFA_F_SECONDARY)
		ifa.flags |= IA_SECONDARY;*/

	/* IFA_LOCAL can be unset for IPv6 interfaces */
	memcpy(&ifa.ip, &i->ip, sizeof(ifa.ip));
	/*ipa_ntoh(ifa.ip);*/
	ifa.pxlen = i->pxlen;
	if (i->pxlen > BITS_PER_IP_ADDRESS) {
		log(L_ERR "KIF: Invalid prefix length for interface %s: %d",
		    ifi->name, i->pxlen);
		new = 0;
	}
	if (i->pxlen == BITS_PER_IP_ADDRESS) {
		ip_addr addr;
		memcpy(&addr, &i->ip, sizeof(addr));
		/*ipa_ntoh(addr);*/
		ifa.prefix = ifa.brd = addr;

		/* It is either a host address or a peer address */
		if (ipa_equal(ifa.ip, addr))
			ifa.flags |= IA_HOST;
		else {
			ifa.flags |= IA_PEER;
			ifa.opposite = addr;
		}
	} else {
		ip_addr netmask = ipa_mkmask(ifa.pxlen);
		ifa.prefix = ipa_and(ifa.ip, netmask);
		ifa.brd = ipa_or(ifa.ip, ipa_not(netmask));
		if (i->pxlen == BITS_PER_IP_ADDRESS - 1)
			ifa.opposite = ipa_opposite_m1(ifa.ip);

#ifndef IPV6
		if (i->pxlen == BITS_PER_IP_ADDRESS - 2)
			ifa.opposite = ipa_opposite_m2(ifa.ip);
/*
		if ((ifi->flags & IF_BROADCAST) && a[IFA_BROADCAST]) {
			ip_addr xbrd;
			memcpy(&xbrd, RTA_DATA(a[IFA_BROADCAST]), sizeof(xbrd));
			ipa_ntoh(xbrd);
			if (ipa_equal(xbrd, ifa.prefix)
			    || ipa_equal(xbrd, ifa.brd))
				ifa.brd = xbrd;
			else if (ifi->flags & IF_TMP_DOWN)
				log(L_ERR "KIF: Invalid broadcast address %I "
						"for %s", xbrd, ifi->name);
		}*/
#endif
	}

	scope = ipa_classify(ifa.ip);
	if (scope < 0) {
		log(L_ERR "KIF: Invalid interface address %I for %s", ifa.ip,
		    ifi->name);
		return;
	}
	ifa.scope = scope & IADDR_SCOPE_MASK;

	DBG("KIF: IF%d(%s): %s IPA %I, flg %x, struct network %I/%d, "
			"brd %I, opp %I\n", ifi->index,
			ifi->name, new ? "added" : "removed",
			ifa.ip, ifa.flags, ifa.prefix, ifa.pxlen,
			ifa.brd, ifa.opposite);


	if (new){
		ifa_update(&ifa);
	} else {
		ifa_delete(&ifa);
	}
}

/* see also: ospf_rt_spf */
static void dpvs_install_route(struct proto *p, struct dpvs_route *r)
{
	struct network *n;
	struct rta a;
	struct rte *e;
	unsigned index = r->port; 

	if (r->installed > 0)
		return;

	DBG("Installing dpvs route %I/%d, rtd=%d\n",
			r->net, r->masklen, r->dest);
	bzero(&a, sizeof(a));
	a.src = p->main_source;
	a.source = RTS_DPVS;
	a.scope = SCOPE_UNIVERSE;
	a.cast = RTC_UNICAST;
	a.dest = r->dest;
	a.gw = r->tip;

	a.iface = if_find_by_index(index);
	if (!a.iface){
		log(L_ERR "can not found port %d", r->port);
		return;
	}

	/* We skip rta_lookup() here */

	n = net_get(p->table, r->net, r->masklen);
	e = rte_get_temp(&a);
	e->net = n;
	e->pflags = 0;
	e->u.dpvs.rt = r;


	rte_update(p, n, e);
	r->installed = 1;

	/* call libdpvs */

}

static void dpvs_setup(struct timer *t UNUSED)
{
	struct proto *p = t->data;
	struct dpvs_config *c = (void *)p->cf;
	static int init;
	struct dpvs_iface *i;
	struct dpvs_route *r;
	struct iface *ifi;
	/*struct dpvs_proto *p = t->data;*/

	dpvs_dump(p);

	if (init) 
		return;


	if_start_update();
	list_for_each_entry(i, &c->ifaces, n){
		/* install interface */
		ifi = dpvs_install_link(i);
		/* install interface address */
		dpvs_install_addr(p, i, ifi);
	}
	if_end_update();

	/* setup route */
	list_for_each_entry(r, &c->iface_routes, n){
		dpvs_install_route(p, r);
		dpvs_dump_rt(r);
	}
	init = 1;
}

static int dpvs_start(struct proto *p)
{
	/*struct dpvs_config *cf = (void *)p->cf;
	struct dpvs_proto *p = (struct dpvs_proto *)P;
	struct dpvs_route *r;*/

	DBG("dpvs: dpvs_start!\n");

	dpvs_timer = tm_new_set(p->pool, dpvs_setup, p, 0, 0);
	tm_start(dpvs_timer, 1);

	/* We have to go UP before routes could be installed */
	/*proto_notify_state(p, PS_UP);*/

	/* we should install if/ifa/route after 
	 * all proto_start be called , that
	 * make sure all proto's if_notify/ifanotify/rt_notify 
	 * can be called */

	return PS_UP;
}

static void dpvs_dump_rt(struct dpvs_route *r)
{
	debug("%-1I/%2d: sip %I via %I dev port%d\n",
			r->net, r->masklen, r->sip, r->tip, r->port);
}

static void dpvs_dump_if(struct dpvs_iface *i)
{
	debug("port%d %I, network %I/%-2d bc %I\n",
			i->port, i->ip, i->prefix, i->pxlen, i->brd);
}

static void dpvs_dump(struct proto *p)
{
	struct dpvs_config *c = (void *)p->cf;
	struct dpvs_iface *i;
	struct dpvs_route *r;

	debug("dpvs ifaces:\n");
	list_for_each_entry(i, &c->ifaces, n)
	    dpvs_dump_if(i);

	debug("dpvs routes:\n");
	list_for_each_entry(r, &c->iface_routes, n)
	    dpvs_dump_rt(r);
}

/*
 * get lpm and add/remove to dpvs rte lpm table
 */
static void dpvs_rt_notify(struct proto *P, struct rtable * tbl UNUSED,
		struct network * n, struct rte * new,
		struct rte * old UNUSED, struct ea_list * ea)
{
}

static int dpvs_reload_routes(struct proto *P)
{
	struct dpvs_proto *p = (struct dpvs_proto *)P;

	if (p->calcrt != 2)
		DPVS_TRACE(D_EVENTS,
			   "Scheduling routing table calculation with route reload");

	p->calcrt = 2;

	return 1;
}

static int dpvs_rte_same(struct rte *new, struct rte *old)
{
	/* new->attrs == old->attrs always */
	return new->u.dpvs.rt == old->u.dpvs.rt;
}

void dpvs_init_config(struct dpvs_config *c)
{
	dpvs_conf = c;
	INIT_LIST_HEAD(&c->iface_routes);
	INIT_LIST_HEAD(&c->ifaces);
}

static struct proto *dpvs_init(struct proto_config *c)
{
	struct proto *P = proto_new(c, sizeof(struct dpvs_proto));

	P->rt_notify = dpvs_rt_notify;
	P->reload_routes = dpvs_reload_routes;
	P->rte_same = dpvs_rte_same;
	kif_flag |= KIF_F_SCAN_DISABLE;

/*
	p->neigh_notify = dpvs_neigh_notify;
	p->if_notify = dpvs_if_notify;
	p->rte_mergable = dpvs_rte_mergable;
*/
	return P;
}

static void dpvs_copy_routes(struct list_head *dlst, struct list_head *slst)
{
	struct dpvs_route *dr, *sr;

	INIT_LIST_HEAD(dlst);
	list_for_each_entry(sr, slst, n) {
		/* copy one route */
		dr = cfg_alloc(sizeof(struct dpvs_route));
		memcpy(dr, sr, sizeof(struct dpvs_route));

		/* This fn is supposed to be called on fresh src routes, which have 'live'
		   fields (like .chain, .neigh or .installed) zero, so no need to zero them */

		list_add_tail((struct list_head *)dr, dlst);
	}
}

static void dpvs_copy_ifaces(struct list_head *dlst, struct list_head *slst)
{
	struct dpvs_iface *di, *si;

	INIT_LIST_HEAD(dlst);
	list_for_each_entry(si, slst, n) {
		/* copy one iface */
		di = cfg_alloc(sizeof(struct dpvs_iface));
		memcpy(di, si, sizeof(struct dpvs_iface));

		list_add_tail((struct list_head *)di, dlst);
	}
}

static void
dpvs_copy_config(struct proto_config *dest, struct proto_config *src)
{
	struct dpvs_config *d = (struct dpvs_config *)dest;
	struct dpvs_config *s = (struct dpvs_config *)src;

	/* Shallow copy of everything */
	proto_copy_rest(dest, src, sizeof(struct dpvs_config));

	/* Copy route lists */
	dpvs_copy_routes(&d->iface_routes, &s->iface_routes);
	dpvs_copy_ifaces(&d->ifaces, &s->ifaces);
}

struct protocol proto_dpvs = {
	.name = "DPVS",
	.template = "dpvs%d",
	.preference = DEF_PREF_DPVS,
	.config_size = sizeof(struct dpvs_config),
	.init = dpvs_init,
	.dump = dpvs_dump,
	.start = dpvs_start,
	.copy_config = dpvs_copy_config
/*
	.reconfigure = dpvs_reconfigure,
	.shutdown = dpvs_shutdown,
	.cleanup = dpvs_cleanup,
*/
};

