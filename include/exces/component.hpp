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

namespace exces {

template <typename Component, typename Group = default_group>
class shared_component
{
private:
	typedef component_storage<Group> _storage;
	_storage* _pstorage;

	typedef typename _storage::component_key component_key;
	component_key _key;

	void _add_ref(void)
	{
		if(is_valid())
		{
			_pstorage->template add_ref<Component>(_key);
		}
	}

	void _release(void)
	{
		if(is_valid())
		{
			_pstorage->template release<Component>(_key);
		}
	}
public:
	shared_component(
		_storage& storage,
		component_key key
	): _pstorage(&storage)
	 , _key(key)
	{
		_add_ref();
	}

	shared_component(const shared_component& that)
	 : _pstorage(that._pstorage)
	 , _key(that._key)
	{
		_add_ref();
	}

	shared_component(shared_component&& tmp)
	 : _pstorage(tmp._pstorage)
	 , _key(tmp._key)
	{
		tmp._pstorage = nullptr;
		tmp._key = _storage::null_key();
	}

	~shared_component(void)
	{
		_release();
	}

	bool is_valid(void) const
	{
		return _pstorage && (_key != _storage::null_key());
	}

	operator bool (void) const
	{
		return is_valid();
	}

	bool operator ! (void) const
	{
		return !is_valid();
	}

	Component& ref(void)
	{
		assert(is_valid());
		return _pstorage->template access<Component>(_key);
	}

	Component* operator -> (void)
	{
		return &ref();
	}
};

} // namespace exces

#endif //include guard

