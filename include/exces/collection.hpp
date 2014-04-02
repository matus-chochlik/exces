/**
 *  @file exces/collection.hpp
 *  @brief Entity collections and classifications
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_COLLECTION_1304231127_HPP
#define EXCES_COLLECTION_1304231127_HPP

#include <exces/group.hpp>
#include <exces/entity_range.hpp>
#include <exces/entity_key_set.hpp>
#include <exces/entity_filters.hpp>
#include <exces/func_adaptors.hpp>
#include <exces/iter_info.hpp>

#include <map>
#include <functional>

namespace exces {

template <typename Group>
class manager;

/// Implementation of basic entity collection traversal functions
template <typename Group>
class collect_entity_range
{
private:
	typedef typename manager<Group>::entity_key entity_key;
	typedef typename std::vector<entity_key>::const_iterator _iter;
	_iter _i;
	const _iter _e;
public:
	collect_entity_range(_iter i, _iter e)
	 : _i(i)
	 , _e(e)
	{ }

	/// Indicates that the range is empty (the traversal is done)
	bool empty(void) const
	{
		return _i == _e;
	}

	/// Returns the current front element of the range
	entity_key front(void) const
	{
		assert(!empty());
		return *_i;
	}

	/// Moves the front of the range one element ahead
	void next(void)
	{
		assert(!empty());
		++_i;
	}
};

/// Base interface for entity collections and classifications
/**
 *  @note do not use directly, use the derived classes instead.
 */
template <typename Group>
class collection_intf
{
private:
	friend class manager<Group>;
	manager<Group>* _pmanager;

	typedef typename manager<Group>::entity_key entity_key;
	typedef std::size_t update_key;
	update_key _cur_uk;
	
	virtual void insert(entity_key key) = 0;
	virtual void remove(entity_key key) = 0;
	virtual update_key begin_update(entity_key key) = 0;
	virtual void finish_update(entity_key ekey, update_key) = 0;
protected:
	update_key _next_update_key(void);

	manager<Group>& _manager(void) const;

	void _register(void);

	collection_intf(manager<Group>& parent_manager)
	 : _pmanager(&parent_manager)
	 , _cur_uk(0)
	{ }
public:
	collection_intf(const collection_intf&) = delete;

	collection_intf(collection_intf&& tmp);

	virtual ~collection_intf(void);
};

/// Entity collection
/** Entity collection stores references to a subset of entities managed by
 *  a manager, satisfying some predicate. For example all entities having
 *  a specified set of components, etc.
 *
 *  @see classification
 */
template <typename Group = default_group>
class collection
 : public collection_intf<Group>
{
private:
	typedef collection_intf<Group> _base;

	std::function<
		bool (
			manager<Group>&,
			typename manager<Group>::entity_key
		)
	> _filter_entity;

	typedef entity_key_set<Group> _entity_key_set;
	_entity_key_set _entities;

	typedef typename manager<Group>::entity_key entity_key;
	typedef std::size_t update_key;

	void insert(entity_key key);
	void remove(entity_key key);
	update_key begin_update(entity_key key);
	void finish_update(entity_key ekey, update_key);

	template <typename Component, typename MemFnRV>
	struct _call_comp_mem_fn
	{
		MemFnRV (Component::*_mem_fn_ptr)(void) const;

		_call_comp_mem_fn(
			MemFnRV (Component::*mem_fn_ptr)(void) const
		): _mem_fn_ptr(mem_fn_ptr)
		{ }

		bool operator ()(
			manager<Group>& mgr,
			typename manager<Group>::entity_key ekey
		)
		{
			assert(mgr.template has<Component>(ekey));
			return bool(
				(mgr.template rw<Component>(ekey)
					.*_mem_fn_ptr)()
			);
		}
	};
public:
	static void _instantiate(void);

	/// Constructs a new collection
	/** The @p parent_manager must not be destroyed during the whole
	 *  lifetime of the newly constructed collection.
	 */ 
	collection(
		manager<Group>& parent_manager,
		const std::function<
			bool (manager<Group>&, entity_key)
		>& entity_filter
	): _base(parent_manager)
	 , _filter_entity(entity_filter)
	{
		this->_register();
	}

	template <typename Component, typename MemFnRV>
	collection(
		manager<Group>& parent_manager,
		MemFnRV (Component::*mem_fn_ptr)(void) const
	): _base(parent_manager)
	 , _filter_entity(_call_comp_mem_fn<Component, MemFnRV>(mem_fn_ptr))
	{
		this->_register();
	}

	/// Collections are non-copyable
	collection(const collection&) = delete;

	/// Collections are movable
	collection(collection&& tmp)
	 : _base(static_cast<_base&&>(tmp))
	 , _filter_entity(std::move(tmp._filter_entity))
	 , _entities(std::move(tmp._entities))
	{ }

	/// Execute a @p function on each entity in the collection.
	void for_each(
		const std::function<bool(
			const iter_info&,
			manager<Group>&,
			typename manager<Group>::entity_key
		)>& function
	) const;
};

