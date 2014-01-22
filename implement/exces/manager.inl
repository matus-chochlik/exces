/**
 *  @file exces/manager.inl
 *  @brief Implementation of manager functions
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

namespace exces {

//------------------------------------------------------------------------------
// forced instantiation
//------------------------------------------------------------------------------
template <typename Group>
void
manager<Group>::
_instantiate(void)
{
	aux_::component_bitset<Group> cb;
	aux_::component_index_map<Group> cim;
	const aux_::component_index_map<Group>& ccim = cim;
	cim.get(cb);
	ccim.get(cb);
	
	manager<Group> m;
}
//------------------------------------------------------------------------------

} // namespace exces

