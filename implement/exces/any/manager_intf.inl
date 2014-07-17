/**
 *  @file exces/any/manager_intf.inl
 *  @brief Implementation of manager type erasure
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <exces/any/manager_intf.hpp>
#include <unordered_map>

namespace exces {
//------------------------------------------------------------------------------
// any_manager_impl
//------------------------------------------------------------------------------
template <typename Group>
class any_manager_impl
 : public any_manager_intf<typename entity<Group>::type>
{
private:
	typedef typename any_entity_key::param_type aekp;
	typedef typename entity<Group>::type _entity_t;

	typedef manager<Group> _mgr_t;
	typedef typename _mgr_t::entity_key _ek_t;

	_mgr_t& _rmgr;

	const _ek_t& _get(aekp aek)
	{
		return aek.get<_ek_t>();
	}

	static std::unordered_map<std::size_t, void*>
	cids_by_names(const char** cnames, void** ptrs)
	{
		assert(cnames);
		std::unordered_map<std::size_t, void*> result;
		while(*cnames)
		{
			void* ptr = nullptr;
			if(ptrs)
			{
				ptr = *ptrs;
				++ptrs;
			}
			result[component_index<Group>::by_name(*cnames)] = ptr;
			++cnames;
		}
		return std::move(result);
	}

	template <typename Func>
	static Func& for_each_comp(Func& functor)
	{
		mp::for_each_ref<typename components<Group>::type>(functor);
		return functor;
	}
public:
	any_manager_impl(_mgr_t& mgr)
	 : _rmgr(mgr)
	{ }

	bool has_key(_entity_t e)
	{
		return _rmgr.has_key(e);
	}

	any_entity_key get_key(_entity_t e)
	{
		return any_entity_key(_rmgr.get_key(e));
	}

	_entity_t get_entity(aekp aek)
	{
		return _rmgr.get_entity(_get(aek));
	}

	template <typename Derived>
	struct _onec_hlp
	{
		_mgr_t& _rmgr;
		std::size_t _cid;

		Derived& derived(void)
		{
			return *(static_cast<Derived*>(this));
		}

		_onec_hlp(_mgr_t& mgr, const char* cname)
		 : _rmgr(mgr)
		 , _cid(component_index<Group>::by_name(cname))
		{ }

		template <typename C>
		void operator ()(mp::identity<C> ic)
		{
			if(_cid == component_id<C, Group>())
			{
				derived().operate(ic);
			}
		}
	};

	template <typename Derived>
	struct _allc_hlp
	{
		_mgr_t& _rmgr;
		std::unordered_map<std::size_t, void*> _cids;

		Derived& derived(void)
		{
			return *(static_cast<Derived*>(this));
		}

		_allc_hlp(_mgr_t& mgr, const char** cnames)
		 : _rmgr(mgr)
		 , _cids(cids_by_names(cnames, nullptr))
		{ }

		_allc_hlp(_mgr_t& mgr, const char** cnames, void** ptrs)
		 : _rmgr(mgr)
		 , _cids(cids_by_names(cnames, ptrs))
		{ }

		template <typename C>
		void operator ()(mp::identity<C> ic)
		{
			auto p = _cids.find(component_id<C, Group>());
			if(p != _cids.end())
			{
				derived().operate(ic, p->second);
			}
		}
	};

	template <typename E>
	struct _has_hlp : _onec_hlp<_has_hlp<E>>
	{
		E _e;
		bool _res;

		_has_hlp(_mgr_t& mgr, const char* cname, E e)
		 : _onec_hlp<_has_hlp<E>>(mgr, cname)
		 , _e(e)
		 , _res(false)
		{ }

		template <typename C>
		void operate(mp::identity<C>)
		{
			_res = this->_rmgr.template has<C>(_e);
		}
	};

	bool has(aekp aek, const char* cname)
	{
		_has_hlp<_ek_t> hlp(_rmgr, cname, _get(aek));
		return for_each_comp(hlp)._res;
	}

	bool has(_entity_t e, const char* cname)
	{
		_has_hlp<_entity_t> hlp(_rmgr, cname, e);
		return for_each_comp(hlp)._res;
	}

	struct _has_as_hlp : _allc_hlp<_has_as_hlp>
	{
		detail::component_bitset<Group> _bits;

		_has_as_hlp(_mgr_t& mgr, const char** cnames)
		 : _allc_hlp<_has_as_hlp>(mgr, cnames)
		{ }

		template <typename C>
		void operate(mp::identity<C>, void*)
		{
			_bits.set(component_id<C, Group>());
		}
	};

	bool has_all(aekp aek, const char** cnames)
	{
		_has_as_hlp hlp(_rmgr, cnames);
		for_each_comp(hlp);
		return _rmgr.has_all_bits(_get(aek), hlp._bits);
	}

	bool has_some(aekp aek, const char** cnames)
	{
		_has_as_hlp hlp(_rmgr, cnames);
		for_each_comp(hlp);
		return _rmgr.has_some_bits(_get(aek), hlp._bits);
	}

	struct _rsrv_hlp : _allc_hlp<_rsrv_hlp>
	{
		std::size_t _n;

		_rsrv_hlp(_mgr_t& mgr, const char** cnames, std::size_t n)
		 : _allc_hlp<_rsrv_hlp>(mgr, cnames)
		 , _n(n)
		{ }

		template <typename C>
		void operate(mp::identity<C>, void*)
		{
			this->_rmgr.template reserve<C>(_n);
		}
	};

	void reserve(std::size_t n, const char** cnames)
	{
		_rsrv_hlp hlp(_rmgr, cnames, n);
		for_each_comp(hlp);
	}

	struct _add_hlp : _allc_hlp<_add_hlp>
	{
		_ek_t _ek;

		_add_hlp(_mgr_t& mgr, const char** cnames, void** ptrs, _ek_t ek)
		 : _allc_hlp<_add_hlp>(mgr, cnames, ptrs)
		 , _ek(ek)
		{ }

		template <typename C>
		void operate(mp::identity<C>, void* ptr)
		{
			C& ref = *(static_cast<C*>(ptr));
			this->_rmgr.template add<C>(_ek, std::move(ref));
		}
	};

	void add(aekp aek, const char** cnames, void** refs)
	{
		_add_hlp hlp(_rmgr, cnames, refs, _get(aek));
		for_each_comp(hlp);
	}

	struct _rmv_hlp : _allc_hlp<_rmv_hlp>
	{
		_ek_t _ek;

		_rmv_hlp(_mgr_t& mgr, const char** cnames, _ek_t ek)
		 : _allc_hlp<_rmv_hlp>(mgr, cnames)
		 , _ek(ek)
		{ }

		template <typename C>
		void operate(mp::identity<C>, void*)
		{
			this->_rmgr.template remove<C>(_ek);
		}
	};

	void remove(aekp aek, const char** cnames)
	{
		_rmv_hlp hlp(_rmgr, cnames, _get(aek));
		for_each_comp(hlp);
	}

	struct _cpy_hlp : _allc_hlp<_cpy_hlp>
	{
		_ek_t _ekf;
		_ek_t _ekt;

		_cpy_hlp(_mgr_t& mgr, const char** cnames, _ek_t ekf, _ek_t ekt)
		 : _allc_hlp<_cpy_hlp>(mgr, cnames)
		 , _ekf(ekf)
		 , _ekt(ekt)
		{ }

		template <typename C>
		void operate(mp::identity<C>, void*)
		{
			this->_rmgr.template copy<C>(_ekf, _ekt);
		}
	};

	void copy(aekp aek_from, aekp aek_to, const char** cnames)
	{
		_cpy_hlp hlp(_rmgr, cnames, _get(aek_from), _get(aek_to));
		for_each_comp(hlp);
	}

	struct _ltlk_hlp : _onec_hlp<_ltlk_hlp>
	{
		any_lock _res;

		_ltlk_hlp(_mgr_t& mgr, const char* cname)
		 : _onec_hlp<_ltlk_hlp>(mgr, cname)
		{ }

		template <typename C>
		void operate(mp::identity<C>)
		{
			_res = any_lock(
				this->_rmgr.template lifetime_lock<C>()
			);
		}
	};

	any_lock lifetime_lock(const char* cname)
	{
		_ltlk_hlp hlp(_rmgr, cname);
		return std::move(for_each_comp(hlp)._res);
	}

	struct _ralk_hlp : _onec_hlp<_ralk_hlp>
	{
		any_lock _res;

		_ralk_hlp(_mgr_t& mgr, const char* cname)
		 : _onec_hlp<_ralk_hlp>(mgr, cname)
		{ }

		template <typename C>
		void operate(mp::identity<C>)
		{
			_res = any_lock(
				this->_rmgr.template raw_access_lock<C>()
			);
		}
	};

	any_lock raw_access_lock(const char* cname)
	{
		_ralk_hlp hlp(_rmgr, cname);
		return std::move(for_each_comp(hlp)._res);
	}

	typedef std::vector<std::size_t> entity_update_op;

	entity_update_op begin_update(aekp aek)
	{
		return _rmgr.begin_update(_get(aek));
	}

	void finish_update(aekp aek, const entity_update_op& update_op)
	{
		_rmgr.finish_update(_get(aek), update_op);
	}

	struct _ra_hlp : _onec_hlp<_ra_hlp>
	{
		_ek_t _ek;
		void* _res;

		_ra_hlp(_mgr_t& mgr, const char* cname, _ek_t ek)
		 : _onec_hlp<_ra_hlp>(mgr, cname)
		 , _ek(ek)
		 , _res(nullptr)
		{ }

		template <typename C>
		void operate(mp::identity<C>)
		{
			C& ref = this->_rmgr.template raw_access<C>(_ek);
			_res = &ref;
		}
	};

	void* raw_access(aekp aek, const char* cname)
	{
		_ra_hlp hlp(_rmgr, cname, _get(aek));
		return for_each_comp(hlp)._res;
	}

	void for_each_imk(
		any_manager<typename entity<Group>::type>& amgr,
		const std::function<bool (
			const iter_info&,
			any_manager<typename entity<Group>::type>&,
			const any_entity_key&
		)>& func
	)
	{
		std::function<bool(const iter_info&, _mgr_t&, _ek_t)> func_wrap=
			[&amgr, &func](
				const iter_info& ii,
				_mgr_t&,
				_ek_t ek
			) -> bool
			{
				return func(ii, amgr, any_entity_key(ek));
			};
		this->_rmgr.for_each(func_wrap);
	}

	void for_each_mk(
		any_manager<typename entity<Group>::type>& amgr,
		const std::function<bool (
			any_manager<typename entity<Group>::type>&,
			const any_entity_key&
		)>& func
	)
	{
		std::function<bool(const iter_info&, _mgr_t&, _ek_t)> func_wrap=
			[&amgr, &func](
				const iter_info&,
				_mgr_t&,
				_ek_t ek
			) -> bool
			{
				return func(amgr, any_entity_key(ek));
			};
		this->_rmgr.for_each(func_wrap);
	}

	struct _fec_hlp : _onec_hlp<_fec_hlp>
	{
		const void* _pstd_fn;

		_fec_hlp(_mgr_t& mgr, const char* cname, const void* pstd_fn)
		 : _onec_hlp<_fec_hlp>(mgr, cname)
		 , _pstd_fn(pstd_fn)
		{
			assert(_pstd_fn);
		}

		template <typename C>
		void operate(mp::identity<C>)
		{
			assert(_pstd_fn);
			typedef std::function<bool (C&)> Fn;
			const Fn& func = *(static_cast<const Fn*>(_pstd_fn));
			this->_rmgr.template for_each<C>(func);
		}
	};

	void for_each_c(const void* pstd_fn, const char* cname)
	{
		_fec_hlp hlp(_rmgr, cname, pstd_fn);
		for_each_comp(hlp);
	}
};
//------------------------------------------------------------------------------
// make_any_manager_impl
//------------------------------------------------------------------------------
template <typename Group>
std::shared_ptr<any_manager_intf<typename entity<Group>::type>>
make_any_manager_impl(manager<Group>& mgr)
{
	return std::make_shared<any_manager_impl<Group>>(mgr);
}
//------------------------------------------------------------------------------
} // namespace exces

