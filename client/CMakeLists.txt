#
# Copyright 2016 Xiaomi Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.
#
# Authors:    Yu Bo <yubo@xiaomi.com>
#
cmake_minimum_required(VERSION 2.6)

set(SOURCES commands.c util.c client.c)

add_library(client-static STATIC ${SOURCES})

add_executable(birdc birdc.c)
target_link_libraries(birdc client-static bird-static pthread readline history ncurses)

add_executable(birdcl birdcl.c)
target_link_libraries(birdcl client-static bird-static pthread)
