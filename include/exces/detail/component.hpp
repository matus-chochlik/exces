/**
 *  @file exces/detail/component.hpp
 *  @brief Implements component-related helpers
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_AUX_COMPONENT_1212101457_HPP
#define EXCES_AUX_COMPONENT_1212101457_HPP

#include <exces/fwd.hpp>
#include <exces/detail/metaprog.hpp>

#include <map>
#include <array>
#include <bitset>

namespace exces {

namespace detail {

template <typename Group>
struct component_less_mf
{
	template <typename C1, typename C2>
	struct apply
	 : mp::integral_constant<
		bool, (
			component_id<C1, Group>::value<
			component_id<C2, Group>::value
		)
	>{ };
};

template <typename Group, typename Components>
struct sort_components
 : mp::sort<Components, component_less_mf<Group>>
{ };

// less than comparison for bitsets smaller than uint
struct component_bitset_less_small
{
	template <typename Bitset>
	bool operator()(const Bitset& a, const Bitset& b) const
	{
		return a.to_ulong() < b.to_ulong();
	}
};

// less than comparison for bitsets bigger than uint
template <typename Size>
struct component_bitset_less_big
{
	template <typename Bitset>
	bool operator()(const Bitset& a, const Bitset& b) const
	{
		for(std::size_t i=0; i!=Size(); ++i)
		{
			if(a[i] < b[i]) return true;
			if(a[i] > b[i]) return false;
		}
		return false;
	}
};

template <typename Group>
struct component_bitset
 : std::bitset<mp::size<components<Group>>::value>
{ };

template <typename Group>
class component_index_map
{
private:
	// type of the component index (unsigned int with sufficient range)
	typedef typename component_index<Group>::type _component_index_t;
	typedef mp::size<components<Group>> _component_count;
public:
	typedef std::array<
		_component_index_t,
		_component_count::value
	> index_vector;
private:
	// the less than comparison functor to be used
	// when comparing two bitsets
	typedef typename mp::if_c<
		_component_count::value <= sizeof(unsigned long)*8,
		component_bitset_less_small,
		component_bitset_less_big<_component_count>
	>::type component_bitset_less;

	typedef std::map<
		component_bitset<Group>,
		index_vector,
		component_bitset_less
	> _index_map;

	_index_map _indices;
public:
	const index_vector& get(const component_bitset<Group>& bits) const;
	const index_vector& get(const component_bitset<Group>& bits);
};

} // namespace detail
} // namespace exces

#endif //include guard

