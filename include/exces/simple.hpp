/**
 *  @file exces/simple.hpp
 *  @brief Declaration of the simplified excess interface.
 *
 *  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_SIMPLE_1304231127_HPP
#define EXCES_SIMPLE_1304231127_HPP

#include <exces/exces.hpp>

namespace exces {

/// The simplified interface is defined in this namespace
/**
 *  @see excess
 */
namespace simple {

/// The entity type
typedef ::exces::entity<>::type entity;

/// The manager type
typedef ::exces::manager<> manager;

/// The classification template
using ::exces::classification;

} // namespace simple
} // namespace exces

/// A shorter alias for the exces::simple namespace
namespace excess = ::exces::simple;

#endif //include guard

