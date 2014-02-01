/**
 *  @example advanced/rooms/location.hpp
 *  @brief The location components
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef EXCES_EXAMPLE_ADVANCED_ROOMS_LOCATIONS_HPP
#define EXCES_EXAMPLE_ADVANCED_ROOMS_LOCATIONS_HPP

#include "common.hpp"
#include <set>

struct location
{
	std::set<entity> locations;
	std::set<entity> items;
};

struct portal
{
	entity from;
	entity to;

	portal(entity f, entity t)
	 : from(f)
	 , to(t)
	{ }
};

#endif // include guard
