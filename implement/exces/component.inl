/**
 *  @file exces/component.inl
 *  @brief Implementation of component-related functions
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

namespace exces {
namespace aux_ {

//------------------------------------------------------------------------------
// component_index_map
//------------------------------------------------------------------------------
template <typename Group>
const typename component_index_map<Group>::index_vector&
component_index_map<Group>::
get(const component_bitset<Group>& bits) const
{
	typename _index_map::const_iterator p = _indices.find(bits);
	assert(p != _indices.end());
	return p->second;
}
//------------------------------------------------------------------------------
template <typename Group>
const typename component_index_map<Group>::index_vector&
component_index_map<Group>::
get(const component_bitset<Group>& bits)
{
	std::pair<
		typename _index_map::iterator,
		bool
	> r = _indices.insert(
		typename _index_map::value_type(
			bits,
			index_vector()
		)
	);
	assert(r.first != _indices.end());
	// if a new element was inserted
	if(r.second)
	{
		_component_index_t i = 0;
		for(std::size_t j=0; j!=_component_count(); ++j)
		{
			if(bits[j]) r.first->second[j] = i++;
		}
	}
	return r.first->second;
}
//------------------------------------------------------------------------------
} // namespace aux_
} // namespace exces

