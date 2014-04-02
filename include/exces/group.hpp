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

#include <exces/aux_/global_list.hpp>
#include <exces/fwd.hpp>

#include <type_traits>
#include <cstring>

#define EXCES_GROUP_SEL_UNQ(GROUP) _group_##GROUP##_sel 
#define EXCES_GROUP_SEL(GROUP) ::exces:: EXCES_GROUP_SEL_UNQ(GROUP)

/// Registers a new component group, the default group is always pre-registered
/**
 *  @see #EXCES_REG_COMPONENT_IN_GROUP
 *  @see #EXCES_REG_FLYWEIGHT_COMPONENT_IN_GROUP
 */
#define EXCES_REG_GROUP(GROUP) \
namespace exces { \
	struct EXCES_GROUP_SEL_UNQ(GROUP) { }; \
	EXCES_REGISTER_COUNTER_SELECTOR(EXCES_GROUP_SEL(GROUP)) \
}

EXCES_REG_GROUP(default)

namespace exces {

typedef EXCES_GROUP_SEL(default) default_group;

template <typename Component, typename Group = default_group>
struct component_id;

template <typename Component, typename Group>
struct component_id<const Component, Group>
 : component_id<Component, Group>
{ };

template <typename Component>
struct component_name;

template <typename Component, typename Group = default_group>
struct component_kind
 : component_kind_normal
{ };

} // namespace exces

#define EXCES_REG_COMPONENT_IN_GROUP_BEGIN(COMPONENT, GROUP) \
namespace exces { \
template <> \
struct component_id< COMPONENT, EXCES_GROUP_SEL(GROUP) > \
 : EXCES_COUNTER_CURRENT(EXCES_GROUP_SEL(GROUP)) \
{ }; \
EXCES_ADD_TO_GLOBAL_LIST(EXCES_GROUP_SEL(GROUP), COMPONENT) \

#define EXCES_REG_COMPONENT_IN_GROUP_END(COMPONENT, GROUP) \
}

/// Registers the specified component in the specifed group
/**
 *  @see #EXCES_REG_GROUP
 *  @see #EXCES_REG_FLYWEIGHT_COMPONENT_IN_GROUP
 *  @see #EXCES_REG_FLYWEIGHT_COMPONENT
 */
#define EXCES_REG_COMPONENT_IN_GROUP(COMPONENT, GROUP) \
	EXCES_REG_COMPONENT_IN_GROUP_BEGIN(COMPONENT, GROUP) \
	EXCES_REG_COMPONENT_IN_GROUP_END(COMPONENT, GROUP)

/// Registers a backbuffered component in the specified group
/**
 *  @see #EXCES_REG_GROUP
 *  @see #EXCES_REG_COMPONENT_IN_GROUP
 *  @see #EXCES_REG_COMPONENT
 */
#define EXCES_REG_BACKBUF_COMPONENT_IN_GROUP(COMPONENT, GROUP) \
	EXCES_REG_COMPONENT_IN_GROUP_BEGIN(COMPONENT, GROUP) \
	template <> struct component_kind<\
		COMPONENT, \
		EXCES_GROUP_SEL(GROUP) \
	> : component_kind_backbuf \
	{ }; \
	EXCES_REG_COMPONENT_IN_GROUP_END(COMPONENT, GROUP)

/// Registers a flyweight component in the specified group
/**
 *  @see #EXCES_REG_GROUP
 *  @see #EXCES_REG_COMPONENT_IN_GROUP
 *  @see #EXCES_REG_COMPONENT
 */
#define EXCES_REG_FLYWEIGHT_COMPONENT_IN_GROUP(COMPONENT, GROUP) \
	EXCES_REG_COMPONENT_IN_GROUP_BEGIN(COMPONENT, GROUP) \
	template <> struct component_kind<\
		COMPONENT, \
		EXCES_GROUP_SEL(GROUP) \
	> : component_kind_flyweight \
	{ }; \
	EXCES_REG_COMPONENT_IN_GROUP_END(COMPONENT, GROUP)

/// Registers the specified component's name
/** The component names are required for the type-erased any_manager
 *
 *  @ see #EXCES_REG_COMPONENT_WITH_NAME
 */
