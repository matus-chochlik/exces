/**
 *  @file exces/func_adaptors/mkcp.hpp
 *  @brief Adaptors for various functors making them usable with for_each
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_FUNC_ADAPTORS_MKCP_1404292124_HPP
#define EXCES_FUNC_ADAPTORS_MKCP_1404292124_HPP

#include <exces/fwd.hpp>
#include <functional>

namespace exces {

// Manager, Key, Pointers-to-components ...

/// Similar to func_adaptor_c but also accepting a reference to manager and a key
/** This adaptor is similar to func_adaptor_cp, but the functions adapted by this
 *  adaptor accept in addition to pointers to the specified Components also
 *  a reference to the manager and an entity key as the first and second argument
 *  respectivelly.
 *
 *  @ingroup func_adaptors
 *
 *  @see adapt_func_mkc
 *  @see func_adaptor_c
 */
template <typename Functor, typename ... Components>
struct func_adaptor_mkcp
{
	Functor _functor;

	/// Adapts the specified functor
	func_adaptor_mkcp(const Functor& functor)
	 : _functor(functor)
	{ }

	template <typename Group, typename Component>
	Component* _get_ptr(
		manager<Group>& m,
		typename manager<Group>::entity_key k
	) const
	{
		Component* ptr = nullptr;
		if(m.template has<Component>(k))
		{
			ptr = &m.template raw_access<Component>(k);
		}
		return ptr;
	}

	/// The function call operator
	template <typename Group>
	bool operator()(
		const iter_info&,
		manager<Group>& m,
		typename manager<Group>::entity_key k
	)
	{
		if(m.template has_some<Components...>(k))
		{
			if(!_functor(
				m, k,
				_get_ptr<Group, Components>(m, k)...
			)) return false;
		}
		return true;
	}
};

/// Constructs a new instance of func_adaptor_mkcp
/**
 *  @ingroup func_adaptors
 */
template <typename ... Components, typename Functor>
inline
func_adaptor_mkcp<Functor, Components...>
adapt_func_mkcp(const Functor& functor)
{
	return func_adaptor_mkcp<Functor, Components...>(functor);
}

/// Constructs a new instance of func_adaptor_mkcp adapting a std::function
/**
 *  @ingroup func_adaptors
 */
template <typename Group, typename ... Components>
inline 
func_adaptor_mkcp<
	std::function<bool(
		manager<Group>&,
		typename manager<Group>::entity_key,
		Components*...
	)>,
	Components...
> adapt_func(
	const std::function<bool(
		manager<Group>&,
		typename manager<Group>::entity_key,
		Components*...
	)>& functor
)
{
	return adapt_func_mkcp<Components...>(functor);
}

} // namespace exces

#endif //include guard

