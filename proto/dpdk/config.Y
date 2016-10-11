/*
 * Copyright 2016 yubo. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

CF_HDR

#include "proto/dpdk/dpdk.h"

CF_DEFINES

#define DPDK_CFG ((struct dpdk_config *) this_proto)
static struct dpdk_route *this_drt, *this_drt_nh, *last_drt_nh;
static struct dpdk_iface *this_if, *this_if_nh, *last_if_nh;
static struct f_inst **this_drt_last_cmd;

static void
dpdk_route_finish(void)
{
}

CF_DECLS

CF_KEYWORDS(DPDK, IFA, LPM, PORT, SIP, TIP)

CF_GRAMMAR

CF_ADDTO(proto, dpdk_proto '}')

dpdk_proto_start: proto_start DPDK {
     this_proto = proto_config_new(&proto_dpdk, $1);
     dpdk_init_config((struct dpdk_config *) this_proto);
  }
 ;

dpdk_proto:
   dpdk_proto_start proto_name '{'
 | dpdk_proto proto_item ';'
 | dpdk_proto dpdk_if ';'
 | dpdk_proto dpdk_route ';' { dpdk_route_finish(); }
 ;

dpdk_if: IFA ip_prefix PORT expr {
	this_if = cfg_allocz(sizeof(struct dpdk_iface));
	list_add_tail(&this_if->n, &DPDK_CFG->ifaces);
	this_if->ip = $2.ip;
	this_if->prefix = $2.prefix;
	this_if->pxlen = $2.pxlen;
	this_if->brd = $2.brd;
	this_if->port = $4;
}
;

dpdk_route0: LPM prefix {
     this_drt = cfg_allocz(sizeof(struct dpdk_route));
     list_add_tail(&this_drt->n,&DPDK_CFG->iface_routes);
     this_drt->net = $2.addr;
     this_drt->masklen = $2.len;
  }
 ;

dpdk_route: 
  dpdk_route0
| dpdk_route PORT expr {
     this_drt->via_if = $3;
}
| dpdk_route SIP ipa {
     this_drt->sip = $3;
}
| dpdk_route TIP ipa {
     this_drt->tip = $3;
}
;

CF_CODE

CF_END
