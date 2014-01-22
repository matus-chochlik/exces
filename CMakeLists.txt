#  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
#  Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 2.8)

project(exces)

option(EXCES_WITH_TESTS "Configure the testsuite" Off)

include(config/CPP11.cmake)

include_directories(include)
include_directories(implement)

find_package(Boost)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
else()
	message(FATAL_ERROR "Boost UUID is required for exces.")
endif()

add_subdirectory(example)
add_subdirectory(doc)

if(EXCES_WITH_TESTS)
	add_subdirectory("test")
endif()

