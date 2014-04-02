/**
 *  @file exces/collection.inl
 *  @brief Implements the collection and classification functions
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

namespace exces {
//------------------------------------------------------------------------------
// collection_intf
//------------------------------------------------------------------------------
template <typename Group>
void
collection_intf<Group>::
_register(void)
{
	assert(_pmanager);
	_pmanager->add_collection(this);
}
//------------------------------------------------------------------------------
template <typename Group>
collection_intf<Group>::
collection_intf(collection_intf&& tmp)
 : _pmanager(tmp.pmanager)
 , _cur_uk(tmp._cur_uk)
{
	if(_pmanager)
	{
		_pmanager->move_collection(&tmp, this);
		tmp._pmanager = nullptr;
	}
}
//------------------------------------------------------------------------------
template <typename Group>
collection_intf<Group>::
~collection_intf(void)
{
	if(_pmanager != nullptr)
	{
		_pmanager->remove_collection(this);
	}
}
//------------------------------------------------------------------------------
template <typename Group>
inline
typename collection_intf<Group>::update_key
collection_intf<Group>::
_next_update_key(void)
{
	if(_cur_uk == 0) ++_cur_uk;
	return _cur_uk++;
}
//------------------------------------------------------------------------------
template <typename Group>
inline
manager<Group>&
collection_intf<Group>::
_manager(void) const
{
	assert(_pmanager);
	return *_pmanager;
}
//------------------------------------------------------------------------------
// collection
//------------------------------------------------------------------------------
template <typename Group>
void
collection<Group>::
insert(entity_key key)
{
	if(!_filter_entity || _filter_entity(this->_manager(), key))
	{
		assert(!_entities.contains(key));
		_entities.insert(key);
	}
}
//------------------------------------------------------------------------------
template <typename Group>
void
collection<Group>::
remove(entity_key key)
{
	assert(_entities.contains(key));
	_entities.erase(key);
}
//------------------------------------------------------------------------------
template <typename Group>
typename collection<Group>::update_key
collection<Group>::
begin_update(entity_key)
{
	return 0;
}
//------------------------------------------------------------------------------
template <typename Group>
void
collection<Group>::
finish_update(entity_key ekey, update_key)
{
	if(!_filter_entity || _filter_entity(this->_manager(), ekey))
	{
		_entities.insert(ekey);
	}
	else
	{
		_entities.erase(ekey);
	}
}
//------------------------------------------------------------------------------
template <typename Group>
void
collection<Group>::
for_each(
	const std::function<bool(
		const iter_info&,
		manager<Group>&,
		typename manager<Group>::entity_key
	)>& function
) const
{
	typename _entity_key_set::const_iterator
		i = _entities.begin(),
		e = _entities.end();

	iter_info ii(_entities.size());

	while(i != e)
	{
		auto k = *i;
		if(!function(ii, this->_manager(), k))
			break;
		++i;
		ii.step();
	}
}
//------------------------------------------------------------------------------
template <typename Group>
inline
void
collection<Group>::
_instantiate(void)
{
	manager<Group> m;
	std::function<bool (
		manager<Group>&,
		typename manager<Group>::entity_key
	)> fi;
	collection<Group> c(m, fi);
}
//------------------------------------------------------------------------------
// classification
//------------------------------------------------------------------------------
template <typename Class, typename Group>
void
classification<Class, Group>::
insert(entity_key key, Class entity_class)
{
	typename _class_map::iterator p =
		_classes.find(entity_class);

	if(p == _classes.end())
	{
		_classes.insert(
			typename _class_map::value_type(
				entity_class,
				_entity_key_set(key)
			)
		);
	}
	else
	{
		p->second.insert(key);
	}
}
//------------------------------------------------------------------------------
template <typename Class, typename Group>
void
classification<Class, Group>::
insert(entity_key key)
{
	if(!_filter_entity || _filter_entity(this->_manager(), key))
	{
		Class entity_class = _classify(this->_manager(), key);
		if(!_filter_class || _filter_class(entity_class))
		{
			insert(key, entity_class);
		}
	}
}
//------------------------------------------------------------------------------
template <typename Class, typename Group>
void
classification<Class, Group>::
remove(entity_key key, Class entity_class)
{
	// find its class
	typename _class_map::iterator cp =
		_classes.find(entity_class);
	
	assert(cp != _classes.end());
	cp->second.erase(key);
}
//------------------------------------------------------------------------------
template <typename Class, typename Group>
void
classification<Class, Group>::
remove(entity_key key)
{
	if(!_filter_entity || _filter_entity(this->_manager(), key))
	{
		Class entity_class = _classify(this->_manager(), key);
		if(!_filter_class || _filter_class(entity_class))
		{
			remove(key, entity_class);
		}
	}
}
//------------------------------------------------------------------------------
template <typename Class, typename Group>
typename classification<Class, Group>::update_key
classification<Class, Group>::
begin_update(entity_key key)
{
	update_key result = 0;
	if(!_filter_entity || _filter_entity(this->_manager(), key))
	{
		Class old_class = _classify(this->_manager(), key);

		if(!_filter_class || _filter_class(old_class))
		{
			typename _class_map::iterator p =
				_classes.find(old_class);

			if(p != _classes.end())
			{
				result = this->_next_update_key();
				_updates[result] = p;
			}
		}
	}
	return result;
}
//------------------------------------------------------------------------------
template <typename Class, typename Group>
void
classification<Class, Group>::
finish_update(entity_key ekey, update_key ukey)
{
	typename _update_map::iterator u = _updates.find(ukey);

	if(!_filter_entity || _filter_entity(this->_manager(), ekey))
	{
		Class new_class = _classify(this->_manager(), ekey);

		// if the entity was previously classified
		if(u != _updates.end())
		{
			// and it was classified differently
			if(u->second->first != new_class)
			{
				// erase it from the vector
				// in the old class
				u->second->second.erase(ekey);
			}
			// if its previous class was the same
			// no need to reclassify
			else return;
		}
		if(!_filter_class || _filter_class(new_class))
		{
			// insert it into the vector
			// of the new class
			insert(ekey, new_class);
		}
	}
	else
	{
		// if the entity was previously classified
		if(u != _updates.end())
		{
			// erase it from the vector
			// in the old class
			u->second->second.erase(ekey);
		}
	}
}
//------------------------------------------------------------------------------
template <typename Class, typename Group>
std::size_t
classification<Class, Group>::
class_count(void) const
{
	return _classes.size();
}
//------------------------------------------------------------------------------
template <typename Class, typename Group>
std::size_t
classification<Class, Group>::
cardinality(const Class& entity_class) const
{
	auto p = _classes.find(entity_class);
	if(p == _classes.end()) return 0;
	return p->second.size();
}
//------------------------------------------------------------------------------
template <typename Class, typename Group>
void
classification<Class, Group>::
for_each(
	const Class& entity_class,
	const std::function<bool(
		const iter_info&,
		manager<Group>&,
		typename manager<Group>::entity_key
	)>& function
) const
{
	typename _class_map::const_iterator p =
		_classes.find(entity_class);
	if(p != _classes.end())
	{
		typename _entity_key_set::const_iterator
			i = p->second.begin(),
			e = p->second.end();

		iter_info ii(p->second.size());

		while(i != e)
		{
			auto k = *i;
			if(!function(ii, this->_manager(), k))
				break;
			++i;
			ii.step();
		}
	}
}
//------------------------------------------------------------------------------
// forced instantiation
//------------------------------------------------------------------------------
template <typename Class, typename Group>
void
classification<Class, Group>::
_instantiate(void)
{
	manager<Group> m;
	std::function<bool (
		manager<Group>&,
		typename manager<Group>::entity_key
	)> fi;
	std::function<Class (
		manager<Group>&,
		typename manager<Group>::entity_key
	)> cl;
	std::function<bool (
		const iter_info&,
		manager<Group>&,
		typename manager<Group>::entity_key
	)> fe;
	classification<Class, Group> c(m, fi, cl);
	c.cardinality(Class());
	c.for_each(Class(), fe);
}
//------------------------------------------------------------------------------
} // namespace exces
