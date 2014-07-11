/**
 *  @file exces/entity/string.hpp
 *  @brief Implementation of Entity using string as identifier
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_ENTITY_STRING_1212101511_HPP
#define EXCES_ENTITY_STRING_1212101511_HPP

#include <string>
#include <cassert>
#include <iostream>

namespace exces {

class string_entity
{
private:
	std::string _id;
public:
	string_entity(std::string&& id)
	 : _id(std::move(id))
	{ }

	friend bool operator == (const string_entity& e1, const string_entity& e2)
	{
		return e1._id == e2._id;
	}

	friend bool operator != (const string_entity& e1, const string_entity& e2)
	{
		return e1._id != e2._id;
	}

	friend bool operator <  (const string_entity& e1, const string_entity& e2)
	{
		return e1._id <  e2._id;
	}

	friend bool operator <= (const string_entity& e1, const string_entity& e2)
	{
		return e1._id <= e2._id;
	}

	friend bool operator >  (const string_entity& e1, const string_entity& e2)
	{
		return e1._id >  e2._id;
	}

	friend bool operator >= (const string_entity& e1, const string_entity& e2)
	{
		return e1._id >= e2._id;
	}

	friend std::ostream& operator << (std::ostream& out, const string_entity& e)
	{
		out << "{" << e._id << "}";
		return out;
	}
};

} // namespace exces

#endif //include guard

