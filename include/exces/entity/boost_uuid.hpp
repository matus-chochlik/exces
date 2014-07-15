/**
 *  @file exces/entity/boost_uuid.hpp
 *  @brief Boost.UUID-based implementation of Entity
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_ENTITY_BOOST_UUID_1212101511_HPP
#define EXCES_ENTITY_BOOST_UUID_1212101511_HPP

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/random_generator.hpp>

namespace exces {

struct boost_uuid_entity
 : ::boost::uuids::uuid
{
	explicit boost_uuid_entity(::boost::uuids::uuid&& init)
	 : ::boost::uuids::uuid(std::move(init))
	{ }

	boost_uuid_entity(void)
	 : ::boost::uuids::uuid(::boost::uuids::random_generator()())
	{ }

	static boost_uuid_entity nil(void)
	{
		return boost_uuid_entity(::boost::uuids::nil_generator()());
	}
};

} // namespace exces

#endif //include guard

