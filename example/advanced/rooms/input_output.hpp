/**
 *  @example advanced/rooms/input_output.hpp
 *  @brief The input/output component
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef EXCES_EXAMPLE_ADVANCED_ROOMS_INPUT_OUTPUT_HPP
#define EXCES_EXAMPLE_ADVANCED_ROOMS_INPUT_OUTPUT_HPP

#include "common.hpp"
#include <iosfwd>

struct io
{
	std::istream& in;
	std::ostream& out;

	io(std::istream& i, std::ostream& o)
	 : in(i)
	 , out(o)
	{ }

	io(const io&) = default;
	io(io&&) = default;

	io& operator = (io&&);
	void new_screen(void) const;
	void delimit(void) const;
};

#endif // include guard
