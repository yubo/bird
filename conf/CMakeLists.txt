#
#target_link_libraries(bird nest-static conf-static proto-static filter-static rt)
# Copyright 2016 Xiaomi Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.
#
# Authors:    Yu Bo <yubo@xiaomi.com>
#
cmake_minimum_required(VERSION 2.6)


set(SOURCES cf-lex.c cf-parse.tab.c conf.c)

add_library(conf-static STATIC ${SOURCES})
target_link_libraries(conf-static nest-static)
