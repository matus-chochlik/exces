/**
 *  @file exces/manager.hpp
 *  @brief Implements component manager
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_MANAGER_1212101457_HPP
#define EXCES_MANAGER_1212101457_HPP

#include <exces/entity.hpp>
#include <exces/entity_range.hpp>
#include <exces/entity_filters.hpp>
#include <exces/storage.hpp>
#include <exces/component.hpp>
#include <exces/collection.hpp>

#include <array>
#include <vector>
#include <map>
#include <cassert>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <iterator>

namespace exces {

template <typename Group>
class manager;

/// Implementation of basic manager entity traversal functions
template <typename Group>
class manager_entity_range
{
private:
	typedef typename manager<Group>::entity_key entity_key;
	entity_key _i;
	const entity_key _e;
public:
	manager_entity_range(entity_key i, entity_key e)
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
		return _i;
	}

	/// Moves the front of the range one element ahead
	void next(void)
	{
		assert(!empty());
		++_i;
	}
};

/// Manages the components of entities
template <typename Group = default_group>
class manager
{
private:
	template <typename C>
	struct _fix1
	 : std::remove_cv<typename std::remove_reference<C>::type>
	{ };

	// metafunction removing type decorations from component typenames
	template <typename ... C>
	struct _fixn
	 : mp::typelist<typename _fix1<C>::type...>
	{ };

	template <typename TL>
	struct _fixl
	{
		template <typename ... C>
		mp::typelist<typename _fix1<C>::type...>
		static _do_fix(mp::typelist<C...>);

		typedef decltype(_do_fix(TL())) type;
	};

	template <typename ... C>
	struct _sortn
	 : aux_::sort_components<Group, mp::typelist<C...>>
	{ };

	// component storage
	typedef component_storage<Group> _component_storage;
	_component_storage _storage;

	// the count of components in the Group
	typedef mp::size<components<Group> > _component_count;

	// type of the bitset used to indicate which components
	// an entity has
	typedef aux_::component_bitset<Group> _component_bitset;


	// converts static group-unique component-ids for a bitset with
	// a particular combination of bits set into a vector of indices
	// to a vector-of-keys pointing to the individual components
	// (ordered by their ids) in the storage.
	typedef aux_::component_index_map<Group> _component_index_map;

	_component_index_map _component_indices;

	// a vector of keys that allow to access the components
	// (ordered by their ids) in the storage
	struct _component_key_vector
	 : std::vector<typename _component_storage::component_key>
	{
		typedef typename _component_storage::component_key
			component_key;

		_component_key_vector(void)
		{ }

		_component_key_vector(std::size_t n)
		 : std::vector<component_key>(n, _component_storage::null_key())
		{ }
	};

	// helper functor that reserves storage space for a Component
	struct _component_reserver
	{
		_component_storage& _storage;
		const std::size_t n;

		template <typename Component>
		void operator()(mp::identity<Component>) const
		{
			_storage.template reserve<Component>(n);
		}
	};

	// helper functor that sets the bit in a bitset for a particular
	// Component to the specified Value
	template <bool Value>
	struct _component_bit_setter
	{
		_component_bitset& _component_bits;

		template <typename Component>
		void operator()(mp::identity<Component>) const
		{
			_component_bits.set(	
				component_id<Component, Group>::value,
				Value
			);
		}
	};

	// generates the bitset for the specified components
	template <typename Sequence>
	static _component_bitset _gen_bits_seq(const Sequence&)
	{
		_component_bitset _bits;
		_component_bit_setter<true> bset = { _bits };
		mp::for_each<Sequence>(bset);
		return std::move(_bits);
	}

	// gets the bitset for the specified components
	template <typename ... Components>
	static const _component_bitset& _get_bits_tl(
		mp::typelist<Components...> seq
	)
	{
		static _component_bitset _bits = _gen_bits_seq(seq);
		return _bits;
	}

	// gets the bitset for the specified components
	template <typename Sequence>
	static const _component_bitset& _get_bits(const Sequence&)
	{
		typename aux_::sort_components<
			Group,
			typename _fixl<
				typename mp::as_typelist<Sequence>::type
			>::type
		>::type tl;
		return _get_bits_tl(tl);
	}

	template <typename ... Components>
	static const _component_bitset& _get_bits(void)
	{
		typename aux_::sort_components<
			Group,
			typename _fixn<Components...>::type
		>::type tl;
		return _get_bits_tl(tl);
	}

	// information about a single entity
	struct _entity_info
	{
		_component_bitset _component_bits;

		_component_key_vector _component_keys;
	};

	// a map that stores the information about entities
	// ordered by the id of the entity
	typedef std::map<
		typename entity<Group>::type,
		_entity_info
	> _entity_info_map;
	typedef typename _entity_info_map::value_type _entity_info_entry;
	_entity_info_map _entities;

	// helper functor that adds a Component into the storage
	// and remembers the key in a vector at the position specified
	// by the Component's id
	struct _component_adder
	{
		_component_storage& _storage;
		_component_key_vector& _keys;

		template <typename Component>
		void operator()(Component& component) const
		{
			const std::size_t cid =
				component_id<Component, Group>::value;
			_keys[cid] = _storage.store(std::move(component));
		}
	};

	void _do_add_seq(
		typename _entity_info_map::iterator ek,
		const _component_bitset& add_bits,
		const std::function<void (_component_adder& adder)>&
	);

	// helper functor that removes a Component from the storage
	struct _component_remover
	{
		_component_storage& _storage;
		_component_key_vector& _keys;

		template <typename Component>
		void operator()(mp::identity<Component>) const
		{
			const std::size_t cid =
				component_id<Component, Group>::value;
			_storage.template release<Component>(_keys[cid]);
		}
	};

	void _do_rem_seq(
		typename _entity_info_map::iterator ek,
		const _component_bitset& rem_bits,
		const std::function<void(_component_remover&)>&
	);

	// helper functor that replaces a Component in the storage
	struct _component_replacer
	{
		_component_storage& _storage;
		_component_key_vector& _keys;

		template <typename Component>
		void operator()(Component& component) const
		{
			const std::size_t cid =
				component_id<Component, Group>::value;
			_keys[cid] = _storage.template replace<Component>(
				_keys[cid],
				std::move(component)
			);
		}
	};

	void _do_rep_seq(
		typename _entity_info_map::iterator ek,
		const _component_bitset& rep_bits,
		const std::function<void(_component_replacer&)>&
	);

	// helper functor that copies components between entities
	struct _component_copier
	{
		_component_storage& _storage;
		_component_key_vector& _src_keys;
		_component_key_vector& _dst_keys;
		const typename _component_index_map::index_vector& _src_map;
		const typename _component_index_map::index_vector& _dst_map;

		template <typename Component>
		void operator()(mp::identity<Component>) const
		{
			const std::size_t cid =
				component_id<Component, Group>::value;
			_dst_keys[_dst_map[cid]] =
				_storage.template copy<Component>(
					_src_keys[_src_map[cid]]
				);
		}
	};

	void _do_cpy_seq(
		typename _entity_info_map::iterator f,
		typename _entity_info_map::iterator t,
		const _component_bitset& cpy_bits,
		const std::function<void(_component_copier&)>&
	);

	// gets the information about an entity, inserts a new one
	// if the entity is not registered yet
	typename _entity_info_map::iterator
	_get_entity(typename entity<Group>::type e)
	{
		typename _entity_info_map::iterator p = _entities.insert(
			_entity_info_entry(e, _entity_info())
		).first;
		assert(p != _entities.end());
		return p;
	}

	// gets the information about an entity, throws if the entity
	// is not registered
	typename _entity_info_map::iterator
	_find_entity(typename entity<Group>::type e);

	std::vector<any_collection<Group>*> _collections;
	friend class any_collection<Group>;

	void add_collection(any_collection<Group>* cl);
	void remove_collection(any_collection<Group>* cl);
	void move_collection(
		any_collection<Group>* old_cl,
		any_collection<Group>* new_cl
	);

	typedef std::vector<std::size_t> _collection_update_key_list;

	_collection_update_key_list _begin_collection_update(
		typename _entity_info_map::iterator key
	);
	void _finish_collection_update(
		typename _entity_info_map::iterator key,
		const _collection_update_key_list& update_keys
	);
public:
	// implementation detail DO NOT use directly
	_component_storage& _get_storage_ref(void)
	{
		return _storage;
	}

	static void _instantiate(void);

	/// The type of entity used by this manager
	typedef typename entity<Group>::type entity_type;

	/// Key for O(1) access to entity data
	typedef typename _entity_info_map::iterator entity_key;

	/// Returns a vector of keys for O(1) access to entities
	/**
	 *  The returned keys are invalidated by removal of the entity
	 *  from the manager.
	 *
	 *  @see get_key
	 */
	template <typename Iterator>
	std::vector<entity_key> get_keys(Iterator cur, Iterator end)
	{
		std::size_t n = distance(cur, end);
		std::vector<entity_key> result(n);
		_entity_info init;
		for(std::size_t i=0; i!=n; ++i)
		{
			assert(cur != end);
			result[i] = _entities.insert(
				_entity_info_entry(*cur, init)
			).first;
			++cur;
		}
		return std::move(result);
	}

	/// Returns a vector of keys for O(1) access to entities
	/**
	 *  The returned keys are invalidated by removal of the entity
	 *  from the manager.
	 *
	 *  @see get_key
	 */
	std::vector<entity_key>
	get_keys(const std::vector<entity_type>& es)
	{
		return get_keys(es.begin(), es.end());
	}

	/// Get a key for O(1) access to the specified entity
	/**
	 *  The returned keys are invalidated by removal of the entity
	 *  from the manager.
	 *
	 *  @see get_keys
	 *  @see get_entity
	 */
	entity_key get_key(entity_type e)
	{
		return _get_entity(e);
	}

	/// Get the entity pointed to by key
	/**
	 *  @see get_key
	 */
	entity_type get_entity(entity_key k)
	{
		return k->first;
	}

	/// Reserves space for n instances of Components in Seqence
	/**
	 *  @see reserve
	 */
	template <typename Sequence>
	manager& reserve_seq(std::size_t n, Sequence seq = Sequence())
	{
		_component_reserver reserver = { _storage, n };
		mp::for_each<Sequence>(reserver);
		return *this;
	}

	/// Reserves storage space 
	/**
	 *  @see reserve_seq
	 */
	template <typename ... Components>
	manager& reserve(std::size_t n)
	{
		return reserve_seq(n, typename _fixn<Components...>::type());
	}

	/// Adds the specified components to the specified entity
	/**
	 *  @see add
	 *  @pre !has_some_seq< Sequence >(ek)
	 *  @post has_all_seq< Sequence >(ek)
	 */
	template <typename Sequence>
	manager& add_seq(entity_key ek, Sequence seq)
	{
		_do_add_seq(
			ek,
			_get_bits(seq),
			[&seq](_component_adder& adder)
			{
				mp::for_each(seq, adder);
			}
		);
		return *this;
	}

	/// Adds the specified components to the specified entity
	/**
	 *  @see add
	 *  @pre !has_some_seq< Sequence >(e)
	 *  @post has_all_seq< Sequence >(e)
	 */
	template <typename Sequence>
	manager& add_seq(entity_type e, Sequence seq)
	{
		return add_seq(get_key(e), seq);
	}

	/// Adds the specified components to the specified entity
	/**
	 *  @see add_seq
	 *  @pre !has_some<Components...>(k)
	 *  @post has_all<Components...>(k)
	 */
	template <typename ... Components>
	manager& add(entity_key k, Components&& ... c)
	{
		return add_seq(k, mp::tie(c...));
	}

	/// Adds the specified components to the specified entity
	/**
	 *  @see add_seq
	 *  @pre !has_some<Components...>(e)
	 *  @post has_all<Components...>(e)
	 */
	template <typename ... Components>
	manager& add(entity_type e, Components&& ... c)
	{
		return add_seq(e, mp::tie(c...));
	}

	/// Creates a new entity and adds the specified components
	/**
	 *  @see add_seq
	 *  @see create
	 *  @post has_all_seq< Sequence >(result)
	 */
	template <typename Sequence>
	entity_type create_seq(Sequence seq)
	{
		entity_type result;
		add_seq(result, seq);
		return result;
	}

	/// Creates a new entity and adds the specified components
	/**
	 *  @see add
	 *  @see create_seq
	 *  @post has_all<Components...>(result)
	 */
	template <typename ... Components>
	entity_type create(Components ... c)
	{
		return create_seq(mp::make_tuple(c...));
	}

	/// Removes the specified components from the specified entity
	/**
	 *  @see remove
	 *  @pre has_all_seq< Sequence >(ek)
	 *  @post !has_some_seq< Sequence >(ek)
	 */
	template <typename Sequence>
	manager& remove_seq(entity_key ek, const Sequence& seq = Sequence())
	{
		_do_rem_seq(
			ek,
			_get_bits(seq),
			[](_component_remover& remover)
			{
				mp::for_each<Sequence>(remover);
			}
		);
		return *this;
	}

	/// Removes the specified components from the specified entity
	/**
	 *  @see remove
	 *  @pre has_all_seq< Sequence >(e)
	 *  @post !has_some_seq< Sequence >(e)
	 */
	template <typename Sequence>
	manager& remove_seq(entity_type e, const Sequence& seq = Sequence())
	{
		return remove_seq(get_key(e), seq);
	}

	/// Removes the specified components from the specified entity
	/**
	 *  @see remove_seq
	 *  @pre has_all<Components...>(k)
	 *  @post !has_some<Components...>(k)
	 */
	template <typename ... Components>
	manager& remove(entity_key k)
	{
		typename _sortn<typename _fix1<Components>::type...>::type seq;
		return remove_seq(k, seq);
	}

	/// Removes the specified components from the specified entity
	/**
	 *  @see remove_seq
	 *  @pre has_all<Components...>(e)
	 *  @post !has_some<Components...>(e)
	 */
	template <typename ... Components>
	manager& remove(entity_type e)
	{
		typename _sortn<typename _fix1<Components>::type...>::type seq;
		return remove_seq(e, seq);
	}

	/// Replaces the specified components in the specified entity
	/**
	 *  @see replace
	 *  @pre has_all_seq< Sequence >(ek)
	 *  @post has_all_seq< Sequence >(ek)
	 */
	template <typename Sequence>
	manager& replace_seq(entity_key ek, const Sequence& seq)
	{
		_do_rep_seq(
			ek,
			_get_bits(seq),
			[&seq](_component_replacer& replacer)
			{
				mp::for_each<Sequence>(seq, replacer);
			}
		);
		return *this;
	}

	/// Replaces the specified components in the specified entity
	/**
	 *  @see replace
	 *  @pre has_all_seq< Sequence >(e)
	 *  @post has_all_seq< Sequence >(e)
	 */
	template <typename Sequence>
	manager& replace_seq(entity_type e, Sequence seq)
	{
		return replace_seq(get_key(e), seq);
	}

	/// Replaces the specified components in the specified entity
	/**
	 *  @see replace_seq
	 *  @pre has_all<Components...>(k)
	 *  @post has_all<Components...>(k)
	 */
	template <typename ... Components>
	manager& replace(entity_key k, Components&& ... c)
	{
		return replace_seq(k, mp::tie(c...));
	}

	/// Replaces the specified components in the specified entity
	/**
	 *  @see replace_seq
	 *  @pre has_all<Components...>(e)
	 *  @post has_all<Components...>(e)
	 */
	template <typename ... Components>
	manager& replace(entity_type e, Components&& ... c)
	{
		return replace_seq(e, mp::tie(c...));
	}

	template <typename Component>
	void replace_component_at(
		entity_key ek,
		typename _component_storage::component_key ck,
		Component&& component
	)
	{
		const std::size_t cid = component_id<Component, Group>::value;
		assert(ek->second._component_bits.test(cid));

		_component_key_vector& new_keys = ek->second._component_keys;

		const typename _component_index_map::index_vector
			&new_map = _component_indices.get(
				ek->second._component_bits
			);

		assert(new_keys[new_map[cid]] == ck);

		new_keys[new_map[cid]] = _storage.template replace<Component>(
			ck,
			std::move(component)
		);
	}

	/// Copy the specified components between the specified entities
	/**
	 *  @see copy
	 *  @pre has_all_seq< Sequence >(f)
	 *  @pre !has_some_seq< Sequence >(t)
	 *  @post has_all_seq< Sequence >(f)
	 *  @post has_all_seq< Sequence >(t)
	 */
	template <typename Sequence>
	manager& copy_seq(
		entity_key f,
		entity_key t,
		const Sequence& seq = Sequence()
	)
	{
		_do_cpy_seq(
			f, t,
			_get_bits(seq),
			[](_component_copier& copier)
			{
				mp::for_each<Sequence>(copier);
			}
		);
		return *this;
	}

	/// Copy the specified components between the specified entities
	/**
	 *  @see copy
	 *  @pre has_all_seq< Sequence >(from)
	 *  @pre !has_some_seq< Sequence >(to)
	 *  @post has_all_seq< Sequence >(from)
	 *  @post has_all_seq< Sequence >(to)
	 */
	template <typename Sequence>
	manager& copy_seq(
		entity_type from,
		entity_type to,
		const Sequence& seq = Sequence()
	)
	{
		return copy_seq(_get_entity(from), _get_entity(to), seq);
	}

	/// Copy the specified components between the specified entities
	/**
	 *  @see copy_seq
	 *  @pre has_all<Components...>(f)
	 *  @pre !has_some<Components...>(t)
	 *  @post has_all<Components...>(f)
	 *  @post has_all<Components...>(t)
	 */
	template <typename ... Components>
	manager& copy(entity_key f, entity_key t)
	{
		typename _sortn<typename _fix1<Components>::type...>::type seq;
		return copy_seq(f, t, seq);
	}

	/// Copy the specified components between the specified entities
	/**
	 *  @see copy_seq
	 *  @pre has_all<Components...>(from)
	 *  @pre !has_some<Components...>(to)
	 *  @post has_all<Components...>(from)
	 *  @post has_all<Components...>(to)
	 */
	template <typename ... Components>
	manager& copy(entity_type from, entity_type to)
	{
		typename _sortn<typename _fix1<Components>::type...>::type seq;
		return copy_seq(from, to, seq);
	}

	/// Returns true if the specified entity has the specified Component
	template <typename Component>
	bool has(entity_key ek)
	{
		typedef typename _fix1<Component>::type fixed_C;
		const std::size_t cid = component_id<fixed_C, Group>::value;
		assert(ek != _entities.end());
		return ek->second._component_bits.test(cid);
	}

	/// Returns true if the specified entity has the specified Component
	template <typename Component>
	bool has(entity_type e)
	{
		typedef typename _fix1<Component>::type fixed_C;
		const std::size_t cid = component_id<fixed_C, Group>::value;
		typename _entity_info_map::const_iterator ek = _entities.find(e);
		if(ek == _entities.end()) return false;
		return ek->second._component_bits.test(cid);
	}

	/// Returns true if the specified entity has all the specified Components
	template <typename Sequence>
	bool has_all_seq(entity_key ek, Sequence seq = Sequence())
	{
		_component_bitset _req_bits = _get_bits(seq);
		assert(ek != _entities.end());
		return ((ek->second._component_bits & _req_bits) == _req_bits);
	}

	/// Returns true if the specified entity has all the specified Components
	template <typename Sequence>
	bool has_all_seq(entity_type e, Sequence seq = Sequence())
	{
		_component_bitset _req_bits = _get_bits(seq);
		typename _entity_info_map::const_iterator ek = _entities.find(e);
		if(ek == _entities.end()) return false;
		return ((ek->second._component_bits & _req_bits) == _req_bits);
	}

	/// Returns true if the specified entity has all the specified Components
	template <typename ... Components>
	bool has_all(entity_key ek)
	{
		typename _sortn<typename _fix1<Components>::type...>::type seq;
		return has_all_seq(ek, seq);
	}

	/// Returns true if the specified entity has all the specified Components
	template <typename ... Components>
	bool has_all(entity_type e)
	{
		typename _sortn<typename _fix1<Components>::type...>::type seq;
		return has_all_seq(e, seq);
	}

	/// Returns true if the specified entity has some of the Components
	template <typename Sequence>
	bool has_some_seq(entity_key ek, Sequence seq = Sequence())
	{
		_component_bitset _req_bits = _get_bits(seq);
		assert(ek != _entities.end());
		return (ek->second._component_bits & _req_bits).any();
	}

	/// Returns true if the specified entity has some of the Components
	template <typename Sequence>
	bool has_some_seq(entity_type e, Sequence seq = Sequence())
	{
		_component_bitset _req_bits = _get_bits(seq);
		typename _entity_info_map::const_iterator ek = _entities.find(e);
		if(ek == _entities.end()) return false;
		return (ek->second._component_bits & _req_bits).any();
	}

	/// Returns true if the specified entity has some of the Components
	template <typename ... Components>
	bool has_some(entity_key ek)
	{
		typename _sortn<typename _fix1<Components>::type...>::type seq;
		return has_some_seq(ek, seq);
	}

	/// Returns true if the specified entity has some of the Components
	template <typename ... Components>
	bool has_some(entity_type e)
	{
		typename _sortn<typename _fix1<Components>::type...>::type seq;
		return has_some_seq(e, seq);
	}

	/// Creates an unlocked Lockable managing Component lifetime
	/** This function returns an instance of type conforming to Lockable
	 *  concept which can be used to prevent component instances from
	 *  being destroyed immediately after they are removed from an entity
	 *  (for example from another thread).
	 *  Component instances are reference counted and are kept alive when
	 *  they are assigned to an entity or if a shared reference to the
	 *  component instance exists.
	 *  If there are no shared references and a component is removed from
	 *  the entity it is destroyed unless a lifetime lock is locked on
	 *  the specified component type. In such case the components are
	 *  destroyed only after all lifetime locks are unlocked.
	 *
	 *  @note The resulting Lockable is unlocked when returned and must
	 *  be locked and unlocked explicitly by the caller.
	 *
	 *  @see raw_access_lock
	 *  @see raw_access
	 */
	template <typename ... C>
	group_lock<typename mp::instead<C, poly_lock>::type...>
	lifetime_lock(void)
	{
		return group_lock<
			typename mp::instead<C,poly_lock>::type...
		>(_storage.template lifetime_lock<typename _fix1<C>::type>()...);
	}

	template <typename ... C>
	auto raw_access_lock_tl(mp::typelist<C...>) -> decltype(
		make_group_lock(
			_storage.template access_lock<typename _fix1<C>::type>(
				get_component_access<C>()
			)...
		)
	)
	{
		return make_group_lock(
			_storage.template access_lock<typename _fix1<C>::type>(
				get_component_access<C>()
			)...
		);
	}

	/// Creates an unlocked Lockable that can be used to sync raw access
	/** This function returns an instance of type conforming to Lockable
	 *  concept that can be used to ensure safe access to the specified
	 *  Components from a thread.
	 *
	 *  Either shared access for read-only operations or exclusive access
	 *  for modifying operations to multiple components may be requested.
	 *  Shared access is obtainied by specifying plain Component type name,
	 *  exclusive access is obtained by specifying a reference-to-Component
	 *  type name.
	 *  Multiple threads having shared access can safely read the values
	 *  of the components, a thread having exclusive access can safely
	 *  modify the values of the components provided that locks are obtained
	 *  consistently from all threads and properly used.
	 *
	 *  Regardless in what order the Components were specified, this function
	 *  orders them by their static unique component ids and the individual
	 *  component locks are obtained using a deadlock-preventing algorithm.
	 *  The deadlock prevention however works only if all accessed components
	 *  are locked together.
	 *
	 *  @note The resulting Lockable is unlocked when returned and must
	 *  be locked and unlocked explicitly by the caller.
	 *
	 *  @see lifetime_lock
	 *  @see raw_access
	 */
	template <typename ... Components>
	auto raw_access_lock(void) -> decltype(
		std::declval<manager>().raw_access_lock_tl(
			typename _sortn<Components...>::type()
		)
	)
	{
		return raw_access_lock_tl(
			typename _sortn<Components...>::type()
		);
	}

	/// Gets a reference to the specified Component of the specified entity
	/** This function provides direct access to the a component of the
	 *  specified type of the specified entity. If the entity does not
	 *  have the specified Component then the result of calling this
	 *  function are undefined and will likely lead to program termination.
	 *
	 *  @pre has<Component>(ek);
	 *
	 *  @warning This function may be safely used in single-threaded
	 *  applications to get access to a Component of an entity.
	 *  In multi-threaded programs when accessing components by using
	 *  raw_access, a lifetime lock and a raw access lock must be obtained
	 *  and locked to prevent other threads from removing the accessed
	 *  component from the entity and to prevent them from modifying it.
	 *  For example:
	 *
	 *  @code
	 *  manager<> m;
	 *  std::vector<entity<>::type> entities(n);
	 *
	 *  auto lt_lock = m.lifetime_lock<Component1, Component2>();
	 *  auto ra_lock = m.raw_access_lock<Component1&, Comoponent2>();
	 *
	 *  for(auto& e: entities)
	 *  {
	 *      auto value = examine_c2(m.raw_access<Component2>());
	 *      modify_c1(m.raw_access<Component1>(), value);
	 *  }
	 *  @endcode
	 *
	 *  @see lifetime_lock
	 *  @see raw_access_lock
	 *  @see ref
	 */
	template <typename Component>
	Component& raw_access(entity_key ek)
	{
		typedef typename _fix1<Component>::type _fixed_C;
		std::size_t cid = component_id<_fixed_C, Group>::value;
		typename _component_storage::component_key key;
		assert(ek->second._component_bits.test(cid));

		typename component_index<_fixed_C>::type cidx =
			_component_indices.get(
				ek->second._component_bits
			)[cid];

		key = ek->second._component_keys[cidx];
		return _storage.template access<_fixed_C>(key);
	}

	/// Equivalent to raw_access<Component>(ek);
	/** @warning Read the documentation for raw_access before using
	 *  this function.
	 *
	 *  @see lifetime_lock
	 *  @see raw_access_lock
	 *  @see raw_access
	 *  @see ref
	 */ 
	template <typename Component>
	Component& rw(entity_key ek)
	{
		return raw_access<Component>(ek);
	}

	template <typename Component>
	Component& rw(entity_type e)
	{
		return raw_access<Component>(_find_entity(e));
	}

	/// Gets a thread-safe shared reference to entity's component
	/**
	 *  The manager that created this reference must not be destroyed
	 *  while any copies of the reference are still valid. Then the
	 *  instance of the referenced component remains valid even if
	 *  if is removed from the entity.
	 *
	 *  @note When executing the same operation accessing entity components
	 *  on multiple entities, it is much more efficient obtain the lifetime
	 *  and raw-access locks and use the raw_access member function instead.
	 *
	 *  @see lifetime_lock
	 *  @see raw_access_lock
	 *  @see raw_access
	 */
	template <typename Component, typename Access>
	shared_component<Group, Component, Access>
	ref(entity_key ek, Access)
	{
		std::size_t cid = component_id<Component, Group>::value;
		typename _component_storage::component_key key;
		if(ek->second._component_bits.test(cid))
		{

			typename component_index<Component>::type cidx =
				_component_indices.get(
					ek->second._component_bits
				)[cid];

			key = ek->second._component_keys[cidx];
		}
		else
		{
			key = _storage.null_key();
		}

		return shared_component<Group, Component, Access>(
			*this,
			ek,
			key
		);
	}

	/// Gets a thread-safe shared reference to entity's component
	/**
	 *  The manager that created this reference must not be destroyed
	 *  while any copies of the reference are still valid. Then the
	 *  instance of the referenced component remains valid even if
	 *  if is removed from the entity.
	 */
	template <typename Component>
	shared_component<Group, Component, component_access_read_only>
	cref(entity_key ek)
	{
		return ref<Component>(ek, component_access_read_only());
	}

	/// Gets a thread-safe shared reference to entity's component
	/**
	 *  The manager that created this reference must not be destroyed
	 *  while any copies of the reference are still valid. Then the
	 *  instance of the referenced component remains valid even if
	 *  if is removed from the entity.
	 */
	template <typename Component>
	shared_component<Group, Component, component_access_read_only>
	cref(entity_type e)
	{
		return cref<Component>(_find_entity(e));
	}

	/// Gets a thread-safe shared reference to entity's component
	/**
	 *  The manager that created this reference must not be destroyed
	 *  while any copies of the reference are still valid. Then the
	 *  instance of the referenced component remains valid even if
	 *  if is removed from the entity.
	 */
	template <typename Component>
	shared_component<Group, Component, component_access_read_write>
	ref(entity_key ek)
	{
		return ref<Component>(ek, component_access_read_write());
	}

	/// Gets a thread-safe shared reference to entity's component
	/**
	 *  The manager that created this reference must not be destroyed
	 *  while any copies of the reference are still valid. Then the
	 *  instance of the referenced component remains valid even if
	 *  if is removed from the entity.
	 */
	template <typename Component>
	shared_component<Group, Component, component_access_read_write>
	ref(entity_type e)
	{
		return ref<Component>(_find_entity(e));
	}

	template <typename Visitor>
	struct _visitor_wrapper
	{
		Visitor& _visitor;
		manager& _manager;
		entity_key& _key;

		template <typename Component>
		void operator()(mp::identity<Component>) const
		{
			if(_manager.has<Component>(_key))
			{
				_visitor(
					_manager,
					_key,
					_manager.template read<Component>(_key)
				);
			}
		}
	};

	template <typename Visitor>
	manager& visit_each(Visitor visitor)
	{
		typename _entity_info_map::iterator
			i = _entities.begin(),
			e = _entities.end();

		if(i != e)
		{
			_visitor_wrapper<Visitor> vw = {
				visitor,
				_storage,
				*this,
				i
			};
			
			while(i != e)
			{
				if(visitor(*this, i))
				{
					typedef typename components<Group>::type
						component_seq;
					mp::for_each<component_seq>(vw);
					visitor();
				}
				++i;
			}
		}
		return *this;
	}

	/// Calls the specified function on the specified entity
	manager& apply(
		entity_key key,
		const std::function<void (
			manager&,
			entity_key
		)>& function
	)
	{
		function(*this, key);
	}

	/// Calls the specified function on the specified entity
	manager& apply(
		entity_type e,
		const std::function<void (
			manager&,
			entity_key
		)>& function
	)
	{
		function(*this, _find_entity(e));
	}

	/// Calls the specified function on each entity
	manager& for_each(
		const std::function<void (
			manager&,
			entity_key
		)>& function
	);

	/// The entity range type
	typedef entity_range_tpl<
		Group,
		manager_entity_range<Group>
	> entity_range;

	/// Returns an entity_range containing entities satisfying a predicate
	entity_range select(
		const std::function<
			bool(manager&, entity_key)
		>& predicate
	)
	{
		return entity_range(
			*this, 
			manager_entity_range<Group>(
				_entities.begin(),
				_entities.end()
			),
			predicate
		);
	}

	/// Returns an entity_range containing entities with the Components
	template <typename ... Components>
	entity_range select_with(void)
	{
		return select(entity_with<Components...>());
	}
};

} // namespace exces

#endif //include guard

