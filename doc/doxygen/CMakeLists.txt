#  Copyright 2010-2013 Matus Chochlik. Distributed under the Boost
#  Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 2.8)

find_package(Doxygen)
if(DOXYGEN_FOUND)
	add_subdirectory(exces)

	# docs may or may not be built by default
	# based on the user preferences
	if(EXCES_NO_DOCS)
		add_custom_target(doc DEPENDS exces-doc)
	else()
		add_custom_target(doc ALL DEPENDS exces-doc)
	endif()
	add_custom_target(
		rebuild-doc
		DEPENDS rebuild-exces-doc
	)
else()
	set(EXCES_NO_DOCS On PARENT_SCOPE)
endif()

