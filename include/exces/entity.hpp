/**
 *  @file exces/entity.hpp
 *  @brief Implements entity
 *
 *  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_ENTITY_1212101511_HPP
#define EXCES_ENTITY_1212101511_HPP

#include <exces/group.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

namespace exces {

struct uuid_entity
 : ::boost::uuids::uuid
{
	uuid_entity(void)
	 : ::boost::uuids::uuid( ::boost::uuids::random_generator()())
	{ }
};


/// Template metafunction that returns the entity type for the specified group
template <typename Group = default_group>
struct entity
{
	typedef uuid_entity type;
};

} // namespace exces

#endif //include guard

