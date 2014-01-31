/**
 *  @example advanced/rooms/locking.hpp
 *  @brief The lock and key components
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef EXCES_EXAMPLE_ADVANCED_ROOMS_LOCKING_HPP
#define EXCES_EXAMPLE_ADVANCED_ROOMS_LOCKING_HPP

#include "common.hpp"

struct key
{
	int pattern;

	key(int pat)
	 : pattern(pat)
	{ }

	friend bool operator == (key a, key b) { return a.pattern == b.pattern; }
	friend bool operator <  (key a, key b) { return a.pattern <  b.pattern; }
};

struct lockable
{
	int key_pattern;
	int strength;
	bool locked;

	lockable(int kp, int s, bool l)
	 : key_pattern(kp)
	 , strength(s)
	 , locked(l)
	{ }
};

#endif // include guard
