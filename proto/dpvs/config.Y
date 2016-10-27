/*
 * Copyright 2016 yubo. All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

CF_HDR

#include "proto/dpvs/dpvs.h"
#define VS_IGNORE_MACRO
#include "vs_config.h"

CF_DEFINES

#define DPVS_CFG ((struct dpvs_config *) this_proto)
static struct dpvs_route *this_drt, *this_drt_nh, *last_drt_nh;
static struct dpvs_iface *this_if, *this_if_nh, *last_if_nh;
static struct f_inst **this_drt_last_cmd;

static void
dpvs_route_finish(void)
{
	int ret;
	if ((ret = parse_arg_route(this_drt)))
		cf_error("parse_arg_route error %d", ret);
}

static void
dpvs_proto_finish(void)
{
	int ret;
	if ((ret = vs_parse_finish()))
		cf_error("vs_parse_finish error %d", ret);
}

CF_DECLS

CF_KEYWORDS(DPVS, IFA, LPM, PORT, SIP, TIP)
CF_KEYWORDS(RX, TX, WORKER_LCORES, RING_SIZES, BURST_SIZES, CTRL_LCORE)

CF_GRAMMAR

CF_ADDTO(proto, dpvs_proto '}' { dpvs_proto_finish(); } )

dpvs_proto_start: proto_start DPVS {
     this_proto = proto_config_new(&proto_dpvs, $1);
     dpvs_init_config((struct dpvs_config *) this_proto);
     vs_parse_start();
  }
 ;

dpvs_proto:
   dpvs_proto_start proto_name '{'
 | dpvs_proto dpvs_proto_item ';'
;

dpvs_proto_item:
  proto_item
| IFA ip_prefix PORT expr {
	this_if = cfg_allocz(sizeof(struct dpvs_iface));
	list_add_tail(&this_if->n, &DPVS_CFG->ifaces);
	this_if->ip = $2.ip;
	this_if->prefix = $2.prefix;
	this_if->pxlen = $2.pxlen;
	this_if->brd = $2.brd;
	this_if->port = $4;
	vs_parse_ifa(this_if->port, this_if->ip.addr, this_if->prefix.addr, this_if->pxlen);
}| RX text {
	int ret;
	if ((ret = parse_arg_rx($2)))
		cf_error("Incorrect value for rx argument (%d)", ret);
}| TX text {
	int ret;
	if ((ret = parse_arg_tx($2)))
		cf_error("Incorrect value for tx argument (%d)", ret);
}| WORKER_LCORES text {
	int ret;
	if ((ret = parse_arg_w($2)))
		cf_error("Incorrect value for w argument (%d)", ret);
}| RING_SIZES text {
	int ret;
	if ((ret = parse_arg_rsz($2)))
		cf_error("Incorrect value for rsz argument (%d)", ret);
}| BURST_SIZES text {
	int ret;
	if ((ret = parse_arg_bsz($2)))
		cf_error("Incorrect value for bsz argument (%d)", ret);
}| CTRL_LCORE text {
	int ret;
	if ((ret = parse_arg_c($2)))
		cf_error("Incorrect value for c argument (%d)", ret);
}| dpvs_route { dpvs_route_finish(); }
;

dpvs_route0: LPM prefix {
     this_drt = cfg_allocz(sizeof(struct dpvs_route));
     list_add_tail(&this_drt->n,&DPVS_CFG->iface_routes);
     this_drt->net = $2.addr;
     this_drt->masklen = $2.len;
}
;

dpvs_route: 
  dpvs_route0
 | dpvs_route PORT expr {
     this_drt->port = $3;
}| dpvs_route SIP ipa {
     this_drt->sip = $3;
}| dpvs_route TIP ipa {
     this_drt->tip = $3;
}
;

CF_CODE

CF_END
