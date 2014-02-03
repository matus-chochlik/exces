#  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
#  Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 2.8)

add_executable(
	rooms EXCLUDE_FROM_ALL
	rooms.cpp
	actor.cpp
	components.cpp
	physical.cpp
	container.cpp
	input_output.cpp
	name_desc.cpp
	gameplay.cpp
	interaction.cpp
	initialization.cpp
)
add_dependencies(exces-examples rooms)
