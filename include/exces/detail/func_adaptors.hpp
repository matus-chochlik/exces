/**
 *  @file exces/detail/func_adaptors/c.hpp
 *  @brief Helpers used by function adaptors
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_DETAIL_FUNC_ADAPTORS_1404292124_HPP
#define EXCES_DETAIL_FUNC_ADAPTORS_1404292124_HPP

#include <exces/fwd.hpp>

namespace exces {
namespace aux_ {

template <typename ... Components>
struct auto_update_func_adaptor
{
	struct NoOp { };

	template <typename Group>
	static NoOp _begin_update(
		component_access_read_only,
		const manager<Group>&,
		typename manager<Group>::entity_key
	)
	{
		return NoOp();
	}

	template <typename Group>
	static typename manager<Group>::entity_update_op
	_begin_update(
		component_access_read_write,
		manager<Group>& m,
		typename manager<Group>::entity_key k
	)
	{
		return m.begin_update(k);
	}

	template <typename Group>
	static auto begin_update(
		manager<Group>& m,
		typename manager<Group>::entity_key k
	) -> decltype(
		_begin_update(
			get_component_access<Components...>(),
			m, k
		)
	)
	{
		return _begin_update(
			get_component_access<Components...>(),
			m, k
		);
	}

	template <typename Group>
	static void _finish_update(
		component_access_read_only,
		const manager<Group>&,
		typename manager<Group>::entity_key,
		NoOp
	)
	{ }

	template <typename Group>
	static void _finish_update(
		component_access_read_write,
		manager<Group>& m,
		typename manager<Group>::entity_key k,
		typename manager<Group>::entity_update_op& op
	)
	{
		m.finish_update(k, op);
	}

	template <typename Group, typename Op>
	static void finish_update(
		manager<Group>& m,
		typename manager<Group>::entity_key k,
		Op& op
	)
	{
		_finish_update(
			get_component_access<Components...>(),
			m, k, op
		);
	}
};

} // namespace aux_
} // namespace exces

#endif //include guard

