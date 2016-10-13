#
#target_link_libraries(bird nest-static conf-static proto-static filter-static rt)
# Copyright 2016 Xiaomi Corporation. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.
#
# Authors:    Yu Bo <yubo@xiaomi.com>
#
cmake_minimum_required(VERSION 2.6)

find_package(BISON)
find_package(FLEX)

add_definitions(-Wno-unused)
set(KEYWORDS_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/gen_keywords.m4
	${CMAKE_CURRENT_SOURCE_DIR}/confbase.Y 
	${CMAKE_CURRENT_SOURCE_DIR}/../sysdep/unix/config.Y 
	${CMAKE_CURRENT_SOURCE_DIR}/../sysdep/unix/krt.Y 
	${CMAKE_CURRENT_SOURCE_DIR}/../sysdep/linux/netlink.Y 
	${CMAKE_CURRENT_SOURCE_DIR}/../nest/config.Y 
	${CMAKE_CURRENT_SOURCE_DIR}/../filter/config.Y 
	#${CMAKE_CURRENT_SOURCE_DIR}/../proto/bfd/config.Y 
	#${CMAKE_CURRENT_SOURCE_DIR}/../proto/bgp/config.Y 
	${CMAKE_CURRENT_SOURCE_DIR}/../proto/ospf/config.Y 
	#${CMAKE_CURRENT_SOURCE_DIR}/../proto/pipe/config.Y 
	#${CMAKE_CURRENT_SOURCE_DIR}/../proto/rip/config.Y 
	${CMAKE_CURRENT_SOURCE_DIR}/../proto/static/config.Y
	${CMAKE_CURRENT_SOURCE_DIR}/../proto/dpvs/config.Y)

set(COMMANDS_SRC 
	${CMAKE_CURRENT_SOURCE_DIR}/gen_commands.m4 
	${CMAKE_CURRENT_SOURCE_DIR}/confbase.Y 
	#${CMAKE_CURRENT_SOURCE_DIR}/../client/cmds.m4 
	${CMAKE_CURRENT_SOURCE_DIR}/../sysdep/unix/config.Y 
	${CMAKE_CURRENT_SOURCE_DIR}/../sysdep/unix/krt.Y
	${CMAKE_CURRENT_SOURCE_DIR}/../sysdep/linux/netlink.Y
	${CMAKE_CURRENT_SOURCE_DIR}/../nest/config.Y
	${CMAKE_CURRENT_SOURCE_DIR}/../filter/config.Y
	#${CMAKE_CURRENT_SOURCE_DIR}/../proto/bfd/config.Y
	#${CMAKE_CURRENT_SOURCE_DIR}/../proto/bgp/config.Y
	${CMAKE_CURRENT_SOURCE_DIR}/../proto/ospf/config.Y 
	#${CMAKE_CURRENT_SOURCE_DIR}/../proto/pipe/config.Y
	#${CMAKE_CURRENT_SOURCE_DIR}/../proto/rip/config.Y
	${CMAKE_CURRENT_SOURCE_DIR}/../proto/static/config.Y
	${CMAKE_CURRENT_SOURCE_DIR}/../proto/dpvs/config.Y)

set(PARSE_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/gen_parser.m4
	${CMAKE_CURRENT_SOURCE_DIR}/confbase.Y 
	${CMAKE_CURRENT_SOURCE_DIR}/../sysdep/unix/config.Y 
	${CMAKE_CURRENT_SOURCE_DIR}/../sysdep/unix/krt.Y 
	${CMAKE_CURRENT_SOURCE_DIR}/../sysdep/linux/netlink.Y 
	${CMAKE_CURRENT_SOURCE_DIR}/../nest/config.Y 
	${CMAKE_CURRENT_SOURCE_DIR}/../filter/config.Y 
	#${CMAKE_CURRENT_SOURCE_DIR}/../proto/bfd/config.Y 
	#${CMAKE_CURRENT_SOURCE_DIR}/../proto/bgp/config.Y 
	${CMAKE_CURRENT_SOURCE_DIR}/../proto/ospf/config.Y 
	#${CMAKE_CURRENT_SOURCE_DIR}/../proto/pipe/config.Y 
	#${CMAKE_CURRENT_SOURCE_DIR}/../proto/rip/config.Y 
	${CMAKE_CURRENT_SOURCE_DIR}/../proto/static/config.Y
	${CMAKE_CURRENT_SOURCE_DIR}/../proto/dpvs/config.Y)

add_custom_target(PARSE_HEAD ALL
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/keywords.h
	${CMAKE_CURRENT_SOURCE_DIR}/commands.h)

add_custom_command(OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/keywords.h"
	COMMAND m4 -P ${KEYWORDS_SRC}
	> "${CMAKE_CURRENT_SOURCE_DIR}/keywords.h"
	DEPENDS ${KEYWORDS_SRC}
	COMMENT "building keywords.h")

add_custom_command(OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/commands.h"
	COMMAND m4 -P ${COMMANDS_SRC} | sort
	> "${CMAKE_CURRENT_SOURCE_DIR}/commands.h"
	DEPENDS ${COMMANDS_SRC}
	COMMENT "building commands.h")

add_custom_command(OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/cf-parse.y"
	COMMAND m4 -P ${PARSE_SRC}
	> "${CMAKE_CURRENT_SOURCE_DIR}/cf-parse.y"
	DEPENDS ${PARSE_SRC}
	COMMENT "building parse.y")


BISON_TARGET(CfParser
	"${CMAKE_CURRENT_SOURCE_DIR}/cf-parse.y"
	"${CMAKE_CURRENT_SOURCE_DIR}/cf-parse.tab.c"
	COMPILE_FLAGS "-bcf-parse -dv -pcf_")

FLEX_TARGET(CfScanner 
	"${CMAKE_CURRENT_SOURCE_DIR}/cf-lex.l"
	"${CMAKE_CURRENT_SOURCE_DIR}/cf-lex.c"
	COMPILE_FLAGS "-s -B -8 -ocf-lex.c -Pcf_")
#ADD_FLEX_BISON_DEPENDENCY(CfScanner CfParser)

MACRO(ADD_FILE_DEPENDANCY file) 
	SET(${file}_deps ${${file}_deps} ${ARGN}) 
	SET_SOURCE_FILES_PROPERTIES( ${file} 
		PROPERTIES OBJECT_DEPENDS "${${file}_deps}" ) 
ENDMACRO(ADD_FILE_DEPENDANCY)

ADD_FILE_DEPENDANCY(${FLEX_CfScanner_OUTPUTS}
	${BISON_CfParser_OUTPUT_HEADER}
	"${CMAKE_CURRENT_SOURCE_DIR}/keywords.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/commands.h"
	)
#set_source_files_properties(${FLEX_CfScanner_OUTPUTS}
#	PROPERTIES OBJECT_DEPENDS ${BISON_CfParser_OUTPUT_HEADER}
#	"${CMAKE_CURRENT_SOURCE_DIR}/keywords.h"
#	"${CMAKE_CURRENT_SOURCE_DIR}/commands.h"
#	)
set(SOURCES
	${BISON_CfParser_OUTPUTS}
	${FLEX_CfScanner_OUTPUTS}
	${CMAKE_CURRENT_SOURCE_DIR}/conf.c)

add_library(conf-static STATIC ${SOURCES})
target_link_libraries(conf-static nest-static)
