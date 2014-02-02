/**
 *  @example advanced/rooms/actor.hpp
 *  @brief The actor component.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef EXCES_EXAMPLE_ADVANCED_ROOMS_ACTOR_HPP
#define EXCES_EXAMPLE_ADVANCED_ROOMS_ACTOR_HPP

#include "common.hpp"
#include <vector>

enum class body_part
{
	legs,
	torso,
	back,
	hands,
	neck,
	head
};

struct gear_kind
{
	body_part used_on;

	gear_kind(body_part bp)
	 : used_on(bp)
	{ }

	friend bool operator == (gear_kind a, gear_kind b)
	{
		return a.used_on == b.used_on;
	}

	friend bool operator <  (gear_kind a, gear_kind b)
	{
		return a.used_on <  b.used_on;
	}
};

struct gear_slots
{
	std::size_t max_items;
	std::set<intity> items;

	gear_slots(std::size_t max)
	 : max_items(max)
	{ }
};

struct actor
{
	std::map<body_part, intity> body_parts;

	intity current_location;

	std::vector<entity> visible_entities;
};

extern void use_inventory(game_data&, intity player);

#endif // include guard
