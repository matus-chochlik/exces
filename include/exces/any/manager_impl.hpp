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

namespace exces {

template <typename Group>
class any_manager_impl
 : public any_manager_intf<typename entity<Group>::type>
{
private:
	typedef typename any_entity_key::param_type any_entity_key_param;
	typedef typename entity<Group>::type _entity_t;

	typedef manager<Group> _mgr_t;
	typedef typename _mgr_t::entity_key _ek_t;

	_mgr_t& _rmgr;

	const _ek_t& _get(any_entity_key_param aek)
	{
		return aek.get<_ek_t>();
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

	_entity_t get_entity(any_entity_key_param aek)
	{
		return _rmgr.get_entity(_get(aek));
	}

	template <typename E>
	struct _has_hlp
	{
		_mgr_t& _rmgr;
		E _e;
		std::size_t _cid;
		bool& _res;

		template <typename C>
		void operator ()(mp::identity<C>)
		{
			if(_cid == component_id<C, Group>())
			{
				_res = _rmgr.template has<C>(_e);
			}
		}
	};

	bool has(any_entity_key_param aek, const char* cname)
	{
		std::size_t cid = component_index<Group>::by_name(cname);
		bool result = false;
		_has_hlp<_ek_t> hlp = { _rmgr, _get(aek), cid, result };
		mp::for_each<typename components<Group>::type>(hlp);
		return result;
	}

	bool has(_entity_t e, const char* cname)
	{
		std::size_t cid = component_index<Group>::by_name(cname);
		bool result = false;
		_has_hlp<_entity_t> hlp = { _rmgr, e, cid, result };
		mp::for_each<typename components<Group>::type>(hlp);
		return result;
	}

	struct _ra_hlp
	{
		_mgr_t& _rmgr;
		_ek_t _ek;
		std::size_t _cid;
		void*& _res;

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

	void* raw_access(any_entity_key_param aek, const char* cname)
	{
		void* result = nullptr;
		std::size_t cid = component_index<Group>::by_name(cname);
		_ra_hlp hlp = { _rmgr, _get(aek), cid, result };
		mp::for_each<typename components<Group>::type>(hlp);
		return result;
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

