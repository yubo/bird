#
# Copyright 2016 Xiaomi Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.
#
# Authors:    Yu Bo <yubo@xiaomi.com>
#
cmake_minimum_required(VERSION 2.6)


set(SOURCES f-util.c filter.c tree.c trie.c)

add_library(filter-static STATIC ${SOURCES})
