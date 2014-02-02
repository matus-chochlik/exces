/**
 *  @example advanced/rooms/input_output.cpp
 *  @brief Implementation of the input/output component
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "input_output.hpp"
#include <iostream>
#include <cassert>

io& io::operator = (io&&)
{
	assert(!"io cannot be reassigned!");
}

void io::newl(void) const
{
	out << std::endl;
}

void io::new_screen(void) const
{
	for(unsigned i=0; i!=8; ++i)
		out << "==========";
	out << std::endl;
}

void io::delimit(void) const
{	
	for(unsigned i=0; i!=8; ++i)
		out << "----------";
	out << std::endl;
}

