/**
 *  @file exces/metaprog.hpp
 *  @brief Meta-programming utilities
 *
 *  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_METAPROG_1304231127_HPP
#define EXCES_METAPROG_1304231127_HPP

#include <type_traits>
#include <tuple>

namespace exces {
namespace mp {

using std::get;
using std::tuple;
using std::make_tuple;
using std::tie;

template <typename T>
struct identity
{
	typedef T type;
};

template <std::size_t I>
struct size_t_ : ::std::integral_constant<std::size_t, I>
{
	typedef size_t_ type;
};

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

template <typename Sequence>
struct as_tuple
 : as_tuple<typename Sequence::type>
{ };

template <typename Sequence>
struct as_typelist
 : as_typelist<typename Sequence::type>
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

} // namespace mp

using mp::typelist;
using mp::get;
using mp::tuple;
using mp::make_tuple;

} // namespac exces

#endif //include guard

