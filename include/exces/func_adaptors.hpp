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

namespace exces {

/** @defgroup func_adaptors Function adaptors
 *
 *  Function adaptor classes can be used to adapt various functors
 *  to the interface required by the manager::for_each, collection::for_each
 *  or classification::for_each entity traversal member functions.
 */

} // namespace exces

#include <exces/func_adaptors/c.hpp>
#include <exces/func_adaptors/ic.hpp>
#include <exces/func_adaptors/cp.hpp>
#include <exces/func_adaptors/icp.hpp>
#include <exces/func_adaptors/cmv.hpp>
#include <exces/func_adaptors/mkc.hpp>
#include <exces/func_adaptors/imkc.hpp>
#include <exces/func_adaptors/mkcp.hpp>
#include <exces/func_adaptors/imkcp.hpp>
#include <exces/func_adaptors/mkec.hpp>

#endif //include guard

