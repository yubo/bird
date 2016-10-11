/*
 * Copyright 2016 yubo. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#ifndef _BIRD_DPDK_H_
#define _BIRD_DPDK_H_

#include "nest/route.h"
#include "nest/bfd.h"

#ifdef LOCAL_DEBUG
#define DPDK_FORCE_DEBUG 1
#else
#define DPDK_FORCE_DEBUG 0
#endif

// dpdk dev port 0~255 -> 256->511
#define DPDK_PORT_FLAG 0x0100
#define DPDK_PORT_MASK 0x00ff

#define DPDK_TRACE(flags, msg, args...) \
  do { if ((p->p.debug & flags) || DPDK_FORCE_DEBUG) \
    log(L_TRACE "%s: " msg, p->p.name , ## args ); } while(0)

struct dpdk_proto {
	struct proto p;
	int calcrt;		/* Routing table calculation scheduled?
				   0=no, 1=normal, 2=forced reload */
};


struct dpdk_config {
	struct proto_config c;
	struct list_head iface_routes;	/* interface routes */
	struct list_head ifaces;	/* dpdk interfaces */
};

void dpdk_init_config(struct dpdk_config *);

struct dpdk_route {
	struct list_head n;	/* Next for the same struct neighbor */
	struct dpdk_route *chain;
	ip_addr net;	/* Network we route */
	int masklen;	/* Mask length */
	int dest;	/* Destination type (RTD_*) */
	int via_if;	/* Destination iface, for link-local vias */
	ip_addr  sip;	/* via_if's addr */
	ip_addr tip;	/* Destination router */
	int installed;	/* Installed in rt table, -1 for reinstall */
};

struct dpdk_iface {
	struct list_head n;	/* Next for the same struct neighbor */
	ip_addr ip;		/* ip address of this host */
	ip_addr prefix;		/* network prefix */
	int pxlen;		/* prefix length */
	ip_addr brd;		/* broadcaast address */
	int port;
};

/* Dummy nodes (parts of multipath route) abuses masklen field for weight
   and if_name field for a ptr to the master (RTD_MULTIPATH) node. */

#define DPDK_RTDX_RECURSIVE 0x7f	/* Phony dest value for recursive routes */

void dpdk_show(struct proto *);

#endif
