/**
 *  @file exces/any/manager.hpp
 *  @brief Type erasure for entity component manager
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_ANY_ENTITY_MANAGER_1403152314_HPP
#define EXCES_ANY_ENTITY_MANAGER_1403152314_HPP

#include <exces/any/manager_intf.hpp>
#include <array>

namespace exces {

template <typename Entity = entity<default_group>::type>
class any_manager
{
private:
	typedef typename any_entity_key::param_type any_entity_key_param;

	std::shared_ptr<any_manager_intf<Entity>> _pimpl;

	template <typename ... C>
	std::array<const char*, sizeof ... (C) + 1>
	static cnames(void)
	{
		std::array<const char*, sizeof ... (C) + 1> result = {
			component_name<C>::c_str() ...,
			nullptr
		};
		return result;
	}

	template <typename ... C>
	std::array<void*, sizeof ... (C) + 1>
	static cptrs(C& ... c)
	{
		std::array<void*, sizeof ... (C) + 1> result = {
			static_cast<void*>(&c) ...,
			nullptr
		};
		return result;
	}
public:
	template <typename Group>
	any_manager(manager<Group>& mgr)
	 : _pimpl(make_any_manager_impl<Group>(mgr))
	{ }

	typedef any_entity_key entity_key;

	bool has_key(Entity e)
	{
		assert(_pimpl);
		return _pimpl->has_key(e);
	}

	entity_key get_key(Entity e)
	{
		assert(_pimpl);
		return _pimpl->get_key(e);
	}

	Entity get_entity(any_entity_key_param aek)
	{
		assert(_pimpl);
		return _pimpl->get_entity(aek);
	}

	template <typename Component>
	bool has(any_entity_key_param aek)
	{
		assert(_pimpl);
		return _pimpl->has(aek, component_name<Component>::c_str());
	}

	template <typename Component>
	bool has(Entity e)
	{
		assert(_pimpl);
		return _pimpl->has(e, component_name<Component>::c_str());
	}

	template <typename ... Components>
	bool has_all(any_entity_key_param aek)
	{
		assert(_pimpl);
		return _pimpl->has_all(aek, cnames<Components...>().data());
	}

	template <typename ... Components>
	bool has_some(any_entity_key_param aek)
	{
		assert(_pimpl);
		return _pimpl->has_some(aek, cnames<Components...>().data());
	}

	template <typename ... Components>
	any_manager& reserve(std::size_t n)
	{
		assert(_pimpl);
		_pimpl->reserve(n, cnames<Components...>().data());
		return *this;
	}

	template <typename ... Components>
	any_manager& add(any_entity_key_param aek, Components&& ... comp)
	{
		assert(_pimpl);
		_pimpl->add(
			aek,
			cnames<Components...>().data(),
			cptrs<Components...>(comp...).data()
		);
		return *this;
	}

	template <typename ... Components>
	any_manager& add(Entity e, Components&& ... comp)
	{
		return add(get_key(e), std::move(comp)...);
	}

	template <typename ... Components>
	any_manager& remove(any_entity_key_param aek)
	{
		assert(_pimpl);
		_pimpl->remove(aek, cnames<Components...>().data());
		return *this;
	}

	template <typename ... Components>
	any_manager& copy(any_entity_key_param aekf, any_entity_key_param aekt)
	{
		assert(_pimpl);
		_pimpl->add(aekf, aekt, cnames<Components...>().data());
		return *this;
	}

	template <typename Component>
	any_lock lifetime_lock(void)
	{
		assert(_pimpl);
		return _pimpl->lifetime_lock(
			component_name<Component>::c_str()
		);
	}

	template <typename Component>
	any_lock raw_access_lock(void)
	{
		assert(_pimpl);
		return _pimpl->raw_access_lock(
			component_name<Component>::c_str()
		);
	}

	template <typename Component>
	Component& raw_access(any_entity_key_param aek)
	{
		assert(_pimpl);
		void* pcomponent = _pimpl->raw_access(
			aek,
			component_name<Component>::c_str()
		);
		assert(pcomponent);
		return *((Component*)pcomponent);
	}

	template <typename Component>
	Component& rw(any_entity_key_param aek)
	{
		return raw_access<Component>(aek);
	}

	any_manager& for_each(
		const std::function<bool (
			const iter_info&,
			any_manager&,
			const any_entity_key&
		)>& func
	)
	{
		assert(_pimpl);
		_pimpl->for_each_imk(*this, func);
		return *this;
	}

	any_manager& for_each(
		const std::function<bool (
			any_manager&,
			const any_entity_key&
		)>& func
	)
	{
		assert(_pimpl);
		_pimpl->for_each_mk(*this, func);
		return *this;
	}

	template <typename Component>
	any_manager& for_each(const std::function<bool (Component&)>& func)
	{
		assert(_pimpl);
		_pimpl->for_each_c(
			static_cast<const void*>(&func), 
			component_name<Component>::c_str()
		);
		return *this;
	}
};


} // namespace exces

#endif //include guard

