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
		else if(!full_desc) p_io->out << "the ";
		p_io->out << i_name.get().self();
		some_desc = true;
		some_name = true;
	}

	if(auto i_type = i.cref<item_type>())
	{
		intity it = i_type.get().self();
		if(auto it_name = it.cref<name>())
		{
			if(some_desc) p_io->out << " ";
			else p_io->out << "a ";
			p_io->out << it_name.get().self();
			some_desc = true;
			some_name = true;
		}
	}

	if(auto i_portal = i.cref<portal>())
	{
		if(!some_name)
		{
			if(some_desc) p_io->out << " ";
			p_io->out << "a passage";
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
		if(auto i_gear = i.cref<gear_slots>())
		{
			auto i_gear_l = i_gear.get();
			if(!i_gear_l->items.empty())
			{
				p_io->out << " occupied by ";
				auto ii = i_gear_l->items.begin();
				if(ii != i_gear_l->items.end())
				{
					brief_description(game, p, *ii);
					++ii;
					while(ii != i_gear_l->items.end())
					{
						auto ji = ii++;
						if(ii == i_gear_l->items.end())
							p_io->out << " and ";
						else p_io->out << ", ";
						brief_description(game, p, *ji);
					}
				}
				p_io->out << ".";
				some_desc = true;
			}
		}
		if(auto i_cont = i.cref<container>())
		{
			auto i_cont_l = i_cont.get();
			if(!i_cont_l->items.empty())
			{
				p_io->out << " containing ";
				auto ii = i_cont_l->items.begin();
				if(ii != i_cont_l->items.end())
				{
					brief_description(game, p, *ii);
					++ii;
					while(ii != i_cont_l->items.end())
					{
						auto ji = ii++;
						if(ii == i_cont_l->items.end())
							p_io->out << " and ";
						else p_io->out << ", ";
						brief_description(game, p, *ji);
					}
				}
				p_io->out << ".";
				some_desc = true;
			}
		}

		if(auto i_desc = i.ref<description>())
		{
			if(some_desc) p_io->newl();
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
