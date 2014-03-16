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
#include <exces/aux_/metaprog.hpp>
#include <cstring>

namespace exces {
namespace aux_ {

template <bool Trivial>
struct any_entity_key_cctr;
	
template <>
struct any_entity_key_cctr<true>
{
	any_entity_key_cctr(void) = default;

	template <typename EK>
	any_entity_key_cctr(const EK&)
	{ }

	operator bool (void) const
	{
		return true;
	}
	
	void operator()(void* dst, const void* src, std::size_t size)
	{
		std::memcpy(dst, src, size);
	}
};

template <>
struct any_entity_key_cctr<false>
{
	void (*_copy)(void*, const void*, std::size_t);

	template <typename EK>
	static void _do_copy(void* dst, const void* src, std::size_t size)
	{
		assert(size >= sizeof(EK));
		new(dst) EK(*((const EK*)src));
	}

	any_entity_key_cctr(void)
	 : _copy(nullptr)
	{ }

	template <typename EK>
	any_entity_key_cctr(const EK&)
	 : _copy(&_do_copy<EK>)
	{ }

	any_entity_key_cctr(const any_entity_key_cctr& that)
	 : _copy(that._copy)
	{ }

	operator bool (void) const
	{
		return _copy != nullptr;
	}
	
	void operator()(void* dst, const void* src, std::size_t size)
	{
		assert(_copy);
		_copy(dst, src, size);
	}
};

template <bool Trivial>
struct any_entity_key_dtr;

template <>
struct any_entity_key_dtr<true>
{
	any_entity_key_dtr(void) = default;

	template <typename EK>
	any_entity_key_dtr(const EK&)
	{ }

	operator bool (void) const
	{
		return true;
	}
	
	void operator()(void*)
	{ }
};

template <>
struct any_entity_key_dtr<false>
{
	void (*_destroy)(void*);

	template <typename EK>
	static void _do_destroy(void* ptr)
	{
		((EK*)ptr)->~EK();
	}

	any_entity_key_dtr(void)
	 : _destroy(nullptr)
	{ }

	template <typename EK>
	any_entity_key_dtr(const EK&)
	 : _destroy(&_do_destroy<EK>)
	{ }

	any_entity_key_dtr(const any_entity_key_dtr& that)
	 : _destroy(that._destroy)
	{ }

	operator bool (void) const
	{
		return _destroy != nullptr;
	}
	
	void operator()(void* ptr)
	{
		assert(_destroy);
		_destroy(ptr);
	}
};

} // namespace aux_

/// Type erasure for manager<Group>::entity_key
/** Exces users should treat any_entity_key as an opaque type supporting
 *  only default and copy construction, destruction and usage with 
 */
class any_entity_key
{
private:
	typedef std::map<int, int>::iterator _ek;

	typedef std::aligned_storage<
		sizeof(_ek),
		std::alignment_of<_ek>::value
	>::type _store_t;
	_store_t _store;

	aux_::any_entity_key_cctr<
		mp::is_trivially_copyable<_ek>::value
	> _copy;

	aux_::any_entity_key_dtr<
		mp::is_trivially_destructible<_ek>::value
	> _destroy;

public:
	typedef const any_entity_key& param_type;

	any_entity_key(void)
	 : _copy()
	 , _destroy()
	{ }

	template <typename EK>
	any_entity_key(EK ek)
	 : _copy(ek)
	 , _destroy(ek)
	{
		_copy(&_store, &ek, sizeof(_store));
	}

	any_entity_key(const any_entity_key& that)
	 : _copy(that._copy)
	 , _destroy(that._destroy)
	{
		if(_copy)
		{
			_copy(&_store, &that._store, sizeof(_store));
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

