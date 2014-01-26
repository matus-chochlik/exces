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
add_collection(any_collection<Group>* cl)
{
	assert(cl != nullptr);
	assert(std::find(
		_collections.begin(),
		_collections.end(),
		cl
	) == _collections.end());

	_collections.push_back(cl);

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
move_collection(
	any_collection<Group>* old_cl,
	any_collection<Group>* new_cl
)
{
	assert(old_cl != nullptr);
	assert(new_cl != nullptr);
	assert(std::find(
		_collections.begin(),
		_collections.end(),
		old_cl
	) != _collections.end());
	assert(std::find(
		_collections.begin(),
		_collections.end(),
		new_cl
	) == _collections.end());

	std::replace(
		_collections.begin(),
		_collections.end(),
		old_cl,
		new_cl
	);
}
//------------------------------------------------------------------------------
template <typename Group>
void
manager<Group>::
remove_collection(any_collection<Group>* cl)
{
	auto p = std::find(
		_collections.begin(),
		_collections.end(),
		cl
	);
	assert(p != _collections.end());
	_collections.erase(p);
}
//------------------------------------------------------------------------------
template <class Group>
typename manager<Group>::_class_update_key_list
manager<Group>::
_begin_class_update(
	typename manager<Group>::_entity_info_map::iterator key
)
{
	auto i = _collections.begin();
	auto e = _collections.end();

	std::size_t j = 0;
	_class_update_key_list result(_collections.size());

	while(i != e)
	{
		any_collection<Group>* pc = *i;
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
	assert(_collections.size() == update_keys.size());

	auto i = _collections.begin();
	auto e = _collections.end();

	auto u = update_keys.begin();

	while(i != e)
	{
		any_collection<Group>* pc = *i;
		assert(pc != nullptr);
		pc->finish_update(key, *u);
		++i;
		++u;
	}
}
//------------------------------------------------------------------------------
template <typename Group>
void
manager<Group>::
_do_add_seq(
	typename manager<Group>::entity_key ek,
	const typename manager<Group>::_component_bitset& add_bits,
	const std::function<
		void (typename manager<Group>::_component_adder&)
	>& for_each_seq
)
{
	auto updates = _begin_class_update(ek);

	_component_bitset  old_bits = ek->second._component_bits;

	ek->second._component_bits |= add_bits;

	_component_bitset& new_bits = ek->second._component_bits;

	const std::size_t cc = _component_count();
	_component_key_vector tmp_keys(cc);
	_component_adder adder = { _storage, tmp_keys };
	for_each_seq(adder);
	
	_component_key_vector  new_keys(new_bits.count());
	_component_key_vector& old_keys = ek->second._component_keys;
	assert(old_keys.size() == old_bits.count());

	const typename _component_index_map::index_vector
		&old_map = _component_indices.get(old_bits),
		&new_map = _component_indices.get(new_bits);

	for(std::size_t i=0; i!=cc; ++i)
	{
		if(new_bits.test(i))
		{
			if(old_bits.test(i))
			{
				new_keys[new_map[i]] =
					old_keys[old_map[i]];
			}
			else
			{
				new_keys[new_map[i]] = tmp_keys[i];
			}
		}
		else
		{
			assert(!old_bits.test(i));
		}
	}
	swap(new_keys, old_keys);

	_finish_class_update(ek, updates);
}
//------------------------------------------------------------------------------
template <typename Group>
void
manager<Group>::
_do_rem_seq(
	typename manager<Group>::entity_key ek,
	const typename manager<Group>::_component_bitset& rem_bits,
	const std::function<
		void (typename manager<Group>::_component_remover&)
	>& for_each_seq
)
{
	auto updates = _begin_class_update(ek);

	if((ek->second._component_bits & rem_bits) != rem_bits)
	{
		throw ::std::invalid_argument(
			"exces::entity manager: "
			"removing components that "
			"the entity does not have"
		);
	}

	_component_bitset  old_bits = ek->second._component_bits;

	ek->second._component_bits &= ~rem_bits;

	_component_bitset& new_bits = ek->second._component_bits;
	
	_component_key_vector  new_keys(new_bits.count());
	_component_key_vector& old_keys = ek->second._component_keys;
	assert(old_keys.size() == old_bits.count());

	const typename _component_index_map::index_vector
		&old_map = _component_indices.get(old_bits),
		&new_map = _component_indices.get(new_bits);

	const std::size_t cc = _component_count();
	_component_key_vector tmp_keys(cc);
	for(std::size_t i=0; i!=cc; ++i)
	{
		if(new_bits.test(i))
		{
			if(old_bits.test(i))
			{
				new_keys[new_map[i]] =
					old_keys[old_map[i]];
			}
			else assert(!"Logic error!");
		}
		else
		{
			if(old_bits.test(i))
			{
				tmp_keys[i] = old_keys[old_map[i]];
			}
		}
	}

	_component_remover remover = { _storage, tmp_keys };
	for_each_seq(remover);

	swap(new_keys, old_keys);

	_finish_class_update(ek, updates);
}
//------------------------------------------------------------------------------
template <typename Group>
void
manager<Group>::
_do_rep_seq(
	typename manager<Group>::entity_key ek,
	const typename manager<Group>::_component_bitset& rep_bits,
	const std::function<
		void (typename manager<Group>::_component_replacer&)
	>& for_each_seq
)
{
	auto updates = _begin_class_update(ek);

	if((ek->second._component_bits & rep_bits) != rep_bits)
	{
		throw ::std::invalid_argument(
			"exces::entity manager: "
			"replacing components that "
			"the entity does not have"
		);
	}

	_component_bitset& new_bits = ek->second._component_bits;

	_component_key_vector& new_keys = ek->second._component_keys;

	const typename _component_index_map::index_vector
		&new_map = _component_indices.get(new_bits);

	const std::size_t cc = _component_count();
	_component_key_vector tmp_keys(cc);

	for(std::size_t i=0; i!=cc; ++i)
	{
		if(new_bits.test(i))
		{
			tmp_keys[i] = new_keys[new_map[i]];
		}
	}

	_component_replacer replacer = { _storage, tmp_keys };
	for_each_seq(replacer);

	for(std::size_t i=0; i!=cc; ++i)
	{
		if(new_bits.test(i))
		{
			new_keys[new_map[i]] = tmp_keys[i];
		}
	}


	_finish_class_update(ek, updates);
}
//------------------------------------------------------------------------------
template <typename Group>
void
manager<Group>::
_do_cpy_seq(
	typename manager<Group>::entity_key f,
	typename manager<Group>::entity_key t,
	const typename manager<Group>::_component_bitset& cpy_bits,
	const std::function<
		void(typename manager<Group>::_component_copier&)
	>& for_each_seq
)
{
	auto updates = _begin_class_update(t);

	_entity_info& fei = f->second;
	_entity_info& tei = t->second;

	tei._component_bits |= cpy_bits;
	tei._component_keys.resize(tei._component_bits.count());

	const typename _component_index_map::index_vector &src_map =
		_component_indices.get(fei._component_bits);
	const typename _component_index_map::index_vector &dst_map =
		_component_indices.get(tei._component_bits);

	_component_copier copier = {
		_storage,
		fei._component_keys,
		tei._component_keys,
		src_map,
		dst_map
	};
	for_each_seq(copier);

	_finish_class_update(t, updates);
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

