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

#include "dpdk.h"



struct dpdk_config *dpdk_conf; 
static struct timer *dpdk_timer;

static void dpdk_dump(struct proto *p);
static void dpdk_dump_rt(struct dpdk_route *r);

static struct iface *dpdk_install_link(struct dpdk_iface *i)
{
	struct iface f, *ifi;
	unsigned index = i->port|DPDK_PORT_FLAG;

	ifi = if_find_by_index(index);
	if (!ifi) {
		bzero(&f, sizeof(f));
		bsnprintf(f.name, sizeof(f.name) - 1, "port%d", i->port);
		f.mtu = 1500;
		f.index = index;
		// IF_MULTICAST for PtP 
		f.flags = IF_LINK_UP | IF_ADMIN_UP | IF_MULTICAST | IF_BROADCAST | IF_MULTIACCESS;
		ifi = if_update(&f);
		debug("install dpdk ifaces: %s mtu: %d\n", f.name, f.mtu);
	}
	return ifi;
}

static void dpdk_install_addr(struct proto *p,
		struct dpdk_iface *i, struct iface *ifi)
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
			"brd %I, opp %I\n", ifi->index & DPDK_PORT_MASK,
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
static void dpdk_install_route(struct proto *p, struct dpdk_route *r)
{
	struct network *n;
	struct rta a;
	struct rte *e;
	unsigned index = r->via_if | DPDK_PORT_FLAG; 

	if (r->installed > 0)
		return;

	DBG("Installing dpdk route %I/%d, rtd=%d\n",
			r->net, r->masklen, r->dest);
	bzero(&a, sizeof(a));
	a.src = p->main_source;
	a.source = RTS_DPDK;
	a.scope = SCOPE_UNIVERSE;
	a.cast = RTC_UNICAST;
	a.dest = r->dest;
	a.gw = r->tip;

	a.iface = if_find_by_index(index);
	if (!a.iface){
		log(L_ERR "can not found port %d", r->via_if);
		return;
	}

	/* We skip rta_lookup() here */

	n = net_get(p->table, r->net, r->masklen);
	e = rte_get_temp(&a);
	e->net = n;
	e->pflags = 0;
	e->u.dpdk.rt = r;


	rte_update(p, n, e);
	r->installed = 1;

	/* call libdpdk */

}

static void dpdk_setup(struct timer *t UNUSED)
{
	struct proto *p = t->data;
	struct dpdk_config *c = (void *)p->cf;
	static int init;
	struct dpdk_iface *i;
	struct dpdk_route *r;
	struct iface *ifi;
	/*struct dpdk_proto *p = t->data;*/

	dpdk_dump(p);

	if (init) 
		return;


	if_start_update();
	list_for_each_entry(i, &c->ifaces, n){
		/* install interface */
		ifi = dpdk_install_link(i);
		/* install interface address */
		dpdk_install_addr(p, i, ifi);
	}
	if_end_update();

	/* setup route */
	list_for_each_entry(r, &c->iface_routes, n){
		dpdk_install_route(p, r);
		dpdk_dump_rt(r);
	}
	init = 1;
}

static int dpdk_start(struct proto *p)
{
	/*struct dpdk_config *cf = (void *)p->cf;
	struct dpdk_proto *p = (struct dpdk_proto *)P;
	struct dpdk_route *r;*/

	DBG("dpdk: dpdk_start!\n");

	dpdk_timer = tm_new_set(p->pool, dpdk_setup, p, 0, 0);
	tm_start(dpdk_timer, 1);

	/* We have to go UP before routes could be installed */
	/*proto_notify_state(p, PS_UP);*/

	/* we should install if/ifa/route after 
	 * all proto_start be called , that
	 * make sure all proto's if_notify/ifanotify/rt_notify 
	 * can be called */

	return PS_UP;
}

static void dpdk_dump_rt(struct dpdk_route *r)
{
	debug("%-1I/%2d: sip %I via %I dev port%d\n",
			r->net, r->masklen, r->sip, r->tip, r->via_if);
}

static void dpdk_dump_if(struct dpdk_iface *i)
{
	debug("port%d %I, network %I/%-2d bc %I\n",
			i->port, i->ip, i->prefix, i->pxlen, i->brd);
}

