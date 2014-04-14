/**
 *  @file exces/component.hpp
 *  @brief Implements component-related utilities
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_COMPONENT_1212101457_HPP
#define EXCES_COMPONENT_1212101457_HPP

#include <exces/group.hpp>
#include <exces/storage.hpp>
#include <exces/aux_/component.hpp>

#include <cassert>
#include <type_traits>

namespace exces {
namespace aux_ {

// sh_comp_base
template <
	typename Group,
	typename Component,
	typename Kind,
	typename Access
> class sh_comp_base;

// read-only sh_comp_base
template <typename Group, typename Component, typename Kind>
class sh_comp_base<
	Group,
	Component,
	Kind,
	component_access_read_only
>
{
public:
	template <typename M, typename S, typename EK, typename CK>
	struct component_ptr
	{
		typedef const Component* type;
	};
protected:
	template <typename M, typename S, typename EK, typename CK>
	static const Component* _make_ptr(const M&, S& storage, EK, CK key)
	{
		return &storage.template access<Component>(key);
	}
};

// read-write normal sh_comp_base
template <typename Group, typename Component>
class sh_comp_base<
	Group,
	Component,
	component_kind_normal,
	component_access_read_write
>
{
public:
	template <typename M, typename S, typename EK, typename CK>
	struct component_ptr
	{
		typedef Component* type;
	};
protected:
	template <typename M, typename S, typename EK, typename CK>
	static Component* _make_ptr(const M&, S& storage, EK, CK key)
	{
		storage.template mark_write<Component>(key);
		return &storage.template access<Component>(key);
	}
};

// read-write flyweight sh_comp_base
template <typename Group, typename Component>
class sh_comp_base<
	Group,
	Component,
	component_kind_flyweight,
	component_access_read_write
>
{
public:
	template <typename M, typename S, typename EK, typename CK>
	class component_ptr : public Component
	{
	private:
		M* _exces_aux_pmanager;
		S* _exces_aux_pstorage;
		EK _exces_aux_ekey;
		CK _exces_aux_ckey;

		Component& temp(void) { return *this; }
		Component& orig(void)
		{
			assert(_exces_aux_pstorage);
			return _exces_aux_pstorage->template access<Component>(
				_exces_aux_ckey
			);
		}
	public:
		typedef component_ptr type;

		component_ptr(
			M& rmanager,
			S& rstorage,
			EK ekey,
			CK ckey
		): Component(rstorage.template access<Component>(ckey))
		 , _exces_aux_pmanager(&rmanager)
		 , _exces_aux_pstorage(&rstorage)
		 , _exces_aux_ekey(ekey)
		 , _exces_aux_ckey(ckey)
		{ }

		component_ptr(component_ptr&& tmp)
		 : Component(static_cast<Component&&>(tmp))
		 , _exces_aux_pmanager(tmp._exces_aux_pmanager)
		 , _exces_aux_ekey(tmp._exces_aux_ekey)
		 , _exces_aux_pstorage(tmp._exces_aux_pstorage)
		 , _exces_aux_ckey(tmp._exces_aux_ckey)
		{
			tmp._exces_aux_pmanager = nullptr;
			tmp._exces_aux_pstorage = nullptr;
		}

		~component_ptr(void)
		{
			if(_exces_aux_pmanager && _exces_aux_pstorage)
			{
				if(!(temp() == orig()))
				{
					_exces_aux_pmanager->replace_component_at(
						_exces_aux_ekey,
						_exces_aux_ckey,
						std::move(temp())
					);
				}
			}
		}

		operator Component * (void)
		{
			return this;
		}
	};
protected:
	template <typename M, typename S, typename EK, typename CK>
	static component_ptr<M, S, EK, CK>
	_make_ptr(M& manager, S& storage, EK ekey, CK ckey)
	{
		storage.template mark_write<Component>(ckey);
		return component_ptr<M, S, EK, CK>(
			manager,
			storage,
			ekey,
			ckey
		);
	}
};

template <typename Group, typename Access>
struct sh_comp_op_ctx
{
	sh_comp_op_ctx(
		manager<Group>&, 
		typename manager<Group>::entity_key
	){ }
};

template <typename Group>
struct sh_comp_op_ctx<Group, component_access_read_write>
{
	manager<Group>* _pmgr;
	typename manager<Group>::entity_key _ekey;
	typename manager<Group>::entity_update_op _upop;

	sh_comp_op_ctx(
		manager<Group>& mgr, 
		typename manager<Group>::entity_key ekey
	): _pmgr(&mgr)
	 , _ekey(ekey)
	 , _upop(_pmgr->begin_update(_ekey))
	{ }

	sh_comp_op_ctx(sh_comp_op_ctx&& tmp)
	 : _pmgr(tmp._pmgr)
	 , _ekey(tmp._ekey)
	 , _upop(std::move(tmp._upop))
	{
		tmp._pmgr = nullptr;
	}

	~sh_comp_op_ctx(void)
	{
		if(_pmgr != nullptr)
		{
			_pmgr->finish_update(_ekey, _upop);
		}
	}
};

template <
	typename Group,
	typename Component,
	typename Kind,
	typename Access
>
class sh_comp_acc_op
{
private:
	typedef typename sh_comp_base<
		Group,
		Component,
		Kind,
		Access
	>::template component_ptr<
		manager<Group>,
		component_storage<Group>,
		typename manager<Group>::entity_key,
		typename component_storage<Group>::component_key
	>::type _component_ptr;

	_component_ptr _ptr;

	sh_comp_op_ctx<Group, Access> _op_ctx;

	typedef typename component_storage<Group>
	::template component_access_lock<
		Component,
		Access
	>::type _lockable;

	_lockable _lock;

	typedef component_locking<Group, Component> _locking;
	typename _locking::template lock_guard<_lockable> _guard;

	static const Component* _get_ptr_type(component_access_read_only);
	static Component* _get_ptr_type(component_access_read_write);
	typedef decltype(_get_ptr_type(Access())) _ptr_type;

	static const Component& _get_ref_type(component_access_read_only);
	static Component& _get_ref_type(component_access_read_write);
	typedef decltype(_get_ref_type(Access())) _ref_type;

	_ptr_type _get_ptr(void)
	{
		_ptr_type result = _ptr;
		assert(result);
		return result;
	}
public:
	sh_comp_acc_op(
		manager<Group>& mgr,
		typename manager<Group>::entity_key ekey,
		_component_ptr&& ptr,
		_lockable&& lock
	): _ptr(std::move(ptr))
	 , _op_ctx(mgr, ekey)
	 , _lock(std::move(lock))
	 , _guard(_lock)
	{ }

	sh_comp_acc_op(sh_comp_acc_op&& tmp)
	 : _ptr(std::move(tmp._ptr))
	 , _op_ctx(std::move(tmp._op_ctx))
	 , _lock(std::move(tmp._lock))
	 , _guard(std::move(tmp._guard))
	{ }

	sh_comp_acc_op(const sh_comp_acc_op&) = delete;

	_ref_type self(void)
	{
		return *(_get_ptr());
	}

	operator _ref_type (void)
	{
		return *(_get_ptr());
	}

	_ptr_type operator -> (void)
	{
		return _get_ptr();
	}
};

} // namespace aux

/// Smart wraper for an instance of a component
/** Instantiations of this template reference a single instance of a component
 *  of a specified entity. It allows the value to be read or chaned conveniently
 *  and ensures that the instance of the component remains valid even if it
 *  is removed from the entity.
 */
