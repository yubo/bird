static void dpdk_install(struct proto *p, struct dpdk_route *r,
		struct iface *ifa)
{
	struct network *n;
	struct rta a;
	struct rte *e;

	if (r->installed > 0)
		return;

	DBG("Installing dpdk route %I/%d, rtd=%d\n", r->net, r->masklen,
	    r->dest);
	bzero(&a, sizeof(a));
	a.src = p->main_source;  //r->sip ?
	a.source = RTS_DPDK;
	a.scope = SCOPE_UNIVERSE;
	a.cast = RTC_UNICAST;
	a.dest = r->dest;
	a.gw = r->tip; //r->rip
	/*
	 * todo: create ifa from int(r->via_if) to struct iface()
	 */
#ifdef TODO
	a.iface = ifa;
#endif

	/* We skip rta_lookup() here */

	n = net_get(p->table, r->net, r->masklen);
	e = rte_get_temp(&a);
	e->net = n;
	e->pflags = 0;

	rte_update(p, n, e);
	r->installed = 1;
}

static void
dpdk_add(struct proto *p, struct dpdk_config *cf, struct dpdk_route *r)
{
	DBG("dpdk_add(%I/%d,%d)\n", r->net, r->masklen, r->dest);
	dpdk_install(p, r, NULL);
}
static void dpdk_remove(struct proto *p, struct dpdk_route *r)
{
	struct network *n;

	if (!r->installed)
		return;

	DBG("Removing dpdk route %I/%d via %I\n",
			r->net, r->masklen, r->via);
	n = net_find(p->table, r->net, r->masklen);
	rte_update(p, n, NULL);
	r->installed = 0;
}

static void dpdk_bfd_notify(struct bfd_request *req);

static void dpdk_update_bfd(struct proto *p, struct dpdk_route *r)
{
	struct neighbor *nb = r->neigh;
	int bfd_up = (nb->scope > 0) && r->use_bfd;

	if (bfd_up && !r->bfd_req) {
		// ip_addr local = ipa_nonzero(r->local) ? r->local : nb->ifa->ip;
		r->bfd_req = bfd_request_session(p->pool, r->via,
				nb->ifa->ip, nb->iface, dpdk_bfd_notify, r);
	}

	if (!bfd_up && r->bfd_req) {
		rfree(r->bfd_req);
		r->bfd_req = NULL;
	}
}

static int dpdk_decide(struct dpdk_config *cf, struct dpdk_route *r)
{
	/* r->dest != RTD_MULTIPATH, but may be RTD_NONE (part of multipath route)
	   the route also have to be valid (r->neigh != NULL) */

	if (r->neigh->scope < 0)
		return 0;

	if (cf->check_link && !(r->neigh->iface->flags & IF_LINK_UP))
		return 0;

	if (r->bfd_req && r->bfd_req->state != BFD_STATE_UP)
		return 0;

	return 1;
}
static void dpdk_show_rt(struct dpdk_route *r)
{
	byte via[STD_ADDRESS_P_LENGTH + 16];

	switch (r->dest) {
	case RTD_ROUTER:
		bsprintf(via, "via %I%J", r->via, r->via_if);
		break;
	case RTD_DEVICE:
		bsprintf(via, "dev %s", r->if_name);
		break;
	case RTD_BLACKHOLE:
		bsprintf(via, "blackhole");
		break;
	case RTD_UNREACHABLE:
		bsprintf(via, "unreachable");
		break;
	case RTD_PROHIBIT:
		bsprintf(via, "prohibited");
		break;
	case RTD_MULTIPATH:
		bsprintf(via, "multipath");
		break;
	case DPDK_RTDX_RECURSIVE:
		bsprintf(via, "recursive %I", r->via);
		break;
	default:
		bsprintf(via, "???");
	}
	cli_msg(-1009, "%I/%d %s%s%s", r->net, r->masklen, via,
		r->bfd_req ? " (bfd)" : "", r->installed ? "" : " (dormant)");

	struct dpdk_route *r2;
	if (r->dest == RTD_MULTIPATH)
		for (r2 = r->mp_next; r2; r2 = r2->mp_next)
			cli_msg(-1009, "\tvia %I%J weight %d%s%s",
					r2->via, r2->via_if, r2->masklen + 1,	/* really */
					r2->bfd_req ? " (bfd)" : "",
					r2->installed ? "" : " (dormant)");
}

