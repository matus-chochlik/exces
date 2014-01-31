/**
 *  @example advanced/rooms/rooms.cpp
 *  @brief Program entry point for the 'rooms' example.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "components.hpp"
#include "gameplay.hpp"

#include <iostream>

int main(void)
{
	use_components();

	manager m;
	implicit_manager im(m);

	game_data game(m);

	game.player.add(io(std::cin, std::cout));
	
	init_game(game);
	play_game(game, game.player);

	return 0;
}

