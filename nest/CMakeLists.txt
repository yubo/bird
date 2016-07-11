#
# Copyright 2016 Xiaomi Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.
#
# Authors:    Yu Bo <yubo@xiaomi.com>
#
cmake_minimum_required(VERSION 2.6)

set(SOURCES a-path.c a-set.c cli.c cmds.c iface.c locks.c neighbor.c password.c proto.c rt-attr.c rt-dev.c rt-fib.c rt-roa.c rt-table.c)

add_executable(bird ${SOURCES})
target_link_libraries(bird bird-static conf-static proto-static filter-static)
