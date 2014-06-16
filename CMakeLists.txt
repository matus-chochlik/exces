#  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
#  Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 2.8)

project(exces)

option(EXCES_WITH_TESTS "Configure the testsuite" Off)

include(config/CPP11.cmake)

include_directories(include)
include_directories(implement)
include_directories(${PROJECT_BINARY_DIR}/include)

find_package(Boost)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	set(BOOST_UUID_FOUND 1)
else()
	set(BOOST_UUID_FOUND 0)
endif()

# create the site-configuration header
configure_file(
	${PROJECT_SOURCE_DIR}/config/exces/site_config.hpp.in
	${PROJECT_BINARY_DIR}/include/exces/site_config.hpp
)

install(
	FILES ${PROJECT_BINARY_DIR}/include/exces/site_config.hpp
	DESTINATION include/exces
)

add_subdirectory(example)
add_subdirectory(doc)

if(EXCES_WITH_TESTS)
	add_subdirectory("test")
endif()

