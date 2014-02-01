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

	intity a_door;
	{
		a_door.add(name("door"));
	}
	intity a_key;
	{
		a_key.add(name("key"));
	}

	intity toy_shovel;
	{
		toy_shovel.add(
			name("toy shovel"),
			physical_object(0.1f, 0.2f)
		);
	}
	intity toy_rake;
	{
		toy_rake.add(
			name("toy rake"),
			physical_object(0.1f, 0.2f)
		);
	}
	intity toy_bucket;
	{
		toy_bucket.add(
			name("toy bucket"),
			container(30, 1.0f, 0.7f, 0.0f),
			physical_object(0.1f, 0.8f)
		);
		auto tbcr = toy_bucket.ref<container>();
		tbcr->items.insert(toy_shovel);
		tbcr->items.insert(toy_rake);
	}

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
				item_type(a_door),
				lockable(no_key_patt, 1000, true)
			);

			intity veranda;
			{
				veranda.add(name("veranda"), location());
				hlr->locations.insert(veranda);
			}
			intity().add(
				portal(yard, veranda),
				item_type(a_door),
				lockable(veranda_kp, 100, true)
			);

			intity hall;
			{
				hall.add(name("hall"), location());
				hlr->locations.insert(hall);
			}
			intity().add(portal(veranda, hall));
			intity().add(portal(garage, hall), item_type(a_door));

			intity basement;
			{
				hall.add(name("basement"), location());
				hlr->locations.insert(basement);
			}
			intity().add(
				portal(basement, hall),
				item_type(a_door),
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
				item_type(a_door),
				lockable(no_key_patt, 1000, true)
			);
		}

		intity shed;
		{
			shed.add(name("shed"), location());
			ylr->locations.insert(shed);
		}
		intity().add(portal(yard, shed), item_type(a_door));

		intity sandbox;
		{
			sandbox.add(name("sandbox"), location());
			ylr->locations.insert(sandbox);

			auto slr = sandbox.ref<location>();
			{
				slr->items.insert(toy_bucket);
			}
		}
		intity().add(portal(yard, sandbox));
	}

	intity veranda_door_key;
	{
		veranda_door_key.add(
			name("veranda door key"),
			key(veranda_kp),
			physical_object(0.005f, 0.005f)
		);
	}

	intity basement_door_key;
	{
		basement_door_key.add(
			item_type(a_key),
			key(basement_kp),
			physical_object(0.005f, 0.005f)
		);
	}
	toy_bucket.ref<container>()->items.insert(basement_door_key);
	

	game.player.add(name("Player"), actor());
	{
		intity throusers;
		throusers.add(
			name("throusers"),
			container(30, 2.0f, 0.5f, 0.0f)
		);
		intity shirt;
		shirt.add(
			name("hawaii shirt"),
			container(15, 0.5f, 0.3f, 0.0f)
		);

		auto tcr = throusers.ref<container>();
		tcr->items.insert(veranda_door_key);

		auto par = game.player.ref<actor>();
		par->current_location = yard;
		par->hands.add(name("hands"), container(2, 20.0f, 25.0f, 0.0f));
		par->gear.push_back(throusers);
		par->gear.push_back(shirt);
	}

}
