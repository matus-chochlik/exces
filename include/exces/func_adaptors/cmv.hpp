/**
 *  @file exces/func_adaptors/cmv.hpp
 *  @brief Adaptors for various functors making them usable with for_each
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_FUNC_ADAPTORS_CMV_1404292124_HPP
#define EXCES_FUNC_ADAPTORS_CMV_1404292124_HPP

#include <exces/fwd.hpp>
#include <functional>

namespace exces {

// Component-member-variables

/// Adapts a functor accepting a member variable of a Component for use with for_each
/** This adaptor adapts a functor, that accepts a (reference to a) member variable
 *  of the MemVarType of a Component so that it can be used with the for_each
 *  member function of manager, collection or classification.
 *
 *  @ingroup func_adaptors
 *
 *  @see adapt_func_cmv
 */
template <typename Functor, typename MemVarType, typename Component>
struct func_adaptor_cmv
{
	Functor _functor;
	MemVarType Component::*_mem_var_ptr;

	/// Adapts the specified functor
	func_adaptor_cmv(
		const Functor& functor,
		MemVarType Component::*mem_var_ptr
	): _functor(functor)
	 , _mem_var_ptr(mem_var_ptr)
	{ }

	/// The function call operator
	/** If the entity managed by manager @p m, referenced by key @p k
	 *  has the specified Component, then the adapted functor is called
	 *  by using manager::raw_access to get the references to the component
	 *  instance and a pointer to Component's member variable to get
	 *  the reference of the member variable.
	 */
	template <typename Group>
	bool operator()(
		const iter_info&,
		manager<Group>& m,
		typename manager<Group>::entity_key k
	)
	{
		if(m.template has<Component>(k))
		{
			if(!_functor(
				m.template raw_access<Component>(k)
					.*_mem_var_ptr
			)) return false;
		}
		return true;
	}
};

/// Constructs a new instance of func_adaptor_cmv
/**
 *  @ingroup func_adaptors
 */
template <typename MemVarType, typename Component, typename Functor>
inline
func_adaptor_cmv<Functor, MemVarType, Component>
adapt_func_cmv(MemVarType Component::*mem_var_ptr, const Functor& functor)
{
	return func_adaptor_cmv<Functor, MemVarType, Component>(
		functor,
		mem_var_ptr
	);
}

} // namespace exces

#endif //include guard