template <
	typename Group,
	typename Component,
	typename Access
>
class shared_component
 : public aux_::sh_comp_base<
	Group,
	Component,
	typename component_kind<Component, Group>::type,
	Access
>
{
private:
	typedef aux_::sh_comp_base<
		Group,
		Component,
		typename component_kind<Component, Group>::type,
		Access
	> _base;

	typedef manager<Group> _manager;
	_manager* _pmanager;

	typedef component_storage<Group> _storage;

	_storage& _cstorage(void)
	{
		assert(_pmanager);
		return _pmanager->_get_storage_ref();
	}

	typedef typename _manager::entity_key _entity_key;
	_entity_key _ekey;

	typedef typename component_storage<Group>::component_key
		_component_key;
	_component_key _ckey;

	void _add_ref(void)
	{
		if(is_valid())
		{
			_cstorage().template add_ref<Component>(_ckey);
		}
	}

	void _release(void)
	{
		if(is_valid())
		{
			_cstorage().template release<Component>(_ckey);
		}
	}

	typedef component_locking<Group, Component> _locking;

	static typename _locking::shared_lock
	_get_access_lock(component_access_read_only);

	static typename _locking::unique_lock
	_get_access_lock(component_access_read_write);
public:
	shared_component(
		_manager& mngr,
		_entity_key ekey,
		_component_key ckey
	): _pmanager(&mngr)
	 , _ekey(ekey)
	 , _ckey(ckey)
	{
		_add_ref();
	}

	/// Shared components are copy-constructible
	shared_component(const shared_component& that)
	 : _pmanager(that._pmanager)
	 , _ekey(that._ekey)
	 , _ckey(that._ckey)
	{
		_add_ref();
	}

	/// Shared components are movable
	shared_component(shared_component&& tmp)
	 : _pmanager(tmp._pmanager)
	 , _ekey(tmp._ekey)
	 , _ckey(tmp._ckey)
	{
		tmp._pmanager = nullptr;
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
		return _pmanager && (_ckey != _storage::null_key());
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

	typedef aux_::sh_comp_acc_op<
		Group,
		Component,
		typename component_kind<Component>::type,
		Access
	> access_op;

	/// Returns a reference to the managed component with specified Access
	access_op get(void)
	{
		assert(is_valid());
		return access_op(
			*_pmanager, _ekey,
			this->_make_ptr(
				*_pmanager,
				_cstorage(),
				_ekey,
				_ckey
			),
			_cstorage().template access_lock<Component>(Access())
		);
	}

	access_op operator -> (void)
	{
		return get();
	}

	/// Replaces the managed component with a new value
	void replace(Component&& component)
	{
		assert(is_valid());
		_pmanager->replace_component_at(
			_ekey,
			_ckey,
			std::move(component)
		);
	}
};

template <typename Component>
inline typename mp::if_c<
	std::is_reference<Component>::value &&
	not(std::is_const<Component>::value),
	component_access_read_write,
	component_access_read_only
>::type get_component_access(void)
{
	return typename mp::if_c<
		std::is_reference<Component>::value &&
		not(std::is_const<Component>::value),
		component_access_read_write,
		component_access_read_only
	>::type();
}

} // namespace exces

#endif //include guard

