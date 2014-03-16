/**
 *  @file exces/any/manager_impl.hpp
 *  @brief Implementation of manager type erasure
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_ANY_ENTITY_MANAGER_IMPL_1403152314_HPP
#define EXCES_ANY_ENTITY_MANAGER_IMPL_1403152314_HPP

#include <exces/any/manager_intf.hpp>
#include <unordered_set>

namespace exces {

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

	static std::unordered_set<std::size_t>
	cids_by_names(const char** cnames)
	{
		assert(cnames);
		std::unordered_set<std::size_t> result;
		while(*cnames)
		{
			result.insert(component_index<Group>::by_name(*cnames));
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
	struct _iter_hlp
	{
		_mgr_t& _rmgr;
		std::unordered_set<std::size_t> _cids;

		Derived& derived(void)
		{
			return *(static_cast<Derived*>(this));
		}

		_iter_hlp(_mgr_t& mgr, const char** cnames)
		 : _rmgr(mgr)
		 , _cids(cids_by_names(cnames))
		{ }

		template <typename C>
		void operator ()(mp::identity<C> ic)
		{
			if(_cids.count(component_id<C, Group>()))
			{
				derived().operate(ic);
			}
		}
	};

	template <typename E>
	struct _has_hlp
	{
		_mgr_t& _rmgr;
		E _e;
		std::size_t _cid;
		bool _res;

		template <typename C>
		void operator ()(mp::identity<C>)
		{
			if(_cid == component_id<C, Group>())
			{
				_res = _rmgr.template has<C>(_e);
			}
		}
	};

	bool has(aekp aek, const char* cname)
	{
		std::size_t cid = component_index<Group>::by_name(cname);
		_has_hlp<_ek_t> hlp = { _rmgr, _get(aek), cid, false };
		return for_each_comp(hlp)._res;
	}

	bool has(_entity_t e, const char* cname)
	{
		std::size_t cid = component_index<Group>::by_name(cname);
		_has_hlp<_entity_t> hlp = { _rmgr, e, cid, false };
		return for_each_comp(hlp)._res;
	}

	struct _has_as_hlp : _iter_hlp<_has_as_hlp>
	{
		aux_::component_bitset<Group> _bits;

		_has_as_hlp(_mgr_t& mgr, const char** cnames)
		 : _iter_hlp<_has_as_hlp>(mgr, cnames)
		{ }

		template <typename C>
		void operate(mp::identity<C>)
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

	struct _rsrv_hlp : _iter_hlp<_rsrv_hlp>
	{
		std::size_t _n;

		_rsrv_hlp(_mgr_t& mgr, const char** cnames, std::size_t n)
		 : _iter_hlp<_rsrv_hlp>(mgr, cnames)
		 , _n(n)
		{ }

		template <typename C>
		void operate(mp::identity<C>)
		{
			this->_rmgr.template reserve<C>(_n);
		}
	};

	void reserve(std::size_t n, const char** cnames)
	{
		_rsrv_hlp hlp(_rmgr, cnames, n);
		for_each_comp(hlp);
	}

	struct _ra_hlp
	{
		_mgr_t& _rmgr;
		_ek_t _ek;
		std::size_t _cid;
		void* _res;

		template <typename C>
		void operator ()(mp::identity<C>)
		{
			if(_cid == component_id<C, Group>())
			{
				C& ref = _rmgr.template raw_access<C>(_ek);
				_res = &ref;
			}
		}
	};

	void* raw_access(aekp aek, const char* cname)
	{
		std::size_t cid = component_index<Group>::by_name(cname);
		_ra_hlp hlp = { _rmgr, _get(aek), cid, nullptr };
		return for_each_comp(hlp)._res;
	}
};

template <typename Group>
std::shared_ptr<any_manager_intf<typename entity<Group>::type>>
make_any_manager_impl(manager<Group>& mgr)
{
	return std::make_shared<any_manager_impl<Group>>(mgr);
}

} // namespace exces

#endif //include guard

