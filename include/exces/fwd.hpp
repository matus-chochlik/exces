/**
 *  @file exces/fwd.hpp
 *  @brief Forward declarations.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_FWD_1304231127_HPP
#define EXCES_FWD_1304231127_HPP

namespace exces {

template <typename Component, typename Group>
struct component_id;

template <typename Component>
struct component_index;

template <typename Group>
class component_storage;

template <typename Group>
class manager;

template <typename Group>
class entity;

class iter_info;

struct component_kind_normal
{
	typedef component_kind_normal type;
};
struct component_kind_backbuf
{
	typedef component_kind_backbuf type;
};
struct component_kind_flyweight
{
	typedef component_kind_flyweight type;
};
 
struct component_access_read_only
{
	typedef component_access_read_only type;
};
struct component_access_read_write
{
	typedef component_access_read_write type;
};


} // namespace exces

#endif //include guard

