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

#include <array>
#include <vector>
#include <map>
#include <bitset>
#include <cassert>
#include <stdexcept>

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
		typename _entity_info_map::const_iterator pos
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
	/// Adds the specified components to the specified entity
	template <typename Sequence>
	manager& add_seq(entity e, Sequence seq)
	{
		typename _entity_info_map::iterator p = _get_entity(e);

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
	template <typename ... Components>
	manager& add(entity e, Components ... c)
	{
		return add_seq(e, mp::make_tuple(c...));
	}

	/// Removes the specified components from the specified entity
	template <typename Sequence>
	manager& remove_seq(entity e, const Sequence& seq = Sequence())
	{
		typename _entity_info_map::iterator p = _get_entity(e);
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
	template <typename ... Components>
	manager& remove(entity e)
	{
		return remove_seq<mp::typelist<Components...> >(e);
	}

	/// Copy the specified components between the specified entities
	template <typename Sequence>
	manager& copy_seq(entity from, entity to, const Sequence& seq=Sequence())
	{
		typename _entity_info_map::iterator f = _get_entity(from);
		typename _entity_info_map::iterator t = _get_entity(to);

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
	template <typename ... Components>
	manager& copy(entity from, entity to)
	{
		return copy_seq(from, to, mp::typelist<Components...>());
	}

	/// Returns if the specified entity has all the specified Components
	template <typename ... Components>
	bool has_all(entity e)
	{
		_component_bitset _req_bits = _get_bits<Components...>();
		typename _entity_info_map::const_iterator p = _entities.find(e);
		if(p == _entities.end()) return false;
		return ((p->second._component_bits & _req_bits) == _req_bits);
	}

	/// Returns if the specified entity has some of the Components
	template <typename ... Components>
	bool has_some(entity e)
	{
		_component_bitset _req_bits = _get_bits<Components...>();
		typename _entity_info_map::const_iterator p = _entities.find(e);
		if(p == _entities.end()) return false;
		return (p->second._component_bits & _req_bits).any();
	}

	/// Gets the specified component of the specified entity
	template <typename Component>
	Component& access(entity e)
	{
		return _storage.template access<Component>(
			_get_component_key<Component>(_find_entity(e))
		);
	}

	/// Calls the specified function on each entity with all Components
	template <typename ... Components>
	manager& for_each(const std::function<void (Components& ...)>& function)
	{
		_component_bitset _req_bits = _get_bits<Components...>();
		typename _entity_info_map::const_iterator
			i = _entities.begin(),
			e = _entities.end();

		while(i != e)
		{
			// if the current entity has all requested components
			if((i->second._component_bits & _req_bits) == _req_bits)
			{
				function(
					_storage.template access<Components>(
						_get_component_key<Components>(i)
					)...
				);
			}
			++i;
		}

		return *this;
	}

	/// Calls the specified function on each entity with all Components
	template <typename ... Components>
	manager& for_each(
		const std::function<void (
			manager&,
			entity,
			Components& ...
		)>& function
	)
	{
		_component_bitset _req_bits = _get_bits<Components...>();
		typename _entity_info_map::const_iterator
			i = _entities.begin(),
			e = _entities.end();

		while(i != e)
		{
			// if the current entity has all requested components
			if((i->second._component_bits & _req_bits) == _req_bits)
			{
				function(
					*this,
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
};

} // namespace exces

#endif //include guard

