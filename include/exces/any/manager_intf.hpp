/**
 *  @file exces/any/manager_intf.hpp
 *  @brief Interface of manager type erasure implementation
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_ANY_ENTITY_MANAGER_INTF_1403152314_HPP
#define EXCES_ANY_ENTITY_MANAGER_INTF_1403152314_HPP

#include <exces/any/entity_key.hpp>

namespace exces {

template <typename Entity>
struct any_manager_intf
{
	typedef typename any_entity_key::param_type any_entity_key_param;

	virtual ~any_manager_intf(void) { }

	virtual bool has_key(Entity e) = 0;
	virtual any_entity_key get_key(Entity e) = 0;
	virtual Entity get_entity(any_entity_key_param) = 0;
	virtual bool has(any_entity_key_param, const char*) = 0;
	virtual bool has(Entity, const char*) = 0;

	virtual void* raw_access(any_entity_key_param, const char*) = 0;
};

template <typename Group>
std::shared_ptr<any_manager_intf<typename entity<Group>::type>>
make_any_manager_impl(manager<Group>&);

} // namespace exces

#endif //include guard