void dpdk_show(struct proto *P)
{
	struct dpdk_config *c = (void *)P->cf;
	struct dpdk_route *r;

	list_for_each_entry(r, &c->other_routes, n)
	    dpdk_show_rt(r);
	list_for_each_entry(r, &c->iface_routes, n)
	    dpdk_show_rt(r);
	cli_msg(0, "");
}
static void dpdk_rte_cleanup(struct proto *p, struct dpdk_route *r)
{
	struct dpdk_route *r2;

	if (r->bfd_req) {
		rfree(r->bfd_req);
		r->bfd_req = NULL;
	}

	if (r->dest == RTD_MULTIPATH)
		for (r2 = r->mp_next; r2; r2 = r2->mp_next)
			if (r2->bfd_req) {
				rfree(r2->bfd_req);
				r2->bfd_req = NULL;
			}
}
static int dpdk_shutdown(struct proto *p)
{
	struct dpdk_config *cf = (void *)p->cf;
	struct dpdk_route *r;

	/* Just reset the flag, the routes will be flushed by the nest */
	list_for_each_entry(r, &cf->iface_routes, n)
	    r->installed = 0;
	list_for_each_entry(r, &cf->other_routes, n) {
		dpdk_rte_cleanup(p, r);
		r->installed = 0;
	}

	return PS_DOWN;
}

static void dpdk_cleanup(struct proto *p)
{
	struct dpdk_config *cf = (void *)p->cf;

	if (cf->igp_table)
		rt_unlock_table(cf->igp_table->table);
}

static void dpdk_update_rte(struct proto *p, struct dpdk_route *r)
{
	switch (r->dest) {
	case RTD_ROUTER:
		if (dpdk_decide((struct dpdk_config *)p->cf, r))
			dpdk_install(p, r, r->neigh->iface);
		else
			dpdk_remove(p, r);
		break;

	case RTD_NONE:		/* a part of multipath route */
		{
			int decision =
			    dpdk_decide((struct dpdk_config *)p->cf, r);
			if (decision == r->installed)
				break;	/* no change */
			r->installed = decision;

			struct dpdk_route *r1, *r2;
			int count = 0;
			r1 = (void *)r->if_name;	/* really */
			for (r2 = r1->mp_next; r2; r2 = r2->mp_next)
				count += r2->installed;

			if (count) {
				/* Set of nexthops changed - force reinstall */
				r1->installed = 0;
				dpdk_install(p, r1, NULL);
			} else
				dpdk_remove(p, r1);

			break;
		}
	}
}

static void dpdk_neigh_notify(struct neighbor *n)
{
	struct proto *p = n->proto;
	struct dpdk_route *r;

	DBG("Static: struct neighbor notify for %I: iface %p\n", n->addr,
	    n->iface);
	for (r = n->data; r; r = r->chain) {
		dpdk_update_bfd(p, r);
		dpdk_update_rte(p, r);
	}
}

static void dpdk_bfd_notify(struct bfd_request *req)
{
	struct dpdk_route *r = req->data;
	struct proto *p = r->neigh->proto;

	// if (req->down) TRACE(D_EVENTS, "BFD session down for nbr %I on %s", XXXX);

	dpdk_update_rte(p, r);
}
static void dpdk_if_notify(struct proto *p, unsigned flags, struct iface *i)
{
	struct dpdk_route *r;
	struct dpdk_config *c = (void *)p->cf;

	if (flags & IF_CHANGE_UP) {
		list_for_each_entry(r, &c->iface_routes, n)
		    if (!strcmp(r->if_name, i->name))
			dpdk_install(p, r, i);
	} else if (flags & IF_CHANGE_DOWN) {
		list_for_each_entry(r, &c->iface_routes, n)
		    if (!strcmp(r->if_name, i->name))
			dpdk_remove(p, r);
	}
}