#define EXCES_REG_COMPONENT_NAME(COMPONENT) \
namespace exces { \
template <> \
struct component_name< COMPONENT > \
{ \
	static const char* c_str(void) { return #COMPONENT ; } \
	static std::size_t size(void) { return sizeof(#COMPONENT)-1 ; } \
};\
} 

/// Registers the specified component in the specifed group
/**
 *  @see #EXCES_REG_GROUP
 *  @see #EXCES_REG_COMPONENT_IN_GROUP
 *  @see #EXCES_REG_FLYWEIGHT_COMPONENT
 */
#define EXCES_REG_COMPONENT(COMPONENT) \
	EXCES_REG_COMPONENT_IN_GROUP(COMPONENT, default)

/// Registers the specified backbuffered component in the specifed group
/**
 *  @see #EXCES_REG_GROUP
 *  @see #EXCES_REG_FLYWEIGHT_COMPONENT_IN_GROUP
 *  @see #EXCES_REG_COMPONENT
 */
#define EXCES_REG_BACKBUF_COMPONENT(COMPONENT) \
	EXCES_REG_BACKBUF_COMPONENT_IN_GROUP(COMPONENT, default)

/// Registers the specified flyweight component in the specifed group
/**
 *  @see #EXCES_REG_GROUP
 *  @see #EXCES_REG_FLYWEIGHT_COMPONENT_IN_GROUP
 *  @see #EXCES_REG_COMPONENT
 */
#define EXCES_REG_FLYWEIGHT_COMPONENT(COMPONENT) \
	EXCES_REG_FLYWEIGHT_COMPONENT_IN_GROUP(COMPONENT, default)

/// Registers the specified component and also registers its name
/**
 *  @see #EXCES_REG_COMPONENT
 *  @see #EXCES_REG_COMPONENT_NAME
 */
#define EXCES_REG_NAMED_COMPONENT(COMPONENT) \
	EXCES_REG_COMPONENT_NAME(COMPONENT) \
	EXCES_REG_COMPONENT(COMPONENT)

/// Registers the specified backbuffered component and also registers its name
/**
 *  @see #EXCES_REG_BACKBUF_COMPONENT
 *  @see #EXCES_REG_COMPONENT_NAME
 */
#define EXCES_REG_NAMED_BACKBUF_COMPONENT(COMPONENT) \
	EXCES_REG_COMPONENT_NAME(COMPONENT) \
	EXCES_REG_BACKBUF_COMPONENT(COMPONENT)

/// Registers the specified flyweight component and also registers its name
/**
 *  @see #EXCES_REG_FLYWEIGHT_COMPONENT
 *  @see #EXCES_REG_COMPONENT_NAME
 */
#define EXCES_REG_NAMED_FLYWEIGHT_COMPONENT(COMPONENT) \
	EXCES_REG_COMPONENT_NAME(COMPONENT) \
	EXCES_REG_FLYWEIGHT_COMPONENT(COMPONENT)

namespace exces {

/// Metafunction returning the sequence of components in the specified group
template <typename Group = default_group>
struct components
 : EXCES_GET_GLOBAL_LIST(Group)
{ };

/// Returns the type for the component index value for the specified group
template <typename Group = default_group>
struct component_index
{
	// TODO: try some size optimizations here
	typedef std::size_t type;

	struct _by_name_hlp
	{
		type& result;
		const char* name;
		std::size_t size;

		_by_name_hlp(type& res, const char* cname)
		 : result(res)
		 , name(cname)
		 , size(std::strlen(name))
		{ }

		template <typename C>
		void operator ()(mp::identity<C>) const
		{
			if(size == component_name<C>::size())
			{
				if(std::strncmp(
					name,
					component_name<C>::c_str(),
					component_name<C>::size()
				) == 0)
				{
					result = component_id<C, Group>();
				}
			}
		}
	};

	static type by_name(const char* name)
	{
		type result = ~type(0);
		_by_name_hlp hlp(result, name);
		mp::for_each<typename components<Group>::type>(hlp);
		return result;
	}
};

} // namespace exces

#endif //include guard

