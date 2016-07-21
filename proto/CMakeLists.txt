#
# Copyright 2016 Xiaomi Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.
#
# Authors:    Yu Bo <yubo@xiaomi.com>
#
cmake_minimum_required(VERSION 2.6)
set(BFD_SOURCES ./bfd/packets.c ./bfd/io.c ./bfd/bfd.c)
set(BGP_SOURCES ./bgp/attrs.c ./bgp/packets.c ./bgp/bgp.c)
set(PIPE_SOURCES ./pipe/pipe.c)
set(RIP_SOURCES ./rip/packets.c ./rip/rip.c)
set(STATIC_SOURCES ./static/static.c)
set(OSPF_SOURCES ./ospf/iface.c ./ospf/lsreq.c ./ospf/dbdes.c ./ospf/ospf.c ./ospf/neighbor.c ./ospf/topology.c ./ospf/rt.c ./ospf/packet.c ./ospf/lsalib.c ./ospf/lsack.c ./ospf/lsupd.c ./ospf/hello.c)
set(SOURCES ${BFD_SOURCES} ${BGP_SOURCES} ${PIPE_SOURCES} ${RIP_SOURCES} ${STATIC_SOURCES} ${OSPF_SOURCES})

ADD_LIBRARY(proto-static STATIC ${OSPF_SOURCES} ${STATIC_SOURCES})
target_link_libraries(proto-static bird-static pthread)

