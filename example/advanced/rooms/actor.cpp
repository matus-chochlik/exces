/**
 *  @example advanced/rooms/actor.cpp
 *  @brief Implementation of actor functions.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "components.hpp"
#include "gameplay.hpp"

void use_inventory_io(game_data& game, intity p)
{
	auto p_io = p.ref<io>();
	if(!p_io) return;

	auto p_actor = p.ref<actor>();
	if(!p_actor) return;

	p_io->new_screen();

	full_description(game, p, p);
	p_io->newl();
	p_io->delimit();

	p_io->out << " Slots:";
	p_io->newl();

	unsigned si = 0;
	unsigned ci = 0;
	unsigned ii = 0;

	for(const auto& bpi: p_actor->body_parts)
	{
		intity bp = bpi.second;
		p_io->out << "  Slot ";
		p_io->out << si++;
		p_io->out << ": ";
		full_description(game, p, bp);
		p_io->newl();

		auto bp_gs = bp.ref<gear_slots>();
		if(!bp_gs) continue;

		for(intity gs: bp_gs->items)
		{
			auto gs_cont = gs.ref<container>();
			if(!gs_cont) continue;
			if(gs_cont->items.empty()) continue;

			p_io->out << "   Container ";
			p_io->out << ci++;
			p_io->out << ": ";
			brief_description(game, p, gs);
			p_io->newl();

			for(intity gs_ci: gs_cont->items)
			{
				p_io->out << "    Item ";
				p_io->out << ii++;
				p_io->out << ": ";
				brief_description(game, p, gs_ci);
				p_io->newl();
			}
		}
	}

	p_io->delimit();
}

void use_inventory(game_data& game, intity player)
{
	use_inventory_io(game, player);
}

