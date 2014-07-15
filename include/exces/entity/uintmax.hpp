/**
 *  @file exces/entity/uintmax.hpp
 *  @brief Implementation of Entity using uintmax_t as identifier
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_ENTITY_UINTMAX_1212101511_HPP
#define EXCES_ENTITY_UINTMAX_1212101511_HPP

#include <cstdint>
#include <cassert>
#include <iostream>

namespace exces {

class uintmax_entity
{
private:
	uintmax_t _id;

	static uintmax_t _gen_id(void)
	{
		static uintmax_t id = 0;
		assert(id != UINTMAX_MAX);
		return ++id;
	}
public:
	uintmax_entity(uintmax_t init)
	 : _id(init)
	{ }

	uintmax_entity(void)
	 : _id(_gen_id())
	{ }

	static uintmax_entity nil(void)
	{
		return uintmax_entity(0);
	}

	friend bool operator == (uintmax_entity e1, uintmax_entity e2)
	{
		return e1._id == e2._id;
	}

	friend bool operator != (uintmax_entity e1, uintmax_entity e2)
	{
		return e1._id != e2._id;
	}

	friend bool operator <  (uintmax_entity e1, uintmax_entity e2)
	{
		return e1._id <  e2._id;
	}

	friend bool operator <= (uintmax_entity e1, uintmax_entity e2)
	{
		return e1._id <= e2._id;
	}

	friend bool operator >  (uintmax_entity e1, uintmax_entity e2)
	{
		return e1._id >  e2._id;
	}

	friend bool operator >= (uintmax_entity e1, uintmax_entity e2)
	{
		return e1._id >= e2._id;
	}

	friend std::ostream& operator << (std::ostream& out, uintmax_entity e)
	{
		out << "{" << e._id << "}";
		return out;
	}
};

} // namespace exces

#endif //include guard

