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

struct actor
{
	intity current_location;

	std::vector<entity> visible_entities;

	intity hands;
	std::vector<entity> gear;
};

#endif // include guard
