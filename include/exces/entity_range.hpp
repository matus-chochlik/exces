/**
 *  @file exces/entity_range.hpp
 *  @brief Implements an entity range template
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_ENTITY_RANGE_1212101457_HPP
#define EXCES_ENTITY_RANGE_1212101457_HPP

#include <exces/entity.hpp>
#include <exces/component.hpp>

#include <functional>

namespace exces {

template <typename Group>
class manager;

/// Implementation of common entity range functions
template <typename Group, typename BaseRange>
class entity_range_tpl
 : public BaseRange
{
private:
	manager<Group>& _manager;
	
	typedef typename manager<Group>::entity_key entity_key;

	typedef std::function<bool (manager<Group>&, entity_key)> _pred_t;
	_pred_t _pred;

	bool _satisfies(void) const
	{
		if(_pred)
		{
			return _pred(_manager, BaseRange::front());
		}
		return true;
	}

	void _skip(void)
	{
		while(!BaseRange::empty() && !_satisfies())
		{
			BaseRange::next();
		}
	}
public:
	entity_range_tpl(
		manager<Group>& man,
		const BaseRange& base,
		const _pred_t& pred = _pred_t()
	): BaseRange(base)
	 , _manager(man)
	 , _pred(pred)
	{
		_skip();
	}

	/// Returns true if the entity at the front has the Component
	template <typename Component>
	const bool has(void) const
	{
		return _manager.template has<Component>(BaseRange::front());
	}

	/// Returns a reference to Component of the entity at front of the range
	template <typename Component>
	shared_component<Component, Group> ref(void) const
	{
		return _manager.template ref<Component>(BaseRange::front());
	}

	/// Returns a reference to Component of the entity at front of the range
	template <typename MemVarType, typename Component>
	shared_component_mem_var<MemVarType, Component, Group>
	mv(MemVarType Component::* mem_var_ptr) const
	{
		return _manager.template mv<MemVarType, Component>(
			BaseRange::front(),
			mem_var_ptr
		);
	}

	/// Moves the front of the range one element ahead
	void next(void)
	{
		assert(!BaseRange::empty());
		BaseRange::next();
		_skip();
	}
};

} // namespace exces

#endif //include guard

