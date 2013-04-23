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

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

namespace exces {

struct entity
 : ::boost::uuids::uuid
{
	entity(void)
	 : ::boost::uuids::uuid( ::boost::uuids::random_generator()())
	{ }
};

} // namespace exces

#endif //include guard

