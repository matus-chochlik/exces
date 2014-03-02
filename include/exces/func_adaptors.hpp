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

// Components ...

template <typename Functor, typename ... Components>
struct func_adaptor_c
{
	Functor _functor;

	func_adaptor_c(const Functor& functor)
	 : _functor(functor)
	{ }

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

template <typename ... Components, typename Functor>
inline
func_adaptor_c<Functor, Components...>
adapt_func_c(const Functor& functor)
{
	return func_adaptor_c<Functor, Components...>(functor);
}

template <typename ... Components>
inline 
func_adaptor_c<std::function<void(Components...)>, Components...>
adapt_func(const std::function<void(Components...)>& functor)
{
	return adapt_func_c<Components...>(functor);
}

// Pointers-to-components ...

template <typename Functor, typename ... Components>
struct func_adaptor_cp
{
	Functor _functor;

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

template <typename ... Components, typename Functor>
inline
func_adaptor_cp<Functor, Components...>
adapt_func_cp(const Functor& functor)
{
	return func_adaptor_cp<Functor, Components...>(functor);
}

template <typename ... Components>
inline 
func_adaptor_cp<std::function<void(Components*...)>, Components...>
adapt_func(const std::function<void(Components*...)>& functor)
{
	return adapt_func_cp<Components...>(functor);
}

// Component-member-variables

template <typename Functor, typename MemVarType, typename Component>
struct func_adaptor_cmv
{
	Functor _functor;
	MemVarType Component::*_mem_var_ptr;

	func_adaptor_cmv(
		const Functor& functor,
		MemVarType Component::*mem_var_ptr
	): _functor(functor)
	 , _mem_var_ptr(mem_var_ptr)
	{ }

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

template <typename Functor, typename ... Components>
struct func_adaptor_mkc
{
	Functor _functor;

	func_adaptor_mkc(const Functor& functor)
	 : _functor(functor)
	{ }

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

template <typename ... Components, typename Functor>
inline
func_adaptor_mkc<Functor, Components...>
adapt_func_mkc(const Functor& functor)
{
	return func_adaptor_mkc<Functor, Components...>(functor);
}

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

template <typename Functor, typename ... Components>
struct func_adaptor_mkcp
{
	Functor _functor;

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

template <typename ... Components, typename Functor>
inline
func_adaptor_mkcp<Functor, Components...>
adapt_func_mkcp(const Functor& functor)
{
	return func_adaptor_mkcp<Functor, Components...>(functor);
}

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

template <typename Functor, typename ... Components>
struct func_adaptor_mkec
{
	Functor _functor;

	func_adaptor_mkec(const Functor& functor)
	 : _functor(functor)
	{ }

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

template <typename ... Components, typename Functor>
inline
func_adaptor_mkec<Functor, Components...>
adapt_func_mkec(const Functor& functor)
{
	return func_adaptor_mkec<Functor, Components...>(functor);
}

template <typename Group, typename ... Components>
inline 
func_adaptor_mkc<
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

