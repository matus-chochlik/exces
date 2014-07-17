/**
 *  @file exces/detail/counter.hpp
 *  @brief Helper template class for creating compile-time counters
 *
 *  Copyright 2008-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_AUX_COUNTER_1011291729_HPP
#define EXCES_AUX_COUNTER_1011291729_HPP

#include <exces/detail/metaprog.hpp>

#ifndef EXCES_COUNTER_LUID
# ifdef __COUNTER__
#  define EXCES_COUNTER_LUID __COUNTER__
# else
#  error "__COUNTER__ is required"
# endif
#endif

namespace exces {

/// This namespace contains everything related to compile time counters
namespace cntr {

/** @defgroup ct_counters Compile-time counters
 *
 *  This module contains a set of macros for declaring, initializing,
 *  incrementing and getting the current and the previews value
 *  of compile-time counters.
 */

// local null-type
struct null_ { };

// the initial value of the counter
typedef exces::mp::size_t_<-1> initial;

// the zero value
typedef exces::mp::size_t_<0> zero;

// a fallback version of the get_next function
template <class Selector>
null_ get_next(Selector, ...);

// the get_next overload for the first incrementation
template <class Selector>
zero get_next(Selector, initial);

// forward declaration of the current counter template
template <class Selector, class Value, int InstId>
struct curr;

// helper template returning the next counter value for the Selector
template <
	class Selector,
	class Current,
	class Next,
	int InstId
>
struct nextval
{
	typedef typename curr<
		Selector,
		Next,
		InstId
	>::type type;
};

// Specialization of nextval for the current value
template <class Selector, class Current, int InstId>
struct nextval<Selector, Current, null_, InstId>
{
	typedef Current type;
};

// Returns the current value of the counter with the given Selector
template <class Selector, class Value, int InstId>
struct curr : nextval<
	Selector,
	Value,
	decltype(get_next(Selector(), Value())),
	InstId
>::type
{ };

// Forward declaration of the selector template
template <class T> struct sel;

} // namespace cntr
} // namespace exces

#ifdef EXCES_DOCUMENTATION_ONLY
/// Registers a new compile-time counter identified by the SELECTOR type
/** This macro registers a selector for the given (fully qualified)
 *  type.
 *
 *  @param SELECTOR name of a type which acts as the identifier of the counter
 *  @ingroup ct_counters
 */
#define EXCES_REGISTER_COUNTER_SELECTOR(SELECTOR)
#else
#define EXCES_REGISTER_COUNTER_SELECTOR(SELECTOR) \
namespace cntr { \
template <> struct sel < SELECTOR > { }; \
} /* namespace cntr */
#endif

// Helper macro used in implementation of EXCES_COUNTER_CURRENT
#define EXCES_COUNTER_CURRENT_COUNT(SELECTOR, LUID) \
	::exces::cntr::curr< \
		::exces::cntr::sel< SELECTOR >, \
		::exces::cntr::initial, \
		LUID \
	>::type

// Helper macro used for implementation of EXCES_COUNTER_PREVIOUS
#define EXCES_COUNTER_PREVIOUS_COUNT(SELECTOR, LUID) \
	::exces::mp::size_t_< \
		EXCES_COUNTER_CURRENT_COUNT(SELECTOR, LUID)::value - 1 \
	>


// Helper macro used in implementation of EXCES_COUNTER_INCREMENT
#define EXCES_COUNTER_INCREMENT_COUNTER(SELECTOR, LUID) \
namespace cntr { \
	exces::mp::size_t_< curr< sel< SELECTOR >, initial, LUID >::value + 1 >\
	get_next( \
		sel< SELECTOR >, \
		curr< sel< SELECTOR >, initial, LUID >::type \
	); \
} /* namespace cntr */

/// Returns the value of a compile-time counter identified by the SELECTOR
/** This macro expands into a compile-time constant type representing the value
 *  of a counter identified by the SELECTOR type.
 *  The resulting type has the same interface as mp::size_t_<I>
 *
 *  @param SELECTOR name of a type which acts as the identifier of the counter
 *  @ingroup ct_counters
 */
#define EXCES_COUNTER_CURRENT(SELECTOR) \
	EXCES_COUNTER_CURRENT_COUNT( \
		SELECTOR, \
		EXCES_COUNTER_LUID \
	)

/// Returns the previous value of a compile-time counter identified by SELECTOR
/** This macro expands into a compile-time constant type representing the
 *  previous value of a counter identified by the SELECTOR type.
 *  The resulting type has the same interface as mp::size_t_<I>.
 *
 *  @param SELECTOR name of a type which acts as the identifier of the counter
 *  @ingroup ct_counters
 */
#define EXCES_COUNTER_PREVIOUS(SELECTOR) \
	EXCES_COUNTER_PREVIOUS_COUNT( \
		SELECTOR, \
		EXCES_COUNTER_LUID \
	)

/// Increments the value of a compile-time counter identified by the SELECTOR
/** This macro increments the value of a counter identified by the SELECTOR
 *  type.
 *
 *  After the use of this macro in the source the value of the counter is
 *  incremented by one.
 *
 *  @param SELECTOR name of a type which acts as the identifier of the counter
 *  @ingroup ct_counters
 */
#define EXCES_COUNTER_INCREMENT(SELECTOR) \
	EXCES_COUNTER_INCREMENT_COUNTER(SELECTOR, EXCES_COUNTER_LUID)

#endif //include guard

