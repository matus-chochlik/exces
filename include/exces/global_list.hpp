/**
 *  @file exces/global_list.hpp
 *  @brief Implements a compile-time linked type lists
 *
 *  Copyright 2008-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_GLOBAL_LIST_1011291729_HPP
#define EXCES_GLOBAL_LIST_1011291729_HPP

#include <exces/counter.hpp>
#include <exces/metaprog.hpp>

namespace exces {
namespace cntr {

// Empty global list
template <typename Selector, typename Counter>
struct global_list;

template <typename Selector, typename Counter>
struct global_list<sel<Selector>, Counter > : mp::typelist<>
{ };

} // namespace cntr

/** @defgroup global_lists Global type lists
 *
 *  This module contains a set of macros which allow to define new
 *  compile-time type lists, append new items to the lists during
 *  source compilation and to get the range of types in these
 *  type lists.
 */

#ifdef EXCES_DOCUMENTATION_ONLY
/// Declares a new global type list with the @a SELECTOR type as identifier
/** This macro declares and initializes a new empty compile time
 *  linked type list.
 *
 *  @see EXCES_ADD_TO_GLOBAL_LIST
 *  @see EXCES_GET_GLOBAL_LIST
 *  @param SELECTOR the type which servers as the global type list identifier
 *  @ingroup global_lists
 */
#define EXCES_REGISTER_GLOBAL_LIST(SELECTOR)
#else
#define EXCES_REGISTER_GLOBAL_LIST(SELECTOR) \
	EXCES_REGISTER_COUNTER_SELECTOR(SELECTOR)
#endif

// Helper macro used in implementation of global lists
#define EXCES_GET_GLOBAL_LIST_BASE_WRAPPER(SELECTOR, LUID) \
exces::cntr::global_list< \
	exces::cntr::sel< SELECTOR >, \
	EXCES_COUNTER_PREVIOUS_COUNT(SELECTOR, LUID) \
>

// Helper macro used in implementation of global lists
#define EXCES_GET_GLOBAL_LIST_BASE(SELECTOR, LUID) \
	EXCES_GET_GLOBAL_LIST_BASE_WRAPPER(SELECTOR, LUID)::type

#define EXCES_ADD_TO_GLOBAL_LIST_BASE(SELECTOR, ITEM, LUID) \
EXCES_COUNTER_INCREMENT_COUNTER(SELECTOR, LUID) \
namespace cntr { \
template <> \
struct global_list< \
	exces::cntr::sel< SELECTOR >, \
	EXCES_COUNTER_CURRENT_COUNT(SELECTOR, LUID) \
> : mp::push_back< EXCES_GET_GLOBAL_LIST_BASE_WRAPPER(SELECTOR, LUID), ITEM > \
{ }; \
}

/// Expands into a compile-time type list identified by the @a SELECTOR
/**
 *  @see EXCES_REGISTER_GLOBAL_LIST
 *  @see EXCES_ADD_TO_GLOBAL_LIST
 *  @param SELECTOR the type serving as the type list identifier
 *  @ingroup global_lists
 */
#define EXCES_GET_GLOBAL_LIST(SELECTOR) \
	EXCES_GET_GLOBAL_LIST_BASE(SELECTOR, EXCES_COUNTER_LUID)

#ifdef EXCES_DOCUMENTATION_ONLY

/// Appends a new @a ITEM into the type list identified by the @a SELECTOR
/**
 *  @see EXCES_REGISTER_GLOBAL_LIST
 *  @see EXCES_GET_GLOBAL_LIST
 *  @param SELECTOR the identifier of the compile-time type list
 *  @param ITEM the item to be added to the compile-time type list
 *  @ingroup global_lists
 */
#define EXCES_ADD_TO_GLOBAL_LIST(SELECTOR, ITEM)
#else

#define EXCES_ADD_TO_GLOBAL_LIST(SELECTOR, ITEM) \
	EXCES_ADD_TO_GLOBAL_LIST_BASE(SELECTOR, ITEM, EXCES_COUNTER_LUID)


#endif // !EXCES_DOCUMENTATION_ONLY

} // namespace exces

#endif //include guard

