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
	intity a_hoodie;
	{
		a_hoodie.add(name("hoodie"));
	}
	intity some_trousers;
	{
		some_trousers.add(name("pair of trousers"));
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

			intity bathroom1;
			{
				bathroom1.add(
					name("ground floor bathroom"),
					location()
				);
				hlr->locations.insert(bathroom1);
				auto blr = bathroom1.ref<location>();

				intity wicker_basket;
				{
					wicker_basket.add(
						name("wicker basket"),
						container(100, 15.0f, 40.0f, 0.0f),
						physical_object(0.7f, 39.0f)
					);
					auto wbcr = wicker_basket.ref<container>();

					intity hoodie;
					{
						hoodie.add(
							item_type(a_hoodie),
							container(30, 2.0f, 1.0f, 0.1f),
							physical_object(0.2f, 1.4f),
							gear_kind(body_part::torso)
						);
					}
					wbcr->items.insert(hoodie);
				}
			}
			intity().add(
				portal(hall, bathroom1),
				item_type(a_door)
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

	intity neckstrap;
	{
		neckstrap.add(
			name("blue neckstrap"),
			physical_object(0.01f, 0.015f),
			container(5, 0.3f, 0.3f, 0.0f),
			gear_kind(body_part::neck)
		);
	}

	intity veranda_door_key;
	{
		veranda_door_key.add(
			name("veranda door key"),
			key(veranda_kp),
			physical_object(0.005f, 0.005f)
		);
	}
	neckstrap.ref<container>()->items.insert(veranda_door_key);

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
		intity trousers;
		trousers.add(
			item_type(some_trousers),
			container(20, 1.5f, 0.5f, 0.0f),
			gear_kind(body_part::legs)
		);
		intity shirt;
		shirt.add(
			name("hawaii shirt"),
			container(15, 0.5f, 0.3f, 0.0f),
			gear_kind(body_part::torso)
		);

		auto par = game.player.ref<actor>();

		intity head;
		{
			head.add(
				name("head"),
				gear_slots(1)
			);
			par->body_parts[body_part::head] = head;
		}

		intity neck;
		{
			neck.add(
				name("neck"),
				gear_slots(1)
			);
			par->body_parts[body_part::neck] = neck;
			par->body_parts[body_part::neck]
				.ref<gear_slots>()->items.insert(neckstrap);
		}

		intity hands;
		{
			hands.add(
				name("hands"),
				gear_slots(2),
				container(2, 20.0f, 40.0f, 0.0f)
			);
			par->body_parts[body_part::hands] = hands;
		}

		intity legs;
		{
			legs.add(
				name("legs"),
				gear_slots(1)
			);
			par->body_parts[body_part::legs] = legs;
			par->body_parts[body_part::legs]
				.ref<gear_slots>()->items.insert(trousers);
		}

		intity torso;
		{
			torso.add(
				name("torso"),
				gear_slots(3)
			);
			par->body_parts[body_part::torso] = torso;
			par->body_parts[body_part::torso]
				.ref<gear_slots>()->items.insert(shirt);
		}

		intity back;
		{
			back.add(
				name("back"),
				gear_slots(2)
			);
			par->body_parts[body_part::back] = back;
		}

		par->current_location = yard;
	}

}
