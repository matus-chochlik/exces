/**
 *  @example advanced/rooms/container.cpp
 *  @brief The container component.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "container.hpp"
#include "components.hpp"

float container::occ_weight(void) const
{
	float result = 0.0f;
	for(const entity& e : items)
	{
		result += get_weight(e);
	}
	return result;
}

float container::occ_volume(void) const
{
	float result = 0.0f;
	for(const entity& e : items)
	{
		result += get_volume(e);
	}
	return result;
}

float container::rem_weight(void) const
{
	return max_weight-occ_weight();
}

float container::rem_volume(void) const
{
	return max_volume-occ_volume();
}

bool container_contains(intity c, std::function<bool(entity)> pred)
{
	if(auto c_cont = c.cref<container>())
	{
		for(intity i: c_cont->items)
		{
			if(pred(i) || container_contains(i, pred))
				return true;
		}
	}
	return false;
}

bool put_into(intity c, intity i)
{
	if(auto c_cont = c.ref<container>())
	{
		if(auto i_phy_obj = i.cref<physical_object>())
		{
			if(c_cont->max_items <= c_cont->items.size())
				return false;
			if(c_cont->rem_weight() < i_phy_obj->weight)
				return false;
			if(c_cont->rem_volume() < i_phy_obj->volume)
				return false;

			return c_cont->items.insert(i).second;
		}
	}
	return false;
}