int dpdk_rte_mergable(struct rte *pri, struct rte *sec)
{
	return 1;
}

static inline int
dpdk_same_net(struct dpdk_route *x, struct dpdk_route *y)
{
	return ipa_equal(x->net, y->net) && (x->masklen == y->masklen);
}

static inline int
dpdk_same_dest(struct dpdk_route *x, struct dpdk_route *y)
{
	if (x->dest != y->dest)
		return 0;

	switch (x->dest) {
	case RTD_ROUTER:
		return ipa_equal(x->via, y->via) && (x->via_if == y->via_if);

	case RTD_DEVICE:
		return !strcmp(x->if_name, y->if_name);

	case RTD_MULTIPATH:
		for (x = x->mp_next, y = y->mp_next;
		     x && y; x = x->mp_next, y = y->mp_next)
			if (!ipa_equal(x->via, y->via)
			    || (x->via_if != y->via_if)
			    || (x->use_bfd != y->use_bfd))
				return 0;
		return !x && !y;

	case DPDK_RTDX_RECURSIVE:
		return ipa_equal(x->via, y->via);

	default:
		return 1;
	}
}

static inline int
dpdk_same_rte(struct dpdk_route *x, struct dpdk_route *y)
{
	return dpdk_same_dest(x, y) && i_same(x->cmds, y->cmds);
}

static void
dpdk_match(struct proto *p, struct dpdk_route *r, struct dpdk_config *n)
{
	struct dpdk_route *t;

	/*
	 * For given old route *r we find whether a route to the same
	 * network is also in the new route list. In that case, we keep the
	 * route and possibly update the route later if destination changed.
	 * Otherwise, we remove the route.
	 */

	if (r->neigh)
		r->neigh->data = NULL;

	list_for_each_entry(t, &n->iface_routes, n)
	    if (dpdk_same_net(r, t))
		goto found;

	list_for_each_entry(t, &n->other_routes, n)
	    if (dpdk_same_net(r, t))
		goto found;

	dpdk_remove(p, r);
	return;

found:
	/* If destination is different, force reinstall */
	if ((r->installed > 0) && !dpdk_same_rte(r, t))
		t->installed = -1;
	else
		t->installed = r->installed;
}

static inline struct rtable *cf_igp_table(struct dpdk_config *cf)
{
	return cf->igp_table ? cf->igp_table->table : NULL;
}


static int dpdk_reconfigure(struct proto *p, struct proto_config *new)
{
	struct dpdk_config *o = (void *)p->cf;
	struct dpdk_config *n = (void *)new;
	struct dpdk_route *r;

	if (cf_igp_table(o) != cf_igp_table(n))
		return 0;

	/* Delete all obsolete routes and reset struct neighbor entries */
	list_for_each_entry(r, &o->iface_routes, n)
	    dpdk_match(p, r, n);
	list_for_each_entry(r, &o->other_routes, n)
	    dpdk_match(p, r, n);

	/* Now add all new routes, those not changed will be ignored by dpdk_install() */
	list_for_each_entry(r, &n->iface_routes, n) {
		struct iface *ifa;
		if ((ifa = if_find_by_name(r->if_name)) && (ifa->flags & IF_UP))
			dpdk_install(p, r, ifa);
	}
	list_for_each_entry(r, &n->other_routes, n)
	    dpdk_add(p, n, r);

	list_for_each_entry(r, &o->other_routes, n)
	    dpdk_rte_cleanup(p, r);

	return 1;
}

