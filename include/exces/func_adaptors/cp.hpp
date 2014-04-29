/**
 *  @file exces/func_adaptors/cp.hpp
 *  @brief Adaptors for various functors making them usable with for_each
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_FUNC_ADAPTORS_CP_1404292124_HPP
#define EXCES_FUNC_ADAPTORS_CP_1404292124_HPP

#include <exces/fwd.hpp>
#include <functional>

namespace exces {

// Pointers-to-components ...

/// Adapts a functor accepting a pack of Component pointers for use with for_each
/** This adaptor adapts a functor, that accepts one or several pointers to
 *  various Components so that it can be used with the for_each member function
 *  of manager, collection or classification.
 *
 *  @ingroup func_adaptors
 *
 *  @see adapt_func_cp
 */
template <typename Functor, typename ... Components>
struct func_adaptor_cp
{
	Functor _functor;

	/// Adapts the specified functor
	func_adaptor_cp(const Functor& functor)
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
	/** If the entity managed by manager @p m, referenced by key @p k
	 *  has some of the specified Components, the adapted functor is called
	 *  by using manager::raw_access to get the references to the component
	 *  instances. If the entity has the i-th component then the address
	 *  of the component instance is passed to the functor, if the entity
	 *  doesn't have the i-th component then a null pointer is passed.
	 */
	template <typename Group>
	bool operator()(
		const iter_info&,
		manager<Group>& m,
		typename manager<Group>::entity_key k
	)
	{
		if(m.template has_some<Components...>(k))
		{
			if(!_functor(_get_ptr<Group, Components>(m, k)...))
				return false;
		}
		return true;
	}
};

/// Constructs a new instance of func_adaptor_cp
/**
 *  @ingroup func_adaptors
 */
template <typename ... Components, typename Functor>
inline
func_adaptor_cp<Functor, Components...>
adapt_func_cp(const Functor& functor)
{
	return func_adaptor_cp<Functor, Components...>(functor);
}

/// Constructs a new instance of func_adaptor_cp adapting a std::function
/**
 *  @ingroup func_adaptors
 */
template <typename ... Components>
inline 
func_adaptor_cp<std::function<bool(Components*...)>, Components...>
adapt_func(const std::function<bool(Components*...)>& functor)
{
	return adapt_func_cp<Components...>(functor);
}

} // namespace exces

#endif //include guard

