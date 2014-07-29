/**
 *  @file exces/detail/metaprog.hpp
 *  @brief Implementation of meta-programming utilities
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_AUX_METAPROG_1304231127_HPP
#define EXCES_AUX_METAPROG_1304231127_HPP

#include <type_traits>
#include <tuple>

namespace exces {
namespace mp {

using std::get;
using std::tuple;
using std::make_tuple;
using std::tie;
using std::integral_constant;

// type traits
// TODO
//using std::is_trivially_destructible;
template <typename T>
struct is_trivially_destructible
 : std::false_type
{ };

// TODO
template <typename T>
struct is_trivially_copyable
 : std::false_type
{ };

// nil type
struct nil
{
	typedef nil type;
};

// identity type
template <typename T>
struct identity
{
	typedef T type;
};

// size_t
template <std::size_t I>
struct size_t_ : integral_constant<std::size_t, I>
{
	typedef size_t_ type;
};

// if
template <bool, typename True, typename False>
struct if_c
{
	typedef True type;
};

template <typename True, typename False>
struct if_c<false, True, False>
{
	typedef False type;
};

// and_c
template <bool ... Values>
struct and_c;

template <>
struct and_c<>
 : integral_constant<bool, true>
{ };

template <bool Value1, bool Value2>
struct and_c<Value1, Value2>
 : integral_constant<bool, Value1 && Value2>
{ };

template <bool Value, bool ... Values>
struct and_c<Value, Values...>
 : integral_constant<bool, Value && and_c<Values...>::value>
{ };

// type list
template <typename ... T>
struct typelist
{
	typedef typelist<T...> type;
};

template <typename Typelist, typename N>
struct push_back
 : push_back<typename Typelist::type, N>
{ };

template <typename Typelist>
struct size
 : size<typename Typelist::type>
{ };

template <typename ... T, typename N>
struct push_back<typelist<T...>, N>
 : typelist<T..., N>
{ };

template <typename ... T>
struct size<typelist<T...> >
 : size_t_<sizeof ... (T)>
{ };

template <typename ... T>
struct size<tuple<T...> >
 : size_t_<sizeof ... (T)>
{ };

template <typename Typelist, typename UnaryMFC>
struct transform
 : transform<typename Typelist::type, UnaryMFC>
{ };

template <typename ... T, typename UnaryMFC>
struct transform<typelist<T...>, UnaryMFC>
 : typelist<
	typename UnaryMFC::template apply<T>::type ...
> { };

// binary search tree
template <typename Left, typename Data, typename Right>
struct typetree
{
	typedef typetree<Left, Data, Right> type;
};

// insert
template <typename Tree, typename Elem, typename Cmp>
struct insert;

template <typename E, typename Cmp>
struct insert<nil, E, Cmp>
 : typetree<nil, E, nil>
{ };

template <typename L, typename D, typename R, typename E, typename Cmp>
struct insert<typetree<L, D, R>, E, Cmp>
 : if_c<
	Cmp::template apply<E, D>::value,
	typetree<typename insert<L, E, Cmp>::type, D, R>,
	typetree<L, D, typename insert<R, E, Cmp>::type>
>{ };

// list2tree
template <typename List, typename Tree, typename Cmp>
struct list2tree;

template <typename Tree, typename Cmp>
struct list2tree<typelist<>, Tree, Cmp>
 : Tree
{ };

template <typename H, typename ... T, typename Tree, typename Cmp>
struct list2tree<typelist<H, T...>, Tree, Cmp>
 : list2tree<typelist<T...>, typename insert<Tree, H, Cmp>::type, Cmp>
{ };

// tree2list
template <typename Tree, typename List>
struct tree2list;

template <typename List>
struct tree2list<nil, List>
 : List
{ };

template <typename L, typename D, typename R, typename List>
struct tree2list<typetree<L, D, R>, List>
 : tree2list<R, typename push_back<typename tree2list<L, List>::type, D>::type>
{ };

// conversions
template <typename Sequence>
struct as_tuple
 : as_tuple<typename Sequence::type>
{ };

template <typename Sequence>
struct as_typelist
 : as_typelist<typename Sequence::type>
{ };

template <>
struct as_typelist<nil>
 : typelist<>
{ };

template <typename ... T>
struct as_tuple<tuple<T...> >
{
	typedef tuple<T...> type;
};

template <typename ... T>
struct as_tuple<typelist<T...> >
{
	typedef tuple<T...> type;
};

template <typename ... T>
struct as_typelist<typelist<T...> >
{
	typedef typelist<T...> type;
};

template <typename ... T>
struct as_typelist<tuple<T...> >
{
	typedef typelist<T...> type;
};

template <typename L, typename D, typename R>
struct as_typelist<typetree<L, D, R>>
 : tree2list<typetree<L, D, R>, typelist<>>
{ };

template <typename Typelist, typename Cmp>
struct as_typetree
 : list2tree<Typelist, nil, Cmp>
{ };

// sort
template <typename Sequence, typename Cmp>
struct sort;

template <typename ... T, typename Cmp>
struct sort<typelist<T...>, Cmp>
 : as_typelist<as_typetree<typelist<T...>, Cmp>>
{ };

// for-each
template <typename Func, template <class...> class Seq>
inline void _for_each_typ(Func& func, Seq<>*) { }

template <
	typename Func,
	typename Head,
	typename ... Tail,
	template <class...> class Seq
> inline void _for_each_typ(Func& func, Seq<Head, Tail...>*)
{
	func(identity<Head>());
	_for_each_typ(func, (Seq<Tail...>*)nullptr);
}

template <typename Sequence, typename Func>
inline void for_each(Func func)
{
	Sequence* pseq = nullptr;
	_for_each_typ(func, pseq);
}

template <typename Sequence, typename Func>
inline void for_each_ref(Func& func)
{
	Sequence* pseq = nullptr;
	_for_each_typ(func, pseq);
};

template <typename Tuple, typename Func>
inline void _for_each_val(Tuple& tup, Func& func, size_t_<0>)
{
	func(get<0>(tup));
}

template <typename Tuple, typename Func, std::size_t I>
inline void _for_each_val(Tuple& tup, Func& func, size_t_<I>)
{
	_for_each_val(tup, func, size_t_<I-1>());
	func(get<I>(tup));
}

template <typename Tuple, typename Func>
inline void _call_for_each_val(Tuple& tup, Func& func, size_t_<0>)
{
}

template <typename Tuple, typename Func, std::size_t N>
inline void _call_for_each_val(Tuple& tup, Func& func, size_t_<N>)
{
	_for_each_val(tup, func, size_t_<N-1>());
}

template <typename Tuple, typename Func>
inline void for_each(Tuple& tup, Func func)
{
	_call_for_each_val(
		tup,
		func,
		size<typename std::remove_const<Tuple>::type>()
	);
};

template <typename Tuple, typename Func>
inline void for_each_ref(Tuple& tup, Func& func)
{
	_call_for_each_val(
		tup,
		func,
		size<typename std::remove_const<Tuple>::type>()
	);
};


// sequence of integers
template <std::size_t ... S>
struct n_seq
{
	typedef n_seq type;
};

// generator of interger sequence
template <std::size_t N, std::size_t ... S>
struct gen_seq : gen_seq<N-1, N-1, S...>
{ };

template <std::size_t ... S>
struct gen_seq<0, S...>
 : n_seq<S...>
{ };

template <typename OldT, typename NewT>
struct instead
{
	typedef NewT type;
};

} // namespace mp
} // namespac exces

#endif //include guard

