/**
 *  @file exces/func_adaptors/imkc.hpp
 *  @brief Adaptors for various functors making them usable with for_each
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_FUNC_ADAPTORS_IMKC_1404292124_HPP
#define EXCES_FUNC_ADAPTORS_IMKC_1404292124_HPP

#include <exces/fwd.hpp>
#include <functional>

namespace exces {

// IterInfo, Manager, Key, Components ...

/// Similar to func_adaptor_mkc but also accepting a const reference to iter_info
/** This adaptor is similar to func_adaptor_mkc, but the functions adapted by this
 *  adaptor accept in addition to the specified Components also a const reference
 *  to the iter_info structure providing info about the traversal
 *
 *  @ingroup func_adaptors
 *
 *  @see adapt_func_imkc
 *  @see func_adaptor_mkc
 */
template <typename Functor, typename ... Components>
struct func_adaptor_imkc
{
	Functor _functor;

	/// Adapts the specified functor
	func_adaptor_imkc(const Functor& functor)
	 : _functor(functor)
	{ }

	/// The function call operator
	template <typename Group>
	bool operator()(
		const iter_info& ii,
		manager<Group>& m,
		typename manager<Group>::entity_key k
	)
	{
		if(m.template has_all<Components...>(k))
		{
			_functor(
				ii, m, k,
				m.template raw_access<Components>(k)...
			);
		}
	}
};

/// Constructs a new instance of func_adaptor_imkc
/**
 *  @ingroup func_adaptors
 */
template <typename ... Components, typename Functor>
inline
func_adaptor_imkc<Functor, Components...>
adapt_func_imkc(const Functor& functor)
{
	return func_adaptor_imkc<Functor, Components...>(functor);
}

/// Constructs a new instance of func_adaptor_imkc adapting a std::function
/**
 *  @ingroup func_adaptors
 */
template <typename Group, typename ... Components>
inline 
func_adaptor_imkc<
	std::function<bool(
		const iter_info&,
		manager<Group>&,
		typename manager<Group>::entity_key,
		Components...
	)>,
	Components...
> adapt_func(
	const std::function<bool(
		const iter_info&,
		manager<Group>&,
		typename manager<Group>::entity_key,
		Components...
	)>& functor
)
{
	return adapt_func_imkc<Components...>(functor);
}

} // namespace exces

#endif //include guard

