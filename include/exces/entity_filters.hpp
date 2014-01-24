/**
 *  @file exces/entity_filters.hpp
 *  @brief Implements entity predicate functors
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_ENTITY_FILTERS_1212101457_HPP
#define EXCES_ENTITY_FILTERS_1212101457_HPP

#include <exces/metaprog.hpp>

namespace exces {

template <typename Group>
class manager;

template <typename Sequence>
struct entity_with_seq
{
	template <typename Group>
	bool operator ()(
		manager<Group>& m,
		typename manager<Group>::entity_key key
	) const
	{
		return m.has_all_seq(key, Sequence());
	}
};

/// Returns true for entities that has all specified Components
template <typename ... Components>
struct entity_with
 : entity_with_seq<mp::typelist<Components...>>
{ };

namespace aux_ {

struct func_adaptor_base
{
	template <typename C>
	struct _fix
	 : std::remove_cv<typename std::remove_reference<C>::type>
	{ };

	template <typename C>
	struct _wr
	 : std::integral_constant<
		bool,
		std::is_reference<C>::value &&
		not(std::is_const<C>::value)
	>{ };

	template <typename Component, typename Group>
	static auto _do_get_c(
		manager<Group>& man,
		typename manager<Group>::entity_key key,
		std::false_type /*write*/
	) -> decltype( man.template read<Component>(key))
	{
		return man.template read<Component>(key);
	}

	template <typename Component, typename Group>
	static auto _do_get_c(
		manager<Group>& man,
		typename manager<Group>::entity_key key,
		std::true_type /*write*/
	) -> decltype( man.template write<Component>(key))
	{
		return man.template write<Component>(key);
	}

	template <typename Component, typename Group>
	static auto _get_c(
		manager<Group>& man,
		typename manager<Group>::entity_key key
	) -> decltype(
		_do_get_c<typename _fix<Component>::type>(
			man,
			key,
			_wr<Component>()
		)
	)
	{
		return _do_get_c<typename _fix<Component>::type>(
			man,
			key,
			_wr<Component>()
		);
	}
};

} // namespace aux_

template <typename Functor, typename ... Components>
struct func_adaptor_cs
 : aux_::func_adaptor_base
{
	typedef aux_::func_adaptor_base _base;
	using _base::_get_c;

	Functor _functor;

	func_adaptor_cs(const Functor& functor)
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
			_functor(_get_c<Components>(m, k)...);
		}
	}
};

template <typename ... Components, typename Functor>
inline
func_adaptor_cs<Functor, Components...>
adapt_func_cs(const Functor& functor)
{
	return func_adaptor_cs<Functor, Components...>(functor);
}

template <typename Functor, typename MemVarType, typename Component>
struct func_adaptor_cmv
 : aux_::func_adaptor_base
{
	typedef aux_::func_adaptor_base _base;
	using _base::_get_c;

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
			_functor(_get_c<Component>(m, k).*_mem_var_ptr);
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

template <typename Functor, typename ... Components>
struct func_adaptor_mkc
 : aux_::func_adaptor_base
{
	typedef aux_::func_adaptor_base _base;
	using _base::_get_c;

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
			_functor(m, k, _get_c<Components>(m, k)...);
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

template <typename Functor>
struct func_adaptor_mke
{
	Functor _functor;

	func_adaptor_mke(const Functor& functor)
	 : _functor(functor)
	{ }

	template <typename Group>
	void operator()(
		manager<Group>& m,
		typename manager<Group>::entity_key k
	)
	{
		_functor(m, k, m.get_entity(k));
	}
};

template <typename Functor>
inline
func_adaptor_mke<Functor>
adapt_func_mke(const Functor& functor)
{
	return func_adaptor_mke<Functor>(functor);
}

template <typename Functor, typename ... Components>
struct func_adaptor_mkec
 : aux_::func_adaptor_base
{
	typedef aux_::func_adaptor_base _base;
	using _base::_get_c;

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
				m, k,
				m.get_entity(k),
				_get_c<Components>(m, k)...
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

} // namespace exces

#endif //include guard

