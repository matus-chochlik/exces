/**
 *  @file exces/func_adaptors/mkec.hpp
 *  @brief Adaptors for various functors making them usable with for_each
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_FUNC_ADAPTORS_MKEC_1404292124_HPP
#define EXCES_FUNC_ADAPTORS_MKEC_1404292124_HPP

#include <exces/fwd.hpp>
#include <functional>

namespace exces {

// Manager, Key, Entity, Components ...

/// Similar to func_adaptor_c but also accepting a manager, a key and entity
/** This adaptor is similar to func_adaptor_c, but the functions adapted by this
 *  adaptor accept in addition to the specified Components also a reference
 *  to the manager, an entity key and an entity as the first, second and third
 *  argument respectivelly.
 *
 *  @ingroup func_adaptors
 *
 *  @see adapt_func_mkec
 *  @see func_adaptor_c
 */
template <typename Functor, typename ... Components>
struct func_adaptor_mkec
{
	Functor _functor;

	/// Adapts the specified functor
	func_adaptor_mkec(const Functor& functor)
	 : _functor(functor)
	{ }

	/// The function call operator
	template <typename Group>
	bool operator()(
		const iter_info&,
		manager<Group>& m,
		typename manager<Group>::entity_key k
	)
	{
		if(m.template has_all<Components...>(k))
		{
			if(!_functor(
				m, k, m.get_entity(k),
				m.template raw_access<Components>(k)...
			)) return false;
		}
		return true;
	}
};

/// Constructs a new instance of func_adaptor_mkec
/**
 *  @ingroup func_adaptors
 */
template <typename ... Components, typename Functor>
inline
func_adaptor_mkec<Functor, Components...>
adapt_func_mkec(const Functor& functor)
{
	return func_adaptor_mkec<Functor, Components...>(functor);
}

/// Constructs a new instance of func_adaptor_mkec adapting a std::function
/**
 *  @ingroup func_adaptors
 */
template <typename Group, typename ... Components>
inline 
func_adaptor_mkec<
	std::function<bool(
		manager<Group>&,
		typename manager<Group>::entity_key,
		typename entity<Group>::type,
		Components...
	)>,
	Components...
> adapt_func(
	const std::function<bool(
		manager<Group>&,
		typename manager<Group>::entity_key,
		typename entity<Group>::type,
		Components...
	)>& functor
)
{
	return adapt_func_mkec<Components...>(functor);
}

} // namespace exces

#endif //include guard

