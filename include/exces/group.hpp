/**
 *  @file exces/group.hpp
 *  @brief Implements component groups
 *
 *  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_GROUP_1212101431_HPP
#define EXCES_GROUP_1212101431_HPP

#include <exces/global_list.hpp>

#define EXCES_GROUP_SEL_UNQ(GROUP) _group_##GROUP##_sel 
#define EXCES_GROUP_SEL(GROUP) ::exces:: EXCES_GROUP_SEL_UNQ(GROUP)

#define EXCES_REG_GROUP(GROUP) \
namespace exces { \
	struct EXCES_GROUP_SEL_UNQ(GROUP) { }; \
	EXCES_REGISTER_COUNTER_SELECTOR(EXCES_GROUP_SEL(GROUP)) \
}

EXCES_REG_GROUP(default)

namespace exces {

typedef EXCES_GROUP_SEL(default) default_group;

template <typename Component, typename Group>
struct component_id;

template <typename Component, typename Group = default_group>
struct component_name;

template <typename Component, typename Group>
struct component_id<const Component, Group>
 : component_id<Component, Group>
{ };

} // namespace exces

#define EXCES_REG_COMPONENT_IN_GROUP(COMPONENT, GROUP) \
namespace exces { \
template <> \
struct component_id< COMPONENT, EXCES_GROUP_SEL(GROUP) > \
 : EXCES_COUNTER_CURRENT(EXCES_GROUP_SEL(GROUP)) \
{ }; \
template <> \
struct component_name< COMPONENT, EXCES_GROUP_SEL(GROUP) > \
{ \
	static const char* c_str(void) { return #COMPONENT ; } \
}; \
EXCES_ADD_TO_GLOBAL_LIST(EXCES_GROUP_SEL(GROUP), COMPONENT) \
}

#define EXCES_REG_COMPONENT(COMPONENT) \
	EXCES_REG_COMPONENT_IN_GROUP(COMPONENT, default)

namespace exces {

template <typename Group = default_group>
struct components
 : EXCES_GET_GLOBAL_LIST(Group)
{ };

template <typename Group = default_group>
struct component_index
{
	// TODO: try some size optimizations here
	typedef std::size_t type;
};

} // namespace exces

#endif //include guard

