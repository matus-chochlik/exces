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
#include <exces/any/lock.hpp>
#include <functional>

namespace exces {

template <typename Entity>
class any_manager;

template <typename Entity>
struct any_manager_intf
{
	typedef typename any_entity_key::param_type aekp;

	virtual ~any_manager_intf(void) { }

	virtual bool has_key(Entity e) = 0;
	virtual any_entity_key get_key(Entity e) = 0;
	virtual Entity get_entity(aekp) = 0;

	virtual bool has(aekp, const char*) = 0;
	virtual bool has(Entity, const char*) = 0;
	virtual bool has_all(aekp, const char**) = 0;
	virtual bool has_some(aekp, const char**) = 0;

	virtual void reserve(std::size_t, const char**) = 0;
	virtual void add(aekp, const char**, void**) = 0;
	virtual void remove(aekp, const char**) = 0;
	virtual void copy(aekp, aekp, const char**) = 0;

	virtual any_lock lifetime_lock(const char*) = 0;
	virtual any_lock raw_access_lock(const char*) = 0;

	typedef std::vector<std::size_t> entity_update_op;
	virtual entity_update_op begin_update(aekp) = 0;
	virtual void finish_update(aekp, const entity_update_op&) = 0;


	virtual void* raw_access(aekp, const char*) = 0;

	virtual void for_each_imk(
		any_manager<Entity>&,
		const std::function<bool (
			const iter_info&,
			any_manager<Entity>&,
			const any_entity_key&
		)>&
	) = 0;
	virtual void for_each_mk(
		any_manager<Entity>&,
		const std::function<bool (
			any_manager<Entity>&,
			const any_entity_key&
		)>&
	) = 0;
	virtual void for_each_c(const void*, const char*) = 0;
};

template <typename Group>
std::shared_ptr<any_manager_intf<typename entity<Group>::type>>
make_any_manager_impl(manager<Group>&);

} // namespace exces

#endif //include guard

