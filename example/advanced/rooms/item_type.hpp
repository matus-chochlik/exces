/**
 *  @example advanced/rooms/item_type.hpp
 *  @brief The item_type component
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef EXCES_EXAMPLE_ADVANCED_ROOMS_ITEM_TYPE_HPP
#define EXCES_EXAMPLE_ADVANCED_ROOMS_ITEM_TYPE_HPP

#include "common.hpp"

struct item_type
 : entity
{
	item_type(void) = default;
	item_type(const entity& e)
	 : entity(e)
	{ }
};

#endif // include guard
