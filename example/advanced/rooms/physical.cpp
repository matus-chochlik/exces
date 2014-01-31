/**
 *  @example advanced/rooms/physical.cpp
 *  @brief The components storing physical properties.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include "physical.hpp"
#include "components.hpp"

float get_weight(intity i)
{
	float po_wgt = 0;
	if(auto i_phy_obj = i.cref<physical_object>())
	{
		po_wgt = i_phy_obj->weight;
	}

	float c_wgt = 0;
	if(auto i_cont = i.cref<container>())
	{
		for(const entity& ce : i_cont->items)
			c_wgt += get_weight(ce);
	}
	float grav_mod = 1;

	if(auto i_gr_mod = i.cref<gravity_modifier>())
	{
		grav_mod = i_gr_mod->coef;
	}

	return po_wgt + c_wgt*grav_mod;
}

float get_volume(intity i)
{

	float po_vol = 0;
	if(auto i_phy_obj = i.ref<physical_object>())
	{
		po_vol = i_phy_obj->volume;
	}

	float c_vol = 0;
	float grow = 0;

	if(auto i_cont = i.ref<container>())
	{
		grow = i_cont->grow_factor;

		for(const entity& ce : i_cont->items)
			c_vol += get_volume(ce);
	}

	return po_vol + ((c_vol > po_vol)?(c_vol-po_vol)*grow:0.0f);
}

