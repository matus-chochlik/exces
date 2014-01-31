/**
 *  @file exces/implicit.hpp
 *  @brief Implements implicit manager and entity
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_IMPLICIT_1401201943_HPP
#define EXCES_IMPLICIT_1401201943_HPP

#include <exces/manager.hpp>

namespace exces {

/// Installs/uninstalls an implicit manager for a component Group
/** The basic manager and entity template classes are designed to be
 *  efficient and flexible. The entities do not store any references
 *  to their managers which makes them very lightweight and usable with
 *  multiple managers or even multiple component groups. This makes the
 *  design quite flexible but also a little inconvenient to use.
 *  Whenever the entity's state is changed, the caller must explicitly
 *  specify which manager to use even in applications which use only
 *  a single manager (which is probably the majority). The implicit manager
 *  and entity templates make the usage of exces more convenient in such cases.
 *
 *  The implicit_manager template class maintains a stack of references to
 *  managers and the implicit_entity template uses the manager at the top
 *  of the stack to do changes to itself. Applications then create a new
 *  instance of manager, 'install' this manager using the implicit_manager
 *  template and use the implicit_entities that wrap some of the manager's
 *  functions to make entity manipulation more convenient:
 *
 *  @code
 *  manager<> the_manager;
 *  implicit_manager<> install(the_manager);
 *
 *  // e will use the_manager
 *  implicit_entity<> e;
 *  e.add(some_component());
 *  f1(e.read<some_component>());
 *  f2(e.write<some_component>());
 *  e.remove<some_component>();
 *  @endcode
 *
 *  @see implicit_entity
 */
template <typename Group = default_group>
class implicit_manager
{
private:
	manager<Group>& _m;
public:
	static void _instantiate(void);

	/// Installs the manager m as the current implicit manager
	implicit_manager(manager<Group>& m);

	/// Uninstalls the manager installed in constructor
	~implicit_manager(void);

	/// Gets a reference to the current implicit manager
	static manager<Group>& get(void);
};

/// Template for entity using the implicit manager to perform operations on itself
/**
 *  @see implicit_manager
 */
template <typename Base, typename Group>
struct implicit_entity_tpl : Base
{
private:
	const Base& self(void) const
	{
		return *this;
	}

	static manager<Group>& _m(void)
	{
		return implicit_manager<Group>::get();
	}
public:
	/// Default construction
	implicit_entity_tpl(void) = default;

	/// Construction from basic entity type
	implicit_entity_tpl(Base e)
	 : Base(e)
	{ }

	/// Adds the specified components on this entity
	template <typename ... Components>
	implicit_entity_tpl& add(Components&& ... components)
	{
		_m().template add(
			self(),
			std::forward<Components>(components)...
		);
		return *this;
	}

	/// Replaces the specified components on this entity
	template <typename ... Components>
	implicit_entity_tpl& replace(Components&& ... components)
	{
		_m().template replace(
			self(),
			std::forward<Components>(components)...
		);
		return *this;
	}

	/// Removes the specified Components from this entity
	template <typename ... Components>
	implicit_entity_tpl& remove(void)
	{
		_m().template remove<Components...>(self());
		return *this;
	}

	/// Returns true if this entity has the specified Component
	template <typename Component>
	bool has(void) const
	{
		return _m().template has<Component>(self());
	}

	/// Returns true if this entity has all the specified Components
	template <typename ... Components>
	bool has_all(void) const
	{
		return _m().template has_all<Components...>(self());
	}

	/// Returns true if this entity has some of the specified Components
	template <typename ... Components>
	bool has_some(void) const
	{
		return _m().template has_some<Components...>(self());
	}

	/// Returns a raw reference to the specified Component
	/** 
	 *  @pre this->has<Component>()
	 *
	 *  @warning This function is not thread safe, see the documentation
	 *  for the manager's raw_access function.
	 *
	 *  @see manager<Group>::raw_access
	 */
	template <typename Component>
	Component& rw(void) const
	{
		return _m().template rw<Component>(self());
	}

	/// Returns a const smart reference to the specified Component
	template <typename Component>
	shared_component<Group, Component, component_access_read_only>
	cref(void) const
	{
		return _m().template cref<Component>(self());
	}

	/// Returns a smart reference to the specified Component
	template <typename Component>
	shared_component<Group, Component, component_access_read_write>
	ref(void) const
	{
		return _m().template ref<Component>(self());
	}

	template <typename ... Components>
	static void copy(implicit_entity_tpl& a, implicit_entity_tpl& b)
	{
		_m().template copy<Components...>(a, b);
	}
};

template <typename ... Components, typename Base, typename Group>
inline void copy(
	implicit_entity_tpl<Base, Group>& a,
	implicit_entity_tpl<Base, Group>& b
)
{
	implicit_entity_tpl<Base, Group>::template copy<Components...>(a, b);
}

/// Entity using the implicit manager to perform operations on itself
/**
 *  @see implicit_manager
 */
template <typename Group = default_group>
struct implicit_entity
 : implicit_entity_tpl<typename entity<Group>::type, Group>
{
private:
	typedef implicit_entity_tpl<typename entity<Group>::type, Group> _tpl;
public:
	implicit_entity(void) = default;

	implicit_entity(typename entity<Group>::type e)
	 : _tpl(e)
	{ }
};

} // namespace exces

#endif //include guard

