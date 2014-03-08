/**
 *  @file exces/func_adaptors.hpp
 *  @brief Adaptors for various functors making them usable with for_each
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_FUNC_ADAPTORS_1212101457_HPP
#define EXCES_FUNC_ADAPTORS_1212101457_HPP

#include <exces/metaprog.hpp>

namespace exces {

template <typename Group>
class manager;

/** @defgroup func_adaptors Function adaptors
 *
 *  Function adaptor classes can be used to adapt various functors
 *  to the interface required by the manager::for_each, collection::for_each
 *  or classification::for_each entity traversal member functions.
 */

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
	void operator()(
		manager<Group>& m,
		typename manager<Group>::entity_key k
	)
	{
		if(m.template has_all<Components...>(k))
		{
			_functor(m.template raw_access<Components>(k)...);
		}
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
func_adaptor_c<std::function<void(Components...)>, Components...>
adapt_func(const std::function<void(Components...)>& functor)
{
	return adapt_func_c<Components...>(functor);
}

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
	void operator()(
		manager<Group>& m,
		typename manager<Group>::entity_key k
	)
	{
		if(m.template has_some<Components...>(k))
		{
			_functor(_get_ptr<Group, Components>(m, k)...);
		}
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
func_adaptor_cp<std::function<void(Components*...)>, Components...>
adapt_func(const std::function<void(Components*...)>& functor)
{
	return adapt_func_cp<Components...>(functor);
}

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
	void operator()(
		manager<Group>& m,
		typename manager<Group>::entity_key k
	)
	{
		if(m.template has<Component>(k))
		{
			_functor(
				m.template raw_access<Component>(k)
					.*_mem_var_ptr
			);
		}
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

// Manager, Key, Components ...

/// Similar to func_adaptor_c but also accepting a reference to manager and a key
/** This adaptor is similar to func_adaptor_c, but the functions adapted by this
 *  adaptor accept in addition to the specified Components also a reference
 *  to the manager and an entity key as the first and second argument respectivelly.
 *
 *  @ingroup func_adaptors
 *
 *  @see adapt_func_mkc
 *  @see func_adaptor_c
 */
template <typename Functor, typename ... Components>
struct func_adaptor_mkc
{
	Functor _functor;

	/// Adapts the specified functor
	func_adaptor_mkc(const Functor& functor)
	 : _functor(functor)
	{ }

	/// The function call operator
	template <typename Group>
	void operator()(
		manager<Group>& m,
		typename manager<Group>::entity_key k
	)
	{
		if(m.template has_all<Components...>(k))
		{
			_functor(
				m, k,
				m.template raw_access<Components>(k)...
			);
		}
	}
};

/// Constructs a new instance of func_adaptor_mkc
/**
 *  @ingroup func_adaptors
 */
template <typename ... Components, typename Functor>
inline
func_adaptor_mkc<Functor, Components...>
adapt_func_mkc(const Functor& functor)
{
	return func_adaptor_mkc<Functor, Components...>(functor);
}

/// Constructs a new instance of func_adaptor_mkc adapting a std::function
/**
 *  @ingroup func_adaptors
 */
template <typename Group, typename ... Components>
inline 
func_adaptor_mkc<
	std::function<void(
		manager<Group>&,
		typename manager<Group>::entity_key,
		Components...
	)>,
	Components...
> adapt_func(
	const std::function<void(
		manager<Group>&,
		typename manager<Group>::entity_key,
		Components...
	)>& functor
)
{
	return adapt_func_mkc<Components...>(functor);
}

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
	void operator()(
		manager<Group>& m,
		typename manager<Group>::entity_key k
	)
	{
		if(m.template has_some<Components...>(k))
		{
			_functor(m, k, _get_ptr<Group, Components>(m, k)...);
		}
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
	std::function<void(
		manager<Group>&,
		typename manager<Group>::entity_key,
		Components*...
	)>,
	Components...
> adapt_func(
	const std::function<void(
		manager<Group>&,
		typename manager<Group>::entity_key,
		Components*...
	)>& functor
)
{
	return adapt_func_mkcp<Components...>(functor);
}

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
	void operator()(
		manager<Group>& m,
		typename manager<Group>::entity_key k
	)
	{
		if(m.template has_all<Components...>(k))
		{
			_functor(
				m, k, m.get_entity(k),
				m.template raw_access<Components>(k)...
			);
		}
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
	std::function<void(
		manager<Group>&,
		typename manager<Group>::entity_key,
		typename entity<Group>::type,
		Components...
	)>,
	Components...
> adapt_func(
	const std::function<void(
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

