/**
 *  @example advanced/rooms/initialization.cpp
 *  @brief Initialization of the game world.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "components.hpp"
#include "gameplay.hpp"

void init_game(game_data& game)
{
	game.world.add(name("World"), location());
	auto wlr = game.world.ref<location>();

	// lock/key patterns
	int no_key_patt = -1;
	int veranda_kp = 1;
	int basement_kp = 2;

	intity yard;
	{
		yard.add(name("yard"), location());
		wlr->locations.insert(yard);
		auto ylr = yard.ref<location>();

		intity driveway;
		{
			driveway.add(name("driveway"), location());
			ylr->locations.insert(driveway);
		}
		intity().add(portal(yard, driveway));

		intity house;
		{
			house.add(name("house"), location());
			ylr->locations.insert(house);
			auto hlr = house.ref<location>();

			intity garage;
			{
				garage.add(name("garage"), location());
				hlr->locations.insert(garage);
			}
			intity garage_door;
			garage_door.add(
				portal(driveway, garage),
				door(),
				lockable(no_key_patt, 1000, true)
			);

			intity veranda;
			{
				veranda.add(name("veranda"), location());
				hlr->locations.insert(veranda);
			}
			intity().add(
				portal(yard, veranda),
				door(),
				lockable(veranda_kp, 100, true)
			);

			intity hall;
			{
				hall.add(name("hall"), location());
				hlr->locations.insert(hall);
			}
			intity().add(portal(veranda, hall));
			intity().add(portal(garage, hall), door());

			intity basement;
			{
				hall.add(name("basement"), location());
				hlr->locations.insert(basement);
			}
			intity().add(
				portal(basement, hall),
				door(),
				lockable(basement_kp, 25, true)
			);

			intity kitchen;
			{
				kitchen.add(name("kitchen"), location());
				hlr->locations.insert(kitchen);
			}
			intity().add(portal(kitchen, hall));

			intity living_room;
			{
				living_room.add(name("living room"), location());
				hlr->locations.insert(living_room);
			}
			intity().add(portal(living_room, hall));
			intity().add(portal(living_room, kitchen));
			intity().add(
				portal(living_room, yard),
				door(),
				lockable(no_key_patt, 1000, true)
			);
		}

		intity shed;
		{
			shed.add(name("shed"), location());
			ylr->locations.insert(shed);
		}
		intity().add(portal(yard, shed), door());

		intity sandbox;
		{
			sandbox.add(name("sandbox"), location());
			ylr->locations.insert(sandbox);
		}
		intity().add(portal(yard, sandbox));
	}

	game.player.add(name("Player"), actor());
	{
		intity throusers;
		throusers.add(
			name("throusers"),
			container(2.0f, 1.0f, 0.0f)
		);
		intity shirt;
		shirt.add(
			name("hawaii shirt"),
			container(0.5f, 0.5f, 0.0f)
		);

		intity veranda_door_key;
		{
			veranda_door_key.add(
				name("veranda door key"),
				key(veranda_kp),
				physical_object(0.005, 0.005)
			);
			auto tcr = throusers.ref<container>();
			tcr->items.insert(veranda_door_key);
		}

		auto par = game.player.ref<actor>();
		par->current_location = yard;
		par->gear.push_back(throusers);
		par->gear.push_back(shirt);
	}

}
