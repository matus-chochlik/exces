/**
 *  @example advanced/rooms/gameplay.hpp
 *  @brief The gameplay.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef EXCES_EXAMPLE_ADVANCED_ROOMS_GAMEPLAY_HPP
#define EXCES_EXAMPLE_ADVANCED_ROOMS_GAMEPLAY_HPP

#include "common.hpp"
#include <iosfwd>
#include <vector>

struct game_data
{
	intity world;
	intity player;

	classification<entity> portals_from;
	classification<entity> portals_to;

	game_data(manager& m);
};

extern void init_game(game_data&);
extern void play_game(game_data&, intity);
extern void entered_location(game_data&, intity);

#endif // include guard
