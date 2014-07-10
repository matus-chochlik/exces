/**
 *  @file exces/entity.hpp
 *  @brief Implements entity
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_ENTITY_1212101511_HPP
#define EXCES_ENTITY_1212101511_HPP

#include <exces/group.hpp>
#include <exces/entity/default.hpp>

namespace exces {

/// Template metafunction that returns the entity type for the specified group
template <typename Group = default_group>
struct entity
{
	typedef default_entity type;
};

} // namespace exces

#define EXCES_USE_ENTITY_TYPE(GROUP, ENTITY_TYPE) \
namespace exces { \
template <> \
struct entity<EXCES_GROUP_SEL(GROUP)> \
{ \
	typedef ENTITY_TYPE##_entity type; \
};\
}

#endif //include guard

