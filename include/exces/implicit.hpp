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

#include <stack>

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
	static std::stack<manager<Group>*>& _mgr_stack(void)
	{
		static std::stack<manager<Group>*> _ms;
		return _ms;
	}

	manager<Group>& _m;
public:
	/// Installs the manager m as the current implicit manager
	implicit_manager(manager<Group>& m)
	 : _m(m)
	{
		_mgr_stack().push(&_m);
	}

	/// Uninstalls the manager installed in constructor
	~implicit_manager(void)
	{
		assert(!_mgr_stack().empty());
		assert(_mgr_stack().top() == &_m);
		_mgr_stack().pop();
	}

	/// Gets a reference to the current implicit manager
	static manager<Group>& get(void)
	{
		assert(!_mgr_stack().empty());
		assert(_mgr_stack().top() != nullptr);
		return *_mgr_stack().top();
	}
};

/// Entity using the implicit manager to perform operations on itself
/**
 *  @see implicit_manager
 */
template <typename Group = default_group>
struct implicit_entity : entity<Group>::type
{
private:
	const typename entity<Group>::type& self(void) const
	{
		return *this;
	}

	static manager<Group>& _m(void)
	{
		return implicit_manager<Group>::get();
	}
public:
	/// Default construction
	implicit_entity(void) = default;

	/// Construction from basic entity type
	implicit_entity(typename entity<Group>::type e)
	 : entity<Group>::type(e)
	{ }

	/// Adds the specified components on this entity
	template <typename ... Components>
	implicit_entity& add(Components ... components)
	{
		_m().add(self(), components...);
		return *this;
	}

	/// Replaces the specified components on this entity
	template <typename ... Components>
	implicit_entity& replace(Components ... components)
	{
		_m().replace(self(), components...);
		return *this;
	}

	/// Removes the specified Components from this entity
	template <typename ... Components>
	implicit_entity& remove(void)
	{
		_m().remove<Components...>(self());
		return *this;
	}

	/// Returns true if this entity has the specified Component
	template <typename Component>
	bool has(void) const
	{
		return _m().has<Component>(self());
	}

	/// Returns true if this entity has all the specified Components
	template <typename ... Components>
	bool has_all(void) const
	{
		return _m().has_all<Components...>(self());
	}

	/// Returns true if this entity has some of the specified Components
	template <typename ... Components>
	bool has_some(void) const
	{
		return _m().has_some<Components...>(self());
	}

	/// Returns a smart reference to the specified Component
	/**
	 *  @pre this->has<Component>()
	 */
	template <typename Component>
	shared_component<Component, Group> ref(void) const
	{
		return _m().ref<Component>(self());
	}

	/// Returns a read-only raw reference to the specifed Component
	/**
	 *  @pre this->has<Component>()
	 */
	template <typename Component>
	const Component& read(void) const
	{
		return _m().read<Component>(self());
	}

	/// Returns a mutable raw reference to the specifed Component
	/**
	 *  @pre this->has<Component>()
	 */
	template <typename Component>
	typename shared_component<Component, Group>::component_ref
	write(void) const
	{
		return _m().write<Component>(self());
	}

	template <typename ... Components>
	static void copy(implicit_entity& a, implicit_entity& b)
	{
		_m().template copy<Components...>(a, b);
	}
};

template <typename ... Components, typename Group>
inline void copy(implicit_entity<Group>& a, implicit_entity<Group>& b)
{
	implicit_entity<Group>::template copy<Components...>(a, b);
}

} // namespace exces

#endif //include guard