static void dpdk_dump(struct proto *p)
{
	struct dpdk_config *c = (void *)p->cf;
	struct dpdk_iface *i;
	struct dpdk_route *r;

	debug("dpdk ifaces:\n");
	list_for_each_entry(i, &c->ifaces, n)
	    dpdk_dump_if(i);

	debug("dpdk routes:\n");
	list_for_each_entry(r, &c->iface_routes, n)
	    dpdk_dump_rt(r);
}

/*
 * get lpm and add/remove to dpdk rte lpm table
 */
static void dpdk_rt_notify(struct proto *P, struct rtable * tbl UNUSED,
		struct network * n, struct rte * new,
		struct rte * old UNUSED, struct ea_list * ea)
{
}

static int dpdk_reload_routes(struct proto *P)
{
	struct dpdk_proto *p = (struct dpdk_proto *)P;

	if (p->calcrt != 2)
		DPDK_TRACE(D_EVENTS,
			   "Scheduling routing table calculation with route reload");

	p->calcrt = 2;

	return 1;
}

static int dpdk_rte_same(struct rte *new, struct rte *old)
{
	/* new->attrs == old->attrs always */
	return new->u.dpdk.rt == old->u.dpdk.rt;
}

void dpdk_init_config(struct dpdk_config *c)
{
	dpdk_conf = c;
	INIT_LIST_HEAD(&c->iface_routes);
	INIT_LIST_HEAD(&c->ifaces);
}

static struct proto *dpdk_init(struct proto_config *c)
{
	struct proto *P = proto_new(c, sizeof(struct dpdk_proto));

	P->rt_notify = dpdk_rt_notify;
	P->reload_routes = dpdk_reload_routes;
	P->rte_same = dpdk_rte_same;
	kif_flag |= KIF_F_SCAN_DISABLE;

/*
	p->neigh_notify = dpdk_neigh_notify;
	p->if_notify = dpdk_if_notify;
	p->rte_mergable = dpdk_rte_mergable;
*/
	return P;
}

static void dpdk_copy_routes(struct list_head *dlst, struct list_head *slst)
{
	struct dpdk_route *dr, *sr;

	INIT_LIST_HEAD(dlst);
	list_for_each_entry(sr, slst, n) {
		/* copy one route */
		dr = cfg_alloc(sizeof(struct dpdk_route));
		memcpy(dr, sr, sizeof(struct dpdk_route));

		/* This fn is supposed to be called on fresh src routes, which have 'live'
		   fields (like .chain, .neigh or .installed) zero, so no need to zero them */

		list_add_tail((struct list_head *)dr, dlst);
	}
}

static void dpdk_copy_ifaces(struct list_head *dlst, struct list_head *slst)
{
	struct dpdk_iface *di, *si;

	INIT_LIST_HEAD(dlst);
	list_for_each_entry(si, slst, n) {
		/* copy one iface */
		di = cfg_alloc(sizeof(struct dpdk_iface));
		memcpy(di, si, sizeof(struct dpdk_iface));

		list_add_tail((struct list_head *)di, dlst);
	}
}

static void
dpdk_copy_config(struct proto_config *dest, struct proto_config *src)
{
	struct dpdk_config *d = (struct dpdk_config *)dest;
	struct dpdk_config *s = (struct dpdk_config *)src;

	/* Shallow copy of everything */
	proto_copy_rest(dest, src, sizeof(struct dpdk_config));

	/* Copy route lists */
	dpdk_copy_routes(&d->iface_routes, &s->iface_routes);
	dpdk_copy_ifaces(&d->ifaces, &s->ifaces);
}

struct protocol proto_dpdk = {
	.name = "DPDK",
	.template = "dpdk%d",
	.preference = DEF_PREF_DPDK,
	.config_size = sizeof(struct dpdk_config),
	.init = dpdk_init,
	.dump = dpdk_dump,
	.start = dpdk_start,
	.copy_config = dpdk_copy_config
/*
	.reconfigure = dpdk_reconfigure,
	.shutdown = dpdk_shutdown,
	.cleanup = dpdk_cleanup,
*/
};

