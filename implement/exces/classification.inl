/**
 *  @file exces/classification.inl
 *  @brief Implements the classification functions
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

namespace exces {
//------------------------------------------------------------------------------
// any_classification
//------------------------------------------------------------------------------
template <typename Group>
void
any_classification<Group>::
_register(void)
{
	assert(_pmanager);
	_pmanager->add_classification(this);
}
//------------------------------------------------------------------------------
template <typename Group>
any_classification<Group>::
any_classification(any_classification&& tmp)
 : _pmanager(tmp.pmanager)
 , _cur_uk(tmp._cur_uk)
{
	if(_pmanager)
	{
		_pmanager->move_classification(&tmp, this);
		tmp._pmanager = nullptr;
	}
}
//------------------------------------------------------------------------------
template <typename Group>
any_classification<Group>::
~any_classification(void)
{
	if(_pmanager != nullptr)
	{
		_pmanager->remove_classification(this);
	}
}
//------------------------------------------------------------------------------
template <typename Group>
inline
typename any_classification<Group>::update_key
any_classification<Group>::
_next_update_key(void)
{
	if(_cur_uk == 0) ++_cur_uk;
	return _cur_uk++;
}
//------------------------------------------------------------------------------
template <typename Group>
inline
manager<Group>&
any_classification<Group>::
_manager(void) const
{
	assert(_pmanager);
	return *_pmanager;
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
				_entity_key_vector(1, key)
			)
		);
	}
	else
	{
		assert(std::find(
			p->second.begin(),
			p->second.end(),
			key
		) == p->second.end());

		p->second.push_back(key);
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
	// find its position
	typename _entity_key_vector::iterator ep =
		std::find(
			cp->second.begin(),
			cp->second.end(),
			key
		);
	assert(ep != cp->second.end());
	// erase it from the vector
	// in its class
	cp->second.erase(ep);
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
				// find its position
				typename _entity_key_vector::iterator ep =
					std::find(
						u->second->second.begin(),
						u->second->second.end(),
						ekey
					);
				assert(ep != u->second->second.end());
				// erase it from the vector
				// in the old class
				u->second->second.erase(ep);
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
			// find its position
			typename _entity_key_vector::iterator ep =
				std::find(
					u->second->second.begin(),
					u->second->second.end(),
					ekey
				);
			assert(ep != u->second->second.end());
			// erase it from the vector
			// in the old class
			u->second->second.erase(ep);
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
	const std::function<void(
		manager<Group>&,
		typename manager<Group>::entity_key
	)>& function
) const
{
	typename _class_map::const_iterator p =
		_classes.find(entity_class);
	if(p != _classes.end())
	{
		typename _entity_key_vector::const_iterator
			i = p->second.begin(),
			e = p->second.end();

		while(i != e)
		{
			auto k = *i;
			function(this->_manager(), k);
			++i;
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
	std::function<void (
		manager<Group>&,
		typename manager<Group>::entity_key
	)> fe;
	classification<Class, Group> c(m, fi, cl);
	c.cardinality(Class());
	c.for_each(Class(), fe);
}
//------------------------------------------------------------------------------
} // namespace exces
