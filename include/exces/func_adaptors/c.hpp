/**
 *  @file exces/func_adaptors/c.hpp
 *  @brief Adaptors for various functors making them usable with for_each
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_FUNC_ADAPTORS_C_1404292124_HPP
#define EXCES_FUNC_ADAPTORS_C_1404292124_HPP

#include <exces/fwd.hpp>
#include <functional>

namespace exces {

// Components ...

/// Adapts a functor that accepts a pack of Components for use with for_each
/** This adaptor adapts a functor, that accepts one or several (references to)
 *  various Components so that it can be used with the for_each member function
 *  of manager, collection or classification.
 *
 *  @ingroup func_adaptors
 *
 *  @see adapt_func_c
 */
template <typename Functor, typename ... Components>
struct func_adaptor_c
{
	Functor _functor;

	/// Adapts the specified functor
	func_adaptor_c(const Functor& functor)
	 : _functor(functor)
	{ }

	/// The function call operator
	/** If the entity managed by manager @p m, referenced by key @p k
	 *  has all specified Components, the adapted functor is called
	 *  by using manager::raw_access to get the references to the component
	 *  instances passed as arguments to the adapted functor.
	 */
	template <typename Group>
	bool operator()(
		const iter_info&,
		manager<Group>& m,
		typename manager<Group>::entity_key k
	)
	{
		if(m.template has_all<Components...>(k))
		{
			if(!_functor(m.template raw_access<Components>(k)...))
				return false;
		}
		return true;
	}
};

/// Constructs a new instance of func_adaptor_c
/**
 *  @ingroup func_adaptors
 */
template <typename ... Components, typename Functor>
inline
func_adaptor_c<Functor, Components...>
adapt_func_c(const Functor& functor)
{
	return func_adaptor_c<Functor, Components...>(functor);
}

/// Constructs a new instance of func_adaptor_c adapting a std::function
/**
 *  @ingroup func_adaptors
 */
template <typename ... Components>
inline 
func_adaptor_c<std::function<bool(Components...)>, Components...>
adapt_func(const std::function<bool(Components...)>& functor)
{
	return adapt_func_c<Components...>(functor);
}

} // namespace exces

#endif //include guard

