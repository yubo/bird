#
# Copyright 2016 Xiaomi Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.
#
# Authors:    Yu Bo <yubo@xiaomi.com>
#
cmake_minimum_required(VERSION 2.6)


set(SOURCES bitops.c checksum.c event.c io.c ip.c krt.c log.c md5.c mempool.c netlink.c patmatch.c printf.c random.c resource.c sha1.c sha256.c sha512.c slab.c tbf.c xmalloc.c)

add_library(bird-static STATIC ${SOURCES})
