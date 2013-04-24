/**
 *  @file exces/manager.hpp
 *  @brief Implements component manager
 *
 *  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_MANAGER_1212101457_HPP
#define EXCES_MANAGER_1212101457_HPP

#include <exces/entity.hpp>
#include <exces/storage.hpp>
#include <exces/component.hpp>

#include <array>
#include <vector>
#include <map>
#include <bitset>
#include <cassert>
#include <stdexcept>
#include <functional>
#include <iterator>

namespace exces {

/// Manages the components of entities
template <typename Group = default_group>
class manager
{
private:
	// component storage
	typedef component_storage<Group> _component_storage;
	_component_storage _storage;

	// the count of components in the Group
	typedef mp::size<components<Group> > _component_count;

	// type of the bitset used to indicate which components
	// an entity has
	typedef std::bitset<_component_count::value> _component_bitset;

	// less than comparison for bitsets smaller than uint
	struct _component_bitset_less_small
	{
		bool operator()(
			const _component_bitset& a,
			const _component_bitset& b
		) const
		{
			return a.to_ulong() < b.to_ulong();
		}
	};

	// less than comparison for bitsets bigger than uint
	struct _component_bitset_less_big
	{
		bool operator()(
			const _component_bitset& a,
			const _component_bitset& b
		) const
		{
			for(std::size_t i=0; i!=_component_count(); ++i)
			{
				if(a[i] < b[i]) return true;
				if(a[i] > b[i]) return false;
			}
			return false;
		}
	};

	// the less than comparison functor to be used
	// when comparing two bitsets
	typedef typename mp::if_c<
		_component_count::value < sizeof(unsigned long)*8,
		_component_bitset_less_small,
		_component_bitset_less_big
	>::type _component_bitset_less;

	// type of the component index (unsigned int with sufficient range)
	typedef typename component_index<Group>::type _component_index_t;

	// converts static group-unique component-ids for a bitset with
	// a particular combination of bits set into a vector of indices
	// to a vector-of-keys pointing to the individual components
	// (ordered by their ids) in the storage.
	struct _component_index_map
	{
	public:
		typedef std::array<
			_component_index_t,
			_component_count::value
		> _index_vector;
	private:
		typedef std::map<
			_component_bitset,
			_index_vector,
			_component_bitset_less
		> _index_map;

		_index_map _indices;
	public:
		const _index_vector& get(_component_bitset bits) const
		{
			typename _index_map::iterator p = _indices.find(bits);
			assert(p != _indices.end());
			return p->second;
		}

		const _index_vector& get(_component_bitset bits)
		{
			std::pair<
				typename _index_map::iterator,
				bool
			> r = _indices.insert(
				typename _index_map::value_type(
					bits,
					_index_vector()
				)
			);
			assert(r.first != _indices.end());
			// if a new element was inserted
			if(r.second)
			{
				_component_index_t i = 0;
				for(std::size_t j=0; j!=_component_count(); ++j)
				{
					if(bits[j]) r.first->second[j] = i++;
				}
			}
			return r.first->second;
		}
	} _component_indices;

	// a vector of keys that allow to access the components
	// (ordered by their ids) in the storage
	struct _component_key_vector
	 : std::vector<typename _component_storage::key_t>
	{
		typedef typename _component_storage::key_t key_t;

		_component_key_vector(void)
		{ }

		_component_key_vector(std::size_t n)
		 : std::vector<key_t>(n, _component_storage::null_key())
		{ }
	};

	// information about a single entity
	struct _entity_info
	{
		_component_bitset _component_bits;

		_component_key_vector _component_keys;
	};

	// helper functor that reserves storage space for a Component
	struct _component_reserver
	{
		_component_storage& _storage;
		std::size_t n;

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
				component_id<Component>::value,
				Value
			);
		}
	};

	// gets the bitset for the specified components
	template <typename Sequence>
	static _component_bitset _get_bits(const Sequence&)
	{
		_component_bitset _bits;
		_component_bit_setter<true> bset = { _bits };
		mp::for_each<Sequence>(bset);
		return std::move(_bits);
	}

	template <typename ... Components>
	static _component_bitset _get_bits(void)
	{
		return _get_bits(mp::typelist<Components...>());
	}

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
			std::size_t cid = component_id<Component>::value;
			_keys[cid] = _storage.store(std::move(component));
		}
	};

	// helper functor that removes a Component from the storage
	struct _component_remover
	{
		_component_storage& _storage;
		_component_key_vector& _keys;

		template <typename Component>
		void operator()(mp::identity<Component>) const
		{
			std::size_t cid = component_id<Component>::value;
			_storage.template release<Component>(_keys[cid]);
		}
	};

	// helper functor that copies components between entities
	struct _component_copier
	{
		_component_storage& _storage;
		_component_key_vector& _src_keys;
		_component_key_vector& _dst_keys;
		const typename _component_index_map::_index_vector& _idx_map;

		template <typename Component>
		void operator()(mp::identity<Component>) const
		{
			std::size_t cid = component_id<Component>::value;
			_dst_keys[_idx_map[cid]] =
				_storage.template copy<Component>(
					_src_keys[_idx_map[cid]]
				);
		}
	};

	// a map that stores the information about entities
	// ordered by the id of the entity
	typedef std::map<entity, _entity_info> _entity_info_map;
	typedef typename _entity_info_map::value_type _entity_info_entry;
	_entity_info_map _entities;

	// gets the information about an entity, inserts a new one
	// if the entity is not registered yet
	typename _entity_info_map::iterator _get_entity(entity e)
	{
		typename _entity_info_map::iterator p = _entities.insert(
			_entity_info_entry(e, _entity_info())
		).first;
		assert(p != _entities.end());
		return p;
	}

	// gets the information about an entity, throws if the entity
	// is not registered
	typename _entity_info_map::iterator _find_entity(entity e)
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

	// get the key of the specified component of an entity
	// pointed to by pos
	template <typename Component>
	typename _component_storage::key_t _get_component_key(
		typename _entity_info_map::iterator pos
	)
	{
		std::size_t cid = component_id<Component>::value;
		if(!pos->second._component_bits.test(cid))
		{
			throw ::std::invalid_argument(
				"exces::entity manager: "
				"entity does not have requested component"
			);
		}

		typename component_index<Component>::type cidx =
			_component_indices.get(pos->second._component_bits)[cid];

		return pos->second._component_keys[cidx];
	}
public:
	/// Key for O(1) access to entity data
	typedef typename _entity_info_map::iterator entity_key;

	/// Returns a vector of keys for O(1) access to entities
	/**
	 *  The returned keys are invalidated by removal of the entity
	 *  from the manager.
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
	 */
	std::vector<entity_key> get_keys(const std::vector<entity>& es)
	{
		return get_keys(es.begin(), es.end());
	}

	/// Get a key for O(1) access to the specified entity
	/**
	 *  The returned keys are invalidated by removal of the entity
	 *  from the manager.
	 */
	entity_key get_key(entity e)
	{
		return _get_entity(e);
	}

	/// Get the entity pointed to by key
	entity get_entity(entity_key k)
	{
		return k->first;
	}

	/// Reserves space for n instances of Components in Seqence
	template <typename Sequence>
	manager& reserve_seq(std::size_t n, Sequence seq = Sequence())
	{
		_component_reserver reserver = { _storage, n };
		mp::for_each<Sequence>(reserver);
		return *this;
	}

	/// Reserves storage space 
	template <typename ... Components>
	manager& reserve(std::size_t n)
	{
		return reserve_seq(n, mp::typelist<Components...>());
	}

	/// Adds the specified components to the specified entity
	template <typename Sequence>
	manager& add_seq(entity_key p, Sequence seq)
	{
		_component_bitset  old_bits = p->second._component_bits;

		_component_bit_setter<true> bset = { p->second._component_bits };
		mp::for_each<Sequence>(bset);

		_component_bitset& new_bits = p->second._component_bits;

		const std::size_t cc = _component_count();
		_component_key_vector tmp_keys(cc);
		_component_adder adder = { _storage, tmp_keys };
		mp::for_each(seq, adder);
		
		_component_key_vector  new_keys(new_bits.count());
		_component_key_vector& old_keys = p->second._component_keys;
		assert(old_keys.size() == old_bits.count());

		const typename _component_index_map::_index_vector
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

		return *this;
	}

	/// Adds the specified components to the specified entity
	template <typename Sequence>
	manager& add_seq(entity e, Sequence seq)
	{
		return add_seq(get_key(e), seq);
	}

	/// Adds the specified components to the specified entity
	template <typename ... Components>
	manager& add(entity_key k, Components ... c)
	{
		return add_seq(k, mp::make_tuple(c...));
	}

	/// Adds the specified components to the specified entity
	template <typename ... Components>
	manager& add(entity e, Components ... c)
	{
		return add_seq(e, mp::make_tuple(c...));
	}

	/// Removes the specified components from the specified entity
	template <typename Sequence>
	manager& remove_seq(entity_key p, const Sequence& seq = Sequence())
	{
		_component_bitset _rem_bits = _get_bits(seq);
		if((p->second._component_bits & _rem_bits) != _rem_bits)
		{
			throw ::std::invalid_argument(
				"exces::entity manager: "
				"removing components that "
				"the entity does not have"
			);
		}

		_component_bitset  old_bits = p->second._component_bits;

		_component_bit_setter<false> bset = { p->second._component_bits };
		mp::for_each<Sequence>(bset);

		_component_bitset& new_bits = p->second._component_bits;
		
		_component_key_vector  new_keys(new_bits.count());
		_component_key_vector& old_keys = p->second._component_keys;
		assert(old_keys.size() == old_bits.count());

		const typename _component_index_map::_index_vector
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
		mp::for_each<Sequence>(remover);

		swap(new_keys, old_keys);

		return *this;
	}

	/// Removes the specified components from the specified entity
	template <typename Sequence>
	manager& remove_seq(entity e, const Sequence& seq = Sequence())
	{
		return remove_seq(get_key(e), seq);
	}

	/// Removes the specified components from the specified entity
	template <typename ... Components>
	manager& remove(entity_key k)
	{
		return remove_seq<mp::typelist<Components...> >(k);
	}

	/// Removes the specified components from the specified entity
	template <typename ... Components>
	manager& remove(entity e)
	{
		return remove_seq<mp::typelist<Components...> >(e);
	}

	/// Copy the specified components between the specified entities
	template <typename Sequence>
	manager& copy_seq(
		entity_key f,
		entity_key t,
		const Sequence& seq=Sequence()
	)
	{
		_entity_info& fei = f->second;
		_entity_info& tei = t->second;

		tei._component_bits = fei._component_bits;
		tei._component_keys.resize(fei._component_keys.size());

		const typename _component_index_map::_index_vector &idx_map =
			_component_indices.get(fei._component_bits);

		_component_copier copier = {
			_storage,
			fei._component_keys,
			tei._component_keys,
			idx_map
		};
		mp::for_each<Sequence>(copier);

		return *this;
	}

	/// Copy the specified components between the specified entities
	template <typename Sequence>
	manager& copy_seq(entity from, entity to, const Sequence& seq=Sequence())
	{
		return copy_seq(_get_entity(from), _get_entity(to), seq);
	}

	/// Copy the specified components between the specified entities
	template <typename ... Components>
	manager& copy(entity_key f, entity_key t)
	{
		return copy_seq(f, t, mp::typelist<Components...>());
	}

	/// Copy the specified components between the specified entities
	template <typename ... Components>
	manager& copy(entity from, entity to)
	{
		return copy_seq(from, to, mp::typelist<Components...>());
	}

	/// Returns if the specified entity has all the specified Components
	template <typename Sequence>
	bool has_all_seq(entity_key p, Sequence seq = Sequence())
	{
		_component_bitset _req_bits = _get_bits(seq);
		assert(p != _entities.end());
		return ((p->second._component_bits & _req_bits) == _req_bits);
	}

	/// Returns if the specified entity has all the specified Components
	template <typename Sequence>
	bool has_all_seq(entity e, Sequence seq = Sequence())
	{
		_component_bitset _req_bits = _get_bits(seq);
		typename _entity_info_map::const_iterator p = _entities.find(e);
		if(p == _entities.end()) return false;
		return ((p->second._component_bits & _req_bits) == _req_bits);
	}

	/// Returns if the specified entity has all the specified Components
	template <typename ... Components>
	bool has_all(entity_key p)
	{
		return has_all_seq(p, mp::typelist<Components...>());
	}

	/// Returns if the specified entity has all the specified Components
	template <typename ... Components>
	bool has_all(entity e)
	{
		return has_all_seq(e, mp::typelist<Components...>());
	}

	/// Returns if the specified entity has some of the Components
	template <typename Sequence>
	bool has_some_seq(entity_key p, Sequence seq = Sequence())
	{
		_component_bitset _req_bits = _get_bits(seq);
		assert(p != _entities.end());
		return (p->second._component_bits & _req_bits).any();
	}

	/// Returns if the specified entity has some of the Components
	template <typename Sequence>
	bool has_some_seq(entity e, Sequence seq = Sequence())
	{
		_component_bitset _req_bits = _get_bits(seq);
		typename _entity_info_map::const_iterator p = _entities.find(e);
		if(p == _entities.end()) return false;
		return (p->second._component_bits & _req_bits).any();
	}

	/// Returns if the specified entity has some of the Components
	template <typename ... Components>
	bool has_some(entity_key p)
	{
		return has_some_seq(p, mp::typelist<Components...>());
	}

	/// Returns if the specified entity has some of the Components
	template <typename ... Components>
	bool has_some(entity e)
	{
		return has_some_seq(e, mp::typelist<Components...>());
	}

	/// Gets the specified component of the specified entity
	template <typename Component>
	Component& access(entity_key p)
	{
		assert(p != _entities.end());
		return _storage.template access<Component>(
			_get_component_key<Component>(p)
		);
	}

	/// Gets the specified component of the specified entity
	template <typename Component>
	Component& access(entity e)
	{
		return _storage.template access<Component>(
			_get_component_key<Component>(_find_entity(e))
		);
	}

	/// Gets a shared reference to entity's component
	/**
	 *  The manager that created this reference must not be destroyed
	 *  while any copies of the reference are still valid. Then the
	 *  instance of the referenced component remains valid even if
	 *  if is removed from the entity.
	 */
	template <typename Component>
	shared_component<Component, Group> ref(entity_key p)
	{
		std::size_t cid = component_id<Component>::value;
		typename _component_storage::key_t key;
		if(p->second._component_bits.test(cid))
		{

			typename component_index<Component>::type cidx =
				_component_indices.get(
					p->second._component_bits
				)[cid];

			key = p->second._component_keys[cidx];
		}
		else
		{
			key = _storage.null_key();
		}

		return shared_component<Component, Group>(_storage, key);
	}

	/// Gets a shared reference to entity's component
	template <typename Component>
	shared_component<Component, Group> ref(entity e)
	{
		return ref<Component>(_find_entity(e));
	}

	/// Calls the specified function on each entity having specified Components
	template <typename ... Components>
	manager& for_each(
		const std::function<void (
			manager&,
			entity_key,
			entity,
			Components& ...
		)>& function
	)
	{
		_component_bitset _req_bits = _get_bits<Components...>();
		typename _entity_info_map::iterator
			i = _entities.begin(),
			e = _entities.end();

		while(i != e)
		{
			// if the current entity has all requested components
			if((i->second._component_bits & _req_bits) == _req_bits)
			{
				function(
					*this,
					i,
					i->first,
					_storage.template access<Components>(
						_get_component_key<Components>(i)
					)...
				);
			}
			++i;
		}

		return *this;
	}

	/// Calls the specified function on each entity having specified Components
	template <typename ... Components>
	manager& for_each(
		const std::function<void (
			manager&,
			entity,
			Components& ...
		)>& function
	)
	{
		std::function<void (manager&, entity_key, entity, Components& ...)> wf =
		[&function](manager& m, entity_key, entity e, Components& ... components)
		{
			function(m, e, components...);
		};
		return for_each(wf);
	}

	/// Calls the specified function on each entity having specified Components
	template <typename ... Components>
	manager& for_each(const std::function<void (Components& ...)>& function)
	{
		std::function<void (manager&, entity_key, entity, Components& ...)> wf =
		[&function](manager&, entity_key, entity, Components& ... components)
		{
			function(components...);
		};
		return for_each(wf);
	}

	/// Range for traversal of entities that are the result of a query
	struct entity_range
	{
	private:
		manager& _m;
		entity_key _i;
		const entity_key _e;
		std::function<bool (manager&, entity_key)> _pred;

		void _skip(void)
		{
			while(!empty() && !_pred(_m, front()))
			{
				++_i;
			}
		}
	public:
		entity_range(
			manager& man,
			entity_key i,
			entity_key e,
			std::function<bool (manager&, entity_key)> pred
		): _m(man)
		 , _i(i)
		 , _e(e)
		 , _pred(pred)
		{
			_skip();
		}

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
			_skip();
		}
	};

	/// Returns an entity_range containing entities satisfying predicate
	entity_range select(std::function<bool(manager&, entity_key)> predicate)
	{
		return entity_range(
			*this, 
			_entities.begin(),
			_entities.end(),
			predicate
		);
	}

	template <typename Sequence>
	struct with_components
	{
		bool operator ()(manager& m, entity_key key) const
		{
			return m.has_all_seq(key, Sequence());
		}
	};

	template <typename Sequence>
	entity_range select_with_seq(Sequence seq = Sequence())
	{
		return select(with_components<Sequence>());
	}

	template <typename ... Components>
	entity_range select_with(void)
	{
		return select_with_seq(mp::typelist<Components...>());
	}
};

} // namespace exces

#endif //include guard

