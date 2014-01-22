/**
 *  @file exces/implicit.inl
 *  @brief Implementation of implicit manager and entity functions
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <stack>
#include <utility>

namespace exces {

//------------------------------------------------------------------------------
// implicit_manager
//------------------------------------------------------------------------------
template <typename Group>
static inline
std::stack<manager<Group>*>&
implicit_manager_stack(void)
{
	static std::stack<manager<Group>*> _ms;
	return _ms;
}
//------------------------------------------------------------------------------
template <typename Group>
implicit_manager<Group>::
implicit_manager(manager<Group>& m)
 : _m(m)
{
	implicit_manager_stack<Group>().push(&_m);
}
//------------------------------------------------------------------------------
template <typename Group>
implicit_manager<Group>::
~implicit_manager(void)
{
	assert(!implicit_manager_stack<Group>().empty());
	assert(implicit_manager_stack<Group>().top() == &_m);
	implicit_manager_stack<Group>().pop();
}
//------------------------------------------------------------------------------
template <typename Group>
manager<Group>& 
implicit_manager<Group>::
get(void)
{
	assert(!implicit_manager_stack<Group>().empty());
	assert(implicit_manager_stack<Group>().top() != nullptr);
	return *(implicit_manager_stack<Group>().top());
}
//------------------------------------------------------------------------------
// forced instantiation
//------------------------------------------------------------------------------
template <typename Group>
void
implicit_manager<Group>::
_instantiate(void)
{
	manager<Group> m;
	bool check = (&implicit_manager<Group>(m).get() == &m);
	assert(check);
}
//------------------------------------------------------------------------------

} // namespace exces

