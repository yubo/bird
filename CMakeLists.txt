#
# Copyright 2016 Xiaomi Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.
#
# Authors:    Yu Bo <yubo@xiaomi.com>
#
cmake_minimum_required(VERSION 2.6)

add_definitions(-I${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-pthread -Wall -Wstrict-prototypes -Wno-parentheses -Wno-pointer-sign -fno-strict-aliasing -fno-strict-overflow)

set(DEBUG ON)

if(DEBUG)
	message(STATUS "build Debug")
	add_definitions(-DLOCAL_DEBUG -g -O0)
	#	set(CMAKE_EXE_LINKER_FLAGS "-rdynamic")
	set(CMAKE_BUILD_TYPE "Debug")
else(DEBUG)
	message(STATUS "build Release")
	add_definitions(-O2)
	set(CMAKE_BUILD_TYPE "Release")
	set(BUILD_STRIP true)
endif(DEBUG)

#add_definitions(-I${CMAKE_CURRENT_SOURCE_DIR}/lib/jsonrpc) 
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_INSTALL_PREFIX /usr)

#if(BUILD_STRIP)
#	add_custom_command(TARGET dpvs POST_BUILD COMMAND strip -s  ${EXECUTABLE_OUTPUT_PATH}/dpvs)
#endif(BUILD_STRIP)

add_subdirectory(conf)
add_subdirectory(lib)
add_subdirectory(nest)
add_subdirectory(filter)
add_subdirectory(proto)


add_executable(bird main.c)
target_link_libraries(bird bird-static nest-static conf-static proto-static filter-static rt)
