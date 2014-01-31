/**
 *  @example advanced/rooms/physical.hpp
 *  @brief The components storing physical properties.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef EXCES_EXAMPLE_ADVANCED_ROOMS_PHYSICAL_HPP
#define EXCES_EXAMPLE_ADVANCED_ROOMS_PHYSICAL_HPP

#include "common.hpp"
#include <vector>

struct destructible
{
	float durability;
	float condition;
};

struct physical_object
{
	float weight;
	float volume;

	physical_object(float w, float v)
	 : weight(w)
	 , volume(v)
	{ }
};

struct gravity_modifier
{
	float coef;
};

float get_weight(intity);
float get_volume(intity);

#endif // include guard
