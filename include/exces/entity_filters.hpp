/**
 *  @file exces/entity_filters.hpp
 *  @brief Implements entity predicate functors
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_ENTITY_FILTERS_1212101457_HPP
#define EXCES_ENTITY_FILTERS_1212101457_HPP

#include <exces/metaprog.hpp>

namespace exces {

template <typename Group>
class manager;

template <typename Sequence>
struct entity_with_seq
{
	template <typename Group>
	bool operator ()(
		manager<Group>& m,
		typename manager<Group>::entity_key key
	) const
	{
		return m.has_all_seq(key, Sequence());
	}
};

/// Returns true for entities that has all specified Components
template <typename ... Components>
struct entity_with
 : entity_with_seq<mp::typelist<Components...>>
{ };

} // namespace exces

#endif //include guard

