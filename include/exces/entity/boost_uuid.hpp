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
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace exces {

struct boost_uuid_entity
 : ::boost::uuids::uuid
{
	boost_uuid_entity(void)
	 : ::boost::uuids::uuid( ::boost::uuids::random_generator()())
	{ }
};

} // namespace exces

#endif //include guard

