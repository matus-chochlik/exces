/**
 *  @file exces/manager.inl
 *  @brief Implementation of manager functions
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

namespace exces {

//------------------------------------------------------------------------------
// manager
//------------------------------------------------------------------------------
template <typename Group>
typename manager<Group>::_entity_info_map::iterator
manager<Group>::
_find_entity(typename entity<Group>::type e)
{
	typename _entity_info_map::iterator p = _entities.find(e);
	
	if(p == _entities.end())
	{
		throw ::std::invalid_argument(
			"exces::entity manager: "
			"requested entity not found"
		);
	}
	return p;
}
//------------------------------------------------------------------------------
template <typename Group>
void
manager<Group>::
add_classification(any_classification<Group>* cl)
{
	assert(cl != nullptr);
	assert(std::find(
		_classifications.begin(),
		_classifications.end(),
		cl
	) == _classifications.end());

	_classifications.push_back(cl);

	typename _entity_info_map::iterator
		i = _entities.begin(),
		e = _entities.end();

	while(i != e)
	{
		cl->insert(i);
		++i;
	}
}
//------------------------------------------------------------------------------
template <typename Group>
void
manager<Group>::
move_classification(
	any_classification<Group>* old_cl,
	any_classification<Group>* new_cl
)
{
	assert(old_cl != nullptr);
	assert(new_cl != nullptr);
	assert(std::find(
		_classifications.begin(),
		_classifications.end(),
		old_cl
	) != _classifications.end());
	assert(std::find(
		_classifications.begin(),
		_classifications.end(),
		new_cl
	) == _classifications.end());

	std::replace(
		_classifications.begin(),
		_classifications.end(),
		old_cl,
		new_cl
	);
}
//------------------------------------------------------------------------------
template <typename Group>
void
manager<Group>::
remove_classification(any_classification<Group>* cl)
{
	auto p = std::find(
		_classifications.begin(),
		_classifications.end(),
		cl
	);
	assert(p != _classifications.end());
	_classifications.erase(p);
}
//------------------------------------------------------------------------------
template <class Group>
typename manager<Group>::_class_update_key_list
manager<Group>::
_begin_class_update(
	typename manager<Group>::_entity_info_map::iterator key
)
{
	auto i = _classifications.begin();
	auto e = _classifications.end();

	std::size_t j = 0;
	_class_update_key_list result(_classifications.size());

	while(i != e)
	{
		any_classification<Group>* pc = *i;
		assert(pc != nullptr);
		result[j] = pc->begin_update(key);
		++i;
		++j;
	}

	return std::move(result);
}
//------------------------------------------------------------------------------
template <typename Group>
void
manager<Group>::
_finish_class_update(
	typename manager<Group>::_entity_info_map::iterator key,
	const typename manager<Group>::_class_update_key_list& update_keys
)
{
	assert(_classifications.size() == update_keys.size());

	auto i = _classifications.begin();
	auto e = _classifications.end();

	auto u = update_keys.begin();

	while(i != e)
	{
		any_classification<Group>* pc = *i;
		assert(pc != nullptr);
		pc->finish_update(key, *u);
		++i;
		++u;
	}
}
//------------------------------------------------------------------------------
template <typename Group>
manager<Group>&
manager<Group>::
for_each(
	const std::function<void (
		manager<Group>& m,
		typename manager<Group>::entity_key k
	)>& function
)
{
	typename _entity_info_map::iterator
		i = _entities.begin(),
		e = _entities.end();

	while(i != e)
	{
		function(*this, i);
		++i;
	}

	return *this;
}
//------------------------------------------------------------------------------
// forced instantiation
//------------------------------------------------------------------------------
template <typename Group>
void
manager<Group>::
_instantiate(void)
{
	aux_::component_bitset<Group> cb;
	aux_::component_index_map<Group> cim;
	const aux_::component_index_map<Group>& ccim = cim;
	cim.get(cb);
	ccim.get(cb);
	
	typename entity<Group>::type e;
	manager<Group> m;
	m.add(e);
	m._find_entity(e);
}
//------------------------------------------------------------------------------

} // namespace exces

