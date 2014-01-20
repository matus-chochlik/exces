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
	implicit_manager(manager<Group>& m)
	 : _m(m)
	{
		_mgr_stack().push(&_m);
	}

	~implicit_manager(void)
	{
		assert(!_mgr_stack().empty());
		assert(_mgr_stack().top() == &_m);
		_mgr_stack().pop();
	}

	static manager<Group>& get(void)
	{
		assert(!_mgr_stack().empty());
		assert(_mgr_stack().top() != nullptr);
		return *_mgr_stack().top();
	}
};

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
	implicit_entity(void) = default;

	implicit_entity(typename entity<Group>::type e)
	 : entity<Group>::type(e)
	{ }

	template <typename ... Components>
	implicit_entity& add(Components ... components)
	{
		_m().add(self(), components...);
		return *this;
	}

	template <typename ... Components>
	implicit_entity& replace(Components ... components)
	{
		_m().replace(self(), components...);
		return *this;
	}

	template <typename ... Components>
	implicit_entity& remove(void)
	{
		_m().remove<Components...>(self());
		return *this;
	}

	template <typename Component>
	bool has(void) const
	{
		return _m().has<Component>(self());
	}

	template <typename ... Components>
	bool has_all(void) const
	{
		return _m().has_all<Components...>(self());
	}

	template <typename ... Components>
	bool has_some(void) const
	{
		return _m().has_some<Components...>(self());
	}

	template <typename Component>
	shared_component<Component, Group> ref(void) const
	{
		return _m().ref<Component>(self());
	}

	template <typename Component>
	const Component& read(void) const
	{
		return _m().read<Component>(self());
	}

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

