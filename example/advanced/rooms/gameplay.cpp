/**
 *  @example advanced/rooms/gameplay.cpp
 *  @brief Implementation of the gameplay
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "components.hpp"
#include "gameplay.hpp"
#include "interaction.hpp"

#include <exces/entity_io.hpp>

#include <iostream>
#include <sstream>
#include <cassert>

game_data::game_data(manager& the_manager)
 : portals_from(the_manager, &portal::from)
 , portals_to(the_manager, &portal::to)
{ }

void entered_location(game_data& game, intity p)
{
	if(auto p_actor = p.ref<actor>())
	{
		p_actor->visible_entities.clear();
		
		if(auto p_io = p.ref<io>())
		{
			p_io->new_screen();
			brief_description(game, p, p);
			p_io->out << " is in ";
			brief_description(game, p, p_actor->current_location);
			p_io->newl();
		}
	}
}

void look_around(game_data& game, intity i)
{
	assert(i.has<actor>());

	auto i_actor = i.ref<actor>();

	std::vector<entity> ev;

	auto push_back_entity =
	[&ev](const iter_info&, manager& m, manager::entity_key k)->bool
	{
		ev.push_back(m.get_entity(k));
		return true;
	};

	game.portals_from.for_each(i_actor->current_location, push_back_entity);
	game.portals_to.for_each(i_actor->current_location, push_back_entity);

	if(auto a_loc = i_actor->current_location.ref<location>())
	{
		auto a_loc_l = a_loc.get();
		ev.insert(ev.end(), a_loc_l->items.begin(), a_loc_l->items.end());
	}
	i_actor->visible_entities.swap(ev);
}

bool parse_index_and_apply(
	game_data& game,
	intity player,
	void (*function)(game_data&, intity, intity),
	std::istream& cmd
)
{
	if(auto p_actor = player.ref<actor>())
	{
		unsigned idx;
		cmd >> idx;
		if((!cmd.fail()) && (idx < p_actor->visible_entities.size()))
		{
			const entity& e = p_actor->visible_entities[idx];
			function(game, player, e);
			return true;
		}
	}
	return false;
}

bool step_game(game_data& game, intity player)
{
	auto p_actor = player.ref<actor>();
	if(!p_actor) return false;

	auto p_io = player.cref<io>();
	if(!p_io) return false;

	p_io->delimit();

	if(!p_actor->visible_entities.empty())
	{
		p_io->out << " Entities:";
		p_io->newl();
		std::size_t i=0;
		for(const entity& e: p_actor->visible_entities)
		{
			p_io->out << "  " << i++ << ": ";
			brief_description(game, player, e);
			p_io->newl();
		}
		p_io->delimit();
	}

	p_io->out << " Actions:";
	p_io->newl();
	p_io->out << "  L   Look around";
	p_io->newl();
	if(!p_actor->visible_entities.empty())
	{
		auto n = p_actor->visible_entities.size()-1;
		// use
		p_io->out << "  Un  Use item number n = (0";
		if(n > 0)
		{
			p_io->out << " ... ";
			p_io->out << n;
		}
		p_io->out << ")";
		p_io->newl();
		//examine
		p_io->out << "  En  Examine item number n = (0";
		if(n > 0)
		{
			p_io->out << " ... ";
			p_io->out << n;
		}
		p_io->out << ")";
		p_io->newl();
	}
	p_io->out << "  I   Enter inventory";
	p_io->newl();
	p_io->out << "  X   Exit game";
	p_io->newl();
	p_io->delimit();

	std::stringbuf line;
	p_io->in.get(line);

	if(p_io->in.eof()) return false;
	if(p_io->in.bad()) return false;

	std::stringstream cmd;

	if(p_io->in.good())
	{
		cmd.str(line.str());
	}
	else
	{
		p_io->in.clear();
		cmd.str("L");
	}
	p_io->in.ignore();

	bool invalid_option = false;

	switch(cmd.peek())
	{
		case 'Q':
		case 'q':
		case 'X':
		case 'x':
		{
			p_io->out << "Quitting";
			p_io->newl();
			return false;
			break;
		}
		case 'L':
		case 'l':
		{
			look_around(game, game.player);
			break;
		}
		case 'I':
		case 'i':
		{
			use_inventory(game, game.player);
			break;
		}
		case 'E':
		case 'e':
		{
			cmd.ignore();
			if(!parse_index_and_apply(game, player, full_description, cmd))
			{
				invalid_option = true;
			}
			p_io->newl();
			break;
		}
		case 'U':
		case 'u':
		{
			cmd.ignore();
			if(!parse_index_and_apply(game, player, use_entity, cmd))
			{
				invalid_option = true;
			}
			break;
		}
		default:
		{
			if(!parse_index_and_apply(game, player, use_entity, cmd))
			{
				invalid_option = true;
			}
		}
	}
	if(invalid_option)
	{
		p_io->out << "Invalid option '";
		p_io->out << line.str();
		p_io->out << "', try again.";
		p_io->newl();
	}
	return true;
}

void play_game(game_data& game, intity player)
{
	entered_location(game, player);

	while(step_game(game, player));
}
