/**
 *  @example advanced/rooms/description.cpp
 *  @brief Implementation of entity description.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <exces/entity_io.hpp>

#include "components.hpp"
#include "gameplay.hpp"
#include "description.hpp"


void entity_description_io(
	game_data& game,
	intity p,
	intity i,
	bool full_desc
)
{
	auto p_io = p.ref<io>();
	if(!p_io) return;

	bool some_desc = false;
	bool some_name = false;

	if(auto i_lockable = i.cref<lockable>())
	{
		p_io->out << "lockable";
		if(full_desc)
		{
			if(i_lockable->locked)
			{
				p_io->out << " locked";
			}
			else
			{
				p_io->out << " unlocked";
			}
		}
		some_desc = true;
	}

	if(auto i_name = i.cref<name>())
	{
		if(some_desc) p_io->out << " ";
		else p_io->out << "the ";
		p_io->out << i_name.get().self();
		some_desc = true;
		some_name = true;
	}
	
	if(auto i_portal = i.cref<portal>())
	{
		if(some_desc) p_io->out << " ";
		else p_io->out << "a ";

		if(!some_name)
		{
			if(i.has<door>())
			{
				p_io->out << "door";
			}
			else
			{
				p_io->out << "passage";
			}
			some_name = true;
			some_desc = true;
		}
		
		if(auto p_actor = p.ref<actor>())
		{
			if(full_desc)
			{
				p_io->out << " leading";
			}
			if(i_portal->from == p_actor->current_location)
			{
				p_io->out << " to ";
				brief_description(game, p, i_portal->to);
			}
			else if(i_portal->to == p_actor->current_location)
			{
				p_io->out << " from ";
				brief_description(game, p, i_portal->from);
			}
			some_desc = true;
		}
	}

	if(!some_desc)
	{
		p_io->out << "entity {" << i << "}";
	}

	if(full_desc)
	{
		if(auto i_desc = i.ref<description>())
		{
			if(some_desc) p_io->out << std::endl;
			p_io->out << i_desc.get().self();
			some_desc = true;
		}
	}
}

void brief_description(game_data& game, intity player, intity item)
{
	entity_description_io(game, player, item, false);
}

void full_description(game_data& game, intity player, intity item)
{
	entity_description_io(game, player, item, true);
}