/// A template for entity classifications
/** A classification stores references to entities managed by a manager
 *  and divides them into disjunct subsets by their 'class' which is a value
 *  assigned to the entities by a 'classifier' function. The second function
 *  called 'class filter' determines which classes are stored and which are not. 
 *  There is also a third function called the entity filter that determines
 *  if an entity should be even considered for classification.
 *  If the entity filter is present and returns true then the entity is
 *  classified. Then if the class filter returns true for a class (value)
 *  then entities being of that class are stored in the classification,
 *  entities having classes for which the class filter returns false are
 *  not stored by the classification.
 *
 *  A classification allows to enumerate entities belonging to a particular
 *  class.
 *
 *  @see collection
 *
 *  @tparam Class the type that is used to classify entities.
 *  @tparam Group the component group.
 */
template <typename Class, typename Group = default_group>
class classification
 : public collection_intf<Group>
{
private:
	typedef collection_intf<Group> _base;

	std::function<
		bool (
			manager<Group>&,
			typename manager<Group>::entity_key
		)
	> _filter_entity;

	std::function<
		Class (
			manager<Group>&,
			typename manager<Group>::entity_key
		)
	> _classify;

	std::function<bool (Class)> _filter_class;

	typedef entity_key_set<Group> _entity_key_set;

	typedef std::map<Class, _entity_key_set> _class_map;
	_class_map _classes;

	typedef typename manager<Group>::entity_key entity_key;
	typedef std::size_t update_key;

	typedef std::map<update_key, typename _class_map::iterator> 
		_update_map;
	_update_map _updates;

	void insert(entity_key key, Class entity_class);
	void insert(entity_key key);

	void remove(entity_key key, Class entity_class);
	void remove(entity_key key);

	update_key begin_update(entity_key key);
	void finish_update(entity_key ekey, update_key ukey);

	template <typename Component>
	static bool _has_component(
		manager<Group>& mgr,
		typename manager<Group>::entity_key ekey
	)
	{
		return mgr.template has<Component>(ekey);
	}

	template <typename Component, typename MemVarType>
	struct _get_comp_mem_var
	{
		MemVarType Component::* _mem_var_ptr;

		_get_comp_mem_var(
			MemVarType Component::* mem_var_ptr
		): _mem_var_ptr(mem_var_ptr)
		{ }

		Class operator ()(
			manager<Group>& mgr,
			typename manager<Group>::entity_key ekey
		)
		{
			assert(mgr.template has<Component>(ekey));
			return Class(
				mgr.template rw<Component>(ekey)
					.*_mem_var_ptr
			);
		}
	};

	template <typename Component, typename MemFnRV>
	struct _call_comp_mem_fn
	{
		MemFnRV (Component::*_mem_fn_ptr)(void) const;

		_call_comp_mem_fn(
			MemFnRV (Component::*mem_fn_ptr)(void) const
		): _mem_fn_ptr(mem_fn_ptr)
		{ }

		Class operator ()(
			manager<Group>& mgr,
			typename manager<Group>::entity_key ekey
		)
		{
			assert(mgr.template has<Component>(ekey));
			return Class(
				(mgr.template rw<Component>(ekey)
					.*_mem_fn_ptr)()
			);
		}
	};
	
public:
	static void _instantiate(void);

	/// Constructs a new classification
	/** The @p parent_manager must not be destroyed during the whole
	 *  lifetime of the newly constructed classification. The classifier
	 *  is used to divide entities of the manager into classes there is
	 *  no class filter so all classes are stored.
	 */ 
	classification(
		manager<Group>& parent_manager,
		const std::function<
			bool (manager<Group>&, entity_key)
		>& entity_filter,
		const std::function<
			Class (manager<Group>&, entity_key)
		>& classifier
	): _base(parent_manager)
	 , _filter_entity(entity_filter)
	 , _classify(classifier)
	 , _filter_class()
	{
		this->_register();
	}

	/// Constructs a new classification
	/** The @p parent_manager must not be destroyed during the whole
	 *  lifetime of the newly constructed classification. The classifier
	 *  is used to divide entities of the manager into classes and filter
	 *  determines the classes that are stored by the classification.
	 */ 
	classification(
		manager<Group>& parent_manager,
		const std::function<
			bool (manager<Group>&, entity_key)
		>& entity_filter,
		const std::function<
			Class (manager<Group>&, entity_key)
		>& classifier,
		const std::function<bool (Class)>& class_filter
	): _base(parent_manager)
	 , _filter_entity(entity_filter)
	 , _classify(classifier)
	 , _filter_class(class_filter)
	{
		this->_register();
	}

	template <typename Component, typename MemVarType>
	classification(
		manager<Group>& parent_manager,
		MemVarType Component::* mem_var_ptr
	): _base(parent_manager)
	 , _filter_entity(&_has_component<Component>)
	 , _classify(_get_comp_mem_var<Component, MemVarType>(mem_var_ptr))
	 , _filter_class()
	{
		this->_register();
	}

	template <typename Component, typename MemVarType>
	classification(
		manager<Group>& parent_manager,
		MemVarType Component::* mem_var_ptr,
		const std::function<bool (Class)>& class_filter
	): _base(parent_manager)
	 , _filter_entity(&_has_component<Component>)
	 , _classify(_get_comp_mem_var<Component, MemVarType>(mem_var_ptr))
	 , _filter_class(class_filter)
	{
		this->_register();
	}

	template <typename Component, typename MemFnRV>
	classification(
		manager<Group>& parent_manager,
		MemFnRV (Component::*mem_fn_ptr)(void) const
	): _base(parent_manager)
	 , _filter_entity(&_has_component<Component>)
	 , _classify(_call_comp_mem_fn<Component, MemFnRV>(mem_fn_ptr))
	 , _filter_class()
	{
		this->_register();
	}

	template <typename Component, typename MemFnRV>
	classification(
		manager<Group>& parent_manager,
		MemFnRV (Component::*mem_fn_ptr)(void) const,
		const std::function<bool (Class)>& class_filter
	): _base(parent_manager)
	 , _filter_entity(&_has_component<Component>)
	 , _classify(_call_comp_mem_fn<Component, MemFnRV>(mem_fn_ptr))
	 , _filter_class(class_filter)
	{
		this->_register();
	}

	/// Classifications are not copyable
	classification(const classification&) = delete;

	/// Classifications are movable
	classification(classification&& tmp)
	 : _base(static_cast<_base&&>(tmp))
	 , _filter_entity(std::move(tmp._filter_entity))
	 , _classify(std::move(tmp._classify))
	 , _filter_class(std::move(tmp._filter_class))
	 , _classes(std::move(tmp._classes))
	 , _updates(std::move(tmp._updates))
	{ }

	/// Returns the number of different classes
	std::size_t class_count(void) const;

	/// Returns the number of entities of the specified class
	std::size_t cardinality(const Class& entity_class) const;

	/// Execute a @p function on each entity in the specified entity_class.
	void for_each(
		const Class& entity_class,
		const std::function<bool(
			const iter_info&,
			manager<Group>&,
			typename manager<Group>::entity_key
		)>& function
	) const;
};

} // namespace exces

#endif //include guard

