#  Copyright 2010-2013 Matus Chochlik. Distributed under the Boost
#  Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 2.8)

include(./exces_testing.cmake)

add_test(
	build-exces-examples 
	"${CMAKE_COMMAND}"
	--build "${CMAKE_BINARY_DIR}"
	--target exces-examples
)

exces_exec_test(entity)
exces_exec_test(group)
