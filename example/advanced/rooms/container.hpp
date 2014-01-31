/**
 *  @example advanced/rooms/container.hpp
 *  @brief The container component.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef EXCES_EXAMPLE_ADVANCED_ROOMS_CONTAINER_HPP
#define EXCES_EXAMPLE_ADVANCED_ROOMS_CONTAINER_HPP

#include "common.hpp"
#include <set>
#include <cassert>

struct container
{
	float max_weight; // [kg]
	float max_volume; // [m^3]
	float grow_factor;
	std::set<entity> items;

	container(float mw, float mv, float gf)
	 : max_weight(mw)
	 , max_volume(mv)
	 , grow_factor(gf)
	{ }

	float occ_weight(void) const;
	float occ_volume(void) const;
	float rem_weight(void) const;
	float rem_volume(void) const;
};

bool put_into(intity cont, intity item);


#endif // include guard
