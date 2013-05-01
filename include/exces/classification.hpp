/**
 *  @file exces/classification.hpp
 *  @brief Entity classification
 *
 *  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_CLASSIFICATION_1304231127_HPP
#define EXCES_CLASSIFICATION_1304231127_HPP

#include <exces/group.hpp>

#include <map>

namespace exces {

template <typename Group>
class manager;

/// Base interface for entity classifications
/**
 *  @note do not use directly, use the derived classes instead.
 */
template <typename Group = default_group>
class any_classification
{
private:
	friend class manager<Group>;
	manager<Group>* _pmanager;

	typedef typename manager<Group>::entity_key entity_key;
	typedef std::size_t update_key;
	update_key _cur_uk;
	
	virtual void insert(entity_key key) = 0;

	virtual update_key begin_update(entity_key key) = 0;

	virtual void finish_update(entity_key ekey, update_key) = 0;

	virtual void remove(entity_key key) = 0;
protected:
	update_key _next_update_key(void)
	{
		return ++_cur_uk;
	}

	manager<Group>& _manager(void) const
	{
		assert(_pmanager);
		return *_pmanager;
	}

	void _register(void)
	{
		assert(_pmanager);
		_pmanager->add_classification(this);
	}

	any_classification(manager<Group>& parent_manager)
	 : _pmanager(&parent_manager)
	 , _cur_uk(0)
	{ }
public:
	any_classification(const any_classification&) = delete;

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

	virtual ~any_classification(void)
	{
		if(_pmanager != nullptr)
		{
			_pmanager->remove_classification(this);
		}
	}
	
};

/// A template for entity classifications
/** A classification stores references to entities managed by a manager
 *  and divides them into disjunct subsets by their 'class' which is a value
 *  assigned to the entities by a 'classifier' function. There is also a second
 *  function called 'filter' that determines which classes are stored and which
 *  are not. If the filter returns true for a class (value) then entities being
 *  of that class are stored in the classification, entities having classes
 *  for which the filter returns false are ignored by the classification.
 *
 *  A classification allows to enumerate entities belonging to a particular
 *  class.
 *
 *  @tparam Class the type that is used to classify entities.
 *  @tparam Group the component group.
 */
template <typename Class, typename Group = default_group>
class classification
 : public any_classification<Group>
{
private:
	std::function<
		Class (manager<Group>&, typename manager<Group>::entity_key)
	> _classify;

	std::function<bool (Class)> _filter;

	typedef std::vector<typename manager<Group>::entity_key>
		_entity_key_vector;

	typedef std::map<Class, _entity_key_vector> _class_map;
	_class_map _classes;

	typedef typename manager<Group>::entity_key entity_key;
	typedef std::size_t update_key;

	typedef std::map<update_key, typename _class_map::iterator> 
		_update_map;
	_update_map _updates;

	void insert(entity_key key, Class entity_class)
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

	void insert(entity_key key)
	{
		Class entity_class = _classify(this->_manager(), key);
		if(_filter(entity_class))
		{
			insert(key, entity_class);
		}
	}

	void remove(entity_key key, Class entity_class)
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

	void remove(entity_key key)
	{
		Class entity_class = _classify(this->_manager(), key);
		if(_filter(entity_class))
		{
			remove(key, entity_class);
		}
	}

	update_key begin_update(entity_key key)
	{
		Class old_class = _classify(this->_manager(), key);

		update_key result = 0;
		if(_filter(old_class))
		{
			typename _class_map::iterator p =
				_classes.find(old_class);

			if(p != _classes.end())
			{
				result = this->_next_update_key();
				_updates[result] = p;
			}
		}
		return result;
	}

	void finish_update(entity_key ekey, update_key ukey)
	{
		Class new_class = _classify(this->_manager(), ekey);

		typename _update_map::iterator u = _updates.find(ukey);

		// if the entity was previously classified
		if(u != _updates.end())
		{
			// and it was classified differently
			if(u->first != new_class)
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
		if(_filter(new_class))
		{
			// insert it into the vector
			// of the new class
			insert(ekey, new_class);
		}
	}

	typedef any_classification<Group> _base;
public:
	/// Constructs a new instance
	/** The @p parent_manager must not be destroyed during the whole
	 *  lifetime of the newly constructed classification. The classifier
	 *  is used to divide entities of the manager into classes and filter
	 *  determines the classes that are stored by the classification.
	 */ 
	classification(
		manager<Group>& parent_manager,
		const std::function<
			Class (manager<Group>&, entity_key)
		>& classifier,
		const std::function<bool (Class)>& filter
	): _base(parent_manager)
	 , _classify(classifier)
	 , _filter(filter)
	{
		this->_register();
	}

	/// Classifications are not copyable
	classification(const classification&) = delete;

	/// Classifications are movable
	classification(classification&& tmp)
	 : _base(static_cast<_base&&>(tmp))
	 , _classify(std::move(_classify))
	 , _filter(std::move(_filter))
	 , _classes(std::move(_classes))
	 , _updates(std::move(_updates))
	{ }

	/// Execute a @p function on each entity in the specified entity_class.
	/**
	 *  @see for_each_mke
	 */
	void for_each(
		Class entity_class,
		const std::function<void(
			manager<Group>&,
			typename manager<Group>::entity_key,
			entity
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
				function(this->_manager(), k, k->first);
				++i;
			}
		}
	}

	/// Execute a @p functor on each entity in the specified entity_class.
	/**
	 *  @see for_each
	 */
	template <typename Functor>
	void for_each_mke(Class entity_class, Functor functor) const
	{
		std::function<void(
			manager<Group>&,
			typename manager<Group>::entity_key,
			entity
		)> wf = [&functor](
			manager<Group>& m,
			typename manager<Group>::entity_key k,
			entity e
		) -> void
		{
			functor(m, k, e);
		};
		for_each(entity_class, wf);
	}
};

} // namespace exces

#endif //include guard

