/**
 *  @file exces/entity_key_set.hpp
 *  @brief Implements entity key set
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_ENTITY_KEY_SET_1212101511_HPP
#define EXCES_ENTITY_KEY_SET_1212101511_HPP

#include <exces/entity.hpp>
#include <algorithm>
#include <vector>

namespace exces {

template <typename Group>
class manager;

// Log(n) insert/removal set of unique entity keys
template <typename Group>
class entity_key_set
{
private:
	typedef typename manager<Group>::entity_key entity_key;
	std::vector<entity_key> _keys;

	static bool _ek_less(entity_key a, entity_key b)
	{
		return a->first < b->first;
	}
public:
	entity_key_set(void) = default;
	entity_key_set(entity_key_set&&) = default;
	entity_key_set(entity_key key)
	 : _keys(1, key)
	{ }

	std::size_t size(void) const
	{
		return _keys.size();
	}

	bool contains(entity_key key)
	{
		auto p = std::lower_bound(
			_keys.begin(),
			_keys.end(),
			key,
			_ek_less
		);
		return ((p != _keys.end()) && (key == *p));
	}

	void insert(entity_key key)
	{
		auto p = std::lower_bound(
			_keys.begin(),
			_keys.end(),
			key,
			_ek_less
		);
		if((p == _keys.end()) || _ek_less(key, *p))
		{
			_keys.insert(p, key);
		}
	}

	void erase(entity_key key)
	{
		auto p = std::lower_bound(
			_keys.begin(),
			_keys.end(),
			key,
			_ek_less
		);
		if(!(p == _keys.end()) && !(_ek_less(key, *p)))
		{
			_keys.erase(p);
		}
	}

	typedef typename std::vector<entity_key>::const_iterator
		const_iterator;

	const_iterator begin(void) const
	{
		return _keys.begin();
	}

	const_iterator end(void) const
	{
		return _keys.end();
	}
};

} // namespace exces

#endif //include guard

