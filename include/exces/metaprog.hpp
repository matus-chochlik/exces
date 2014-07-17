/**
 *  @file exces/metaprog.hpp
 *  @brief Meta-programming utilities
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_METAPROG_1304231127_HPP
#define EXCES_METAPROG_1304231127_HPP

#include <exces/detail/metaprog.hpp>

namespace exces {

using mp::typelist;
using mp::get;
using mp::tuple;
using mp::make_tuple;

} // namespace exces

#define EXCES_INSTANTIATE(TYPE) { auto f = &TYPE::_instantiate; if(!f&&f) f(); }

#endif //include guard

