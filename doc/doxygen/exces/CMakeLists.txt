#  Copyright 2010-2013 Matus Chochlik. Distributed under the Boost
#  Software License, Version 1.0. (See accompanying file
#  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 2.8)

# doxygen working directory
set(DOXYGEN_WD ${CMAKE_CURRENT_SOURCE_DIR})

# get the version string
file(READ ${PROJECT_SOURCE_DIR}/VERSION EXCES_VERSION_STR)

# doxyfile for building html docs
set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile-html)
file(WRITE ${DOXYFILE} "@INCLUDE = ${DOXYGEN_WD}/Doxyfile.exces\n")
file(APPEND ${DOXYFILE} "IMAGE_PATH += ../../logo/ \n")
file(APPEND ${DOXYFILE} "OUTPUT_DIRECTORY = ${CMAKE_CURRENT_BINARY_DIR}\n")
file(APPEND ${DOXYFILE} "GENERATE_TAGFILE = ${CMAKE_CURRENT_BINARY_DIR}/doxygen.tags\n")
file(APPEND ${DOXYFILE} "PROJECT_NUMBER = ${EXCES_VERSION_STR}\n")
file(APPEND ${DOXYFILE} "QUIET = YES\n")

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
	COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "Generating exces HTML documentation"
)

add_custom_target(
	exces-doc
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
)

add_custom_target(
	rebuild-exces-doc
	COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

unset(DOXYFILE)
