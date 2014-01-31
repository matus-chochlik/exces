/**
 *  @example advanced/rooms/common.hpp
 *  @brief Common definitions used by almost all other files
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef EXCES_EXAMPLE_ADVANCED_ROOMS_COMMON_HPP
#define EXCES_EXAMPLE_ADVANCED_ROOMS_COMMON_HPP

#include <exces/entity.hpp>
#include <exces/implicit.hpp>

typedef exces::entity<>::type entity;
typedef exces::implicit_entity<> intity;
typedef exces::manager<> manager;
typedef exces::implicit_manager<> implicit_manager;
using   exces::classification;

struct game_data;

#endif // include guard
