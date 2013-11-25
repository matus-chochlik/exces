/**
 *  @file exces/component.hpp
 *  @brief Implements component-related utilities
 *
 *  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_COMPONENT_1212101457_HPP
#define EXCES_COMPONENT_1212101457_HPP

#include <exces/group.hpp>
#include <exces/storage.hpp>

#include <cassert>
#include <type_traits>

namespace exces {

template <typename Group>
class manager;

namespace aux {

template <typename Component, typename Group>
void manager_replace_component_at(
	manager<Group>& mngr,
	typename manager<Group>::entity_key ek,
	typename component_storage<Group>::component_key ck,
	Component&& component
);

template <typename Component, typename Group, bool IsFlyweight>
class shared_component_base;

template <typename Component, typename Group>
class shared_component_base<Component, Group, false>
{
protected:
	typedef manager<Group> _manager;

	typedef typename _manager::entity_key entity_key;

	typedef component_storage<Group> _storage;

	typedef typename _storage::component_key component_key;

	typedef Component& component_ref;

	static component_ref _make_component_ref(
		_manager*,
		entity_key,
		_storage* pstorage,
		component_key key
	)
	{
		pstorage->template mark_write<Component>(key);
		return pstorage->template access<Component>(key);
	}
public:
};

template <typename Component, typename Group>
class shared_component_base<Component, Group, true>
{
protected:
	typedef manager<Group> _manager;

	typedef typename _manager::entity_key entity_key;

	typedef component_storage<Group> _storage;

	typedef typename _storage::component_key component_key;

	class component_ref : public Component
	{
	private:
		_manager* _exces_aux_pmanager;
		entity_key _exces_aux_ekey;
		_storage* _exces_aux_pstorage;
		component_key _exces_aux_ckey;
		component_equal_to<Component, Group> _exces_aux_equal;

		Component& temp(void) { return *this; }
		Component& orig(void)
		{
			assert(_exces_aux_pstorage);
			return _exces_aux_pstorage->template access<Component>(
				_exces_aux_ckey
			);
		}
	public:
		component_ref(
			_manager* pmanager,
			entity_key ekey,
			_storage* pstorage,
			component_key ckey
		): Component(pstorage->template access<Component>(ckey))
		 , _exces_aux_pmanager(pmanager)
		 , _exces_aux_ekey(ekey)
		 , _exces_aux_pstorage(pstorage)
		 , _exces_aux_ckey(ckey)
		{ }

		component_ref(component_ref&& tmp)
		 : Component(static_cast<Component&&>(tmp))
		 , _exces_aux_pmanager(tmp._exces_aux_pmanager)
		 , _exces_aux_ekey(tmp._exces_aux_ekey)
		 , _exces_aux_pstorage(tmp._exces_aux_pstorage)
		 , _exces_aux_ckey(tmp._exces_aux_ckey)
		{
			tmp._exces_aux_pmanager = nullptr;
			tmp._exces_aux_pstorage = nullptr;
		}

		~component_ref(void)
		{
			if(_exces_aux_pmanager && _exces_aux_pstorage)
			{
				if(!_exces_aux_equal(temp(), orig()))
				{
					aux::manager_replace_component_at(
						*_exces_aux_pmanager,
						_exces_aux_ekey,
						_exces_aux_ckey,
						std::move(temp())
					);
				}
			}
		}

		operator Component (void) const
		{
			return temp();
		}

		component_ref& operator = (Component that)
		{
			temp() = that;
			return *this;
		}
	};
public:
	static component_ref _make_component_ref(
		_manager* pmanager,
		entity_key ekey,
		_storage* pstorage,
		component_key ckey
	)
	{
		return component_ref(
			pmanager,
			ekey,
			pstorage,
			ckey
		);
	}
};

} // namespace aux

/// Smart wraper for an instance of a component
/** Instantiations of this template reference a single instance of a component
 *  of a specified entity. It allows the value to be read or chaned conveniently
 *  and ensures that the instance of the component remains valid even if it
 *  is removed from the entity.
 */
template <typename Component, typename Group = default_group>
class shared_component
 : public aux::shared_component_base<
	Component,
	Group,
	flyweight_component<Component>::value
>
{
private:
	typedef aux::shared_component_base<
		Component,
		Group,
		flyweight_component<Component>::value
	> _base;

	typedef manager<Group> _manager;
	_manager* _pmanager;

	typedef typename _manager::entity_key _entity_key;
	_entity_key _ekey;

	typedef component_storage<Group> _storage;
	_storage* _pstorage;

	typedef typename _storage::component_key component_key;
	component_key _ckey;

	void _add_ref(void)
	{
		if(is_valid())
		{
			_pstorage->template add_ref<Component>(_ckey);
		}
	}

	void _release(void)
	{
		if(is_valid())
		{
			_pstorage->template release<Component>(_ckey);
		}
	}
public:
	shared_component(
		_manager& mngr,
		_entity_key ekey,
		_storage& storage,
		component_key ckey
	): _pmanager(&mngr)
	 , _ekey(ekey)
	 , _pstorage(&storage)
	 , _ckey(ckey)
	{
		_add_ref();
	}

	/// Shared components are copy-constructible
	shared_component(const shared_component& that)
	 : _pmanager(that._pmanager)
	 , _ekey(that._ekey)
	 , _pstorage(that._pstorage)
	 , _ckey(that._ckey)
	{
		_add_ref();
	}

	/// Shared components are movable
	shared_component(shared_component&& tmp)
	 : _pmanager(tmp._pmanager)
	 , _ekey(tmp._ekey)
	 , _pstorage(tmp._pstorage)
	 , _ckey(tmp._ckey)
	{
		tmp._pmanager = nullptr;
		tmp._pstorage = nullptr;
		tmp._ckey = _storage::null_key();
	}

	/// Releases a reference to the managed component instance
	~shared_component(void)
	{
		_release();
	}

	/// Returns true if the component has not been moved from
	bool is_valid(void) const
	{
		return _pmanager && _pstorage && (_ckey != _storage::null_key());
	}

	/// Returns true if the component has not been moved from
	operator bool (void) const
	{
		return is_valid();
	}

	/// Returns true if the component has been moved from
	bool operator ! (void) const
	{
		return !is_valid();
	}

	/// Returns a const reference to the managed component
	const Component& read(void) const
	{
		assert(is_valid());
		return _pstorage->template access<Component>(_ckey);
	}

#if OALPLUS_DOCUMENTATION_ONLY
	/// The type of the reference that allows to mutate the component
	typedef Unspecified component_ref;
#else
	typedef typename _base::component_ref component_ref;
#endif

	/// Returns a reference that allows to change the managed component
	component_ref write(void)
	{
		assert(is_valid());
		return this->_make_component_ref(
			_pmanager,
			_ekey,
			_pstorage,
			_ckey
		);
	}

	/// Replaces the managed component with a new value
	void replace(Component&& component)
	{
		assert(is_valid());
		aux::manager_replace_component_at(
			*_pmanager,
			_ekey,
			_ckey,
			std::move(component)
		);
	}

};

} // namespace exces

#endif //include guard

