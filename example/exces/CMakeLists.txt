#  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
#  Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 2.8)

# add a target for the examples
add_custom_target(exces-examples ALL)

file(GLOB EXAMPLES "${CMAKE_CURRENT_SOURCE_DIR}/[0-9][0-9][0-9]*.cpp")

foreach(EXAMPLE_PATH ${EXAMPLES})
	get_filename_component(EXAMPLE_NAME "${EXAMPLE_PATH}" NAME_WE)
	set(TARGET_NAME "${EXAMPLE_NAME}")
	add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL ${EXAMPLE_PATH})
	add_dependencies(exces-examples ${TARGET_NAME})
endforeach()
