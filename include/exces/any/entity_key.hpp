/**
 *  @file exces/any/entity_key.hpp
 *  @brief Type erasure manager<Group>::entity_key types
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_ANY_ENTITY_KEY_1403152314_HPP
#define EXCES_ANY_ENTITY_KEY_1403152314_HPP

#include <exces/manager.hpp>

namespace exces {

/// Type erasure for manager<Group>::entity_key
class any_entity_key
{
private:
	typedef std::aligned_storage<
		sizeof(std::map<int, int>::iterator),
		std::alignment_of<std::map<int, int>::iterator>::value
	>::type _store_t;
	_store_t _store;

	// TODO optimizations if EK has trivial copy and/or destructor

	void (*_copy)(void*, const void*);

	template <typename EK>
	static void _do_copy(void* dst, const void* src)
	{
		static_assert(
			sizeof(_store_t) >= sizeof(EK),
			"Insufficient storage space for entity key"
		);
		new(dst) EK(*((const EK*)src));
	}

	void (*_destroy)(void*);

	template <typename EK>
	static void _do_destroy(void* ptr)
	{
		((EK*)ptr)->~EK();
	}
public:
	typedef const any_entity_key& param_type;

	any_entity_key(void)
	 : _copy(nullptr)
	 , _destroy(nullptr)
	{ }

	template <typename EK>
	any_entity_key(EK ek)
	 : _copy(&_do_copy<EK>)
	 , _destroy(&_do_destroy<EK>)
	{
		_copy(&_store, &ek);
	}

	any_entity_key(const any_entity_key& that)
	 : _copy(that._copy)
	 , _destroy(that._destroy)
	{
		if(_copy)
		{
			_copy(&_store, &that._store);
		}
	}

	~any_entity_key(void)
	{
		if(_destroy)
		{
			_destroy(&_store);
		}
	}

	template <typename EK>
	const EK& get(void) const
	{
		return *((EK*)&_store);
	}
};

} // namespace exces

#endif //include guard

