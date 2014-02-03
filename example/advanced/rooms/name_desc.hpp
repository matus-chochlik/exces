/**
 *  @example advanced/rooms/name_desc.hpp
 *  @brief The name and description components.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef EXCES_EXAMPLE_ADVANCED_ROOMS_NAME_DESC_HPP
#define EXCES_EXAMPLE_ADVANCED_ROOMS_NAME_DESC_HPP

#include "common.hpp"
#include <string>

struct name : std::string
{
	name(std::string&& str)
	 : std::string(std::move(str))
	{ }
};

struct description : std::string
{
	description(std::string&& str)
	 : std::string(std::move(str))
	{ }
};

extern void brief_description(game_data&, intity player, intity item);
extern void full_description(game_data&, intity player, intity item);

#endif // include guard
