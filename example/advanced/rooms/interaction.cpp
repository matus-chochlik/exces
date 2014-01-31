/**
 *  @example advanced/rooms/interaction.cpp
 *  @brief Implementation of interaction with entities
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "components.hpp"
#include "gameplay.hpp"
#include "interaction.hpp"
#include "description.hpp"

#include <sstream>
#include <iostream>
#include <string>
#include <cctype>

void lock_portal(game_data& game, intity p, intity i, bool lock)
{
	auto p_actor = p.ref<actor>();
	assert(p_actor);

	auto i_portal = i.cref<portal>();
	assert(i_portal);

	if(auto i_lock = i.ref<lockable>())
	{
		bool has_key = false;
		for(intity g: p_actor->gear)
		{
			if(has_key) break;
			if(auto g_cont = g.ref<container>())
			{
				for(intity ci: g_cont->items)
				{
					if(auto ci_key = ci.ref<key>())
					{
						if(ci_key->pattern == i_lock->key_pattern)
						{
							has_key = true;
							break;
						}
					}
				}
			}
		}
		if(has_key)
		{
			i_lock->locked = lock;
			if(auto p_io = p.ref<io>())
			{
				p_io->delimit();
				brief_description(game, p, i);
				p_io->out << " successfully";
				if(lock) p_io->out << " locked.";
				else p_io->out << " unlocked.";
				
				p_io->out << std::endl;
			}
		}
		else
		{
			if(auto p_io = p.ref<io>())
			{
				p_io->delimit();
				p_io->out << "You don't have the key to ";
				brief_description(game, p, i);
				p_io->out << "!";
				p_io->out << std::endl;
			}
		}
	}
}

void go_through_portal(game_data& game, intity p, intity i)
{
	auto p_actor = p.ref<actor>();
	assert(p_actor);

	auto i_portal = i.cref<portal>();
	assert(i_portal);

	if(auto i_lockable = i.ref<lockable>())
	{
		if(i_lockable->locked)
		{
			if(auto p_io = p.ref<io>())
			{
				p_io->delimit();
				brief_description(game, p, i);
				p_io->out << " is locked.";
				p_io->out << std::endl;
			}
			return;
		}
	}

	if(i_portal->from == p_actor->current_location)
	{
		p_actor->current_location = i_portal->to;
	}
	else if(i_portal->to == p_actor->current_location)
	{
		p_actor->current_location = i_portal->from;
	}
	else
	{
		assert(!"Location navigation error!");
	}
	entered_location(game, p);
}

void use_entity_io(game_data& game, intity p, intity i)
{
	auto p_io = p.ref<io>();
	if(!p_io) return;

	p_io->new_screen();

	char def_action = '\0';
	std::string allowed;
	p_io->out << " Actions:" << std::endl;

	if(i.has<portal>())
	{
		if(auto i_lockable = i.ref<lockable>())
		{
			if(i_lockable->locked)
			{
				allowed.push_back('U');
				if(!def_action) def_action = 'U';
				p_io->out << "  U   Unlock" << std::endl;
			}
			else
			{
				allowed.push_back('L');
				p_io->out << "  L   Lock" << std::endl;
			}
		}
		allowed.push_back('G');
		if(!def_action) def_action = 'G';
		p_io->out << "  G   Go through" << std::endl;
	}

	allowed.push_back('X');
	p_io->out << "  X   Cancel" << std::endl;

	p_io->delimit();

	std::stringbuf line;
	p_io->in.get(line);

	if(p_io->in.eof()) return;
	if(p_io->in.bad()) return;

	std::stringstream cmd;

	if(p_io->in.good())
	{
		cmd.str(line.str());
	}
	else
	{
		p_io->in.clear();
		cmd.str(std::string(1, def_action));
	}
	p_io->in.ignore();

	bool invalid_option = false;

	if(allowed.find(std::toupper(cmd.peek())) == std::string::npos)
	{
		invalid_option = true;
	}
	else switch(cmd.peek())
	{
		case 'X':
		case 'x': break;
		case 'G':
		case 'g': return go_through_portal(game, p, i);
		case 'L':
		case 'l': return lock_portal(game, p, i, true);
		case 'U':
		case 'u': return lock_portal(game, p, i, false);

		default: invalid_option = true;
	}
	if(invalid_option)
	{
		p_io->out << "Invalid option '";
		p_io->out << line.str();
		p_io->out << "', try again.";
		p_io->out << std::endl;
	}
}

void use_entity(game_data& game, intity player, intity item)
{
	use_entity_io(game, player, item);
}
