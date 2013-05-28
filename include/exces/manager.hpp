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
#include <exces/classification.hpp>

#include <array>
#include <vector>
#include <map>
#include <bitset>
#include <cassert>
#include <stdexcept>
#include <functional>
#include <algorithm>
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
		_component_count::value <= sizeof(unsigned long)*8,
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
	static _component_bitset _gen_bits(const Sequence&)
	{
		_component_bitset _bits;
		_component_bit_setter<true> bset = { _bits };
		mp::for_each<Sequence>(bset);
		return std::move(_bits);
	}

	// gets the bitset for the specified components
	template <typename Sequence>
	static const _component_bitset& _get_bits(const Sequence& seq)
	{
		static _component_bitset _bits = _gen_bits(seq);
		return _bits;
	}

	template <typename ... Components>
	static const _component_bitset& _get_bits(void)
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
			const std::size_t cid =
				component_id<Component, Group>::value;
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
			const std::size_t cid =
				component_id<Component, Group>::value;
			_storage.template release<Component>(_keys[cid]);
		}
	};

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

	// helper functor that copies components between entities
	struct _component_copier
	{
		_component_storage& _storage;
		_component_key_vector& _src_keys;
		_component_key_vector& _dst_keys;
		const typename _component_index_map::_index_vector& _src_map;
		const typename _component_index_map::_index_vector& _dst_map;

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

	// get the key of the specified component of an entity
	// pointed to by pos
	template <typename Component>
	typename _component_storage::component_key _get_component_key(
		typename _entity_info_map::iterator pos
	)
	{
		const std::size_t cid = component_id<Component, Group>::value;
		if(!pos->second._component_bits.test(cid))
		{
			throw ::std::invalid_argument(
				"exces::entity manager: "
				"entity does not have requested component"
			);
		}

		const typename component_index<Component>::type cidx =
			_component_indices.get(
				pos->second._component_bits
			)[cid];

		return pos->second._component_keys[cidx];
	}

	std::vector<any_classification<Group>*> _classifications;
	friend class any_classification<Group>;

	void add_classification(any_classification<Group>* cl)
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

	void move_classification(
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

	void remove_classification(any_classification<Group>* cl)
	{
		auto p = std::find(
			_classifications.begin(),
			_classifications.end(),
			cl
		);
		assert(p != _classifications.end());
		_classifications.erase(p);
	}

	typedef std::vector<std::size_t> _class_update_key_list;

	_class_update_key_list _begin_class_update(
		typename _entity_info_map::iterator key
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

	void _finish_class_update(
		typename _entity_info_map::iterator key,
		const _class_update_key_list& update_keys
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
public:
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
		return reserve_seq(n, mp::typelist<Components...>());
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
		auto updates = _begin_class_update(ek);

		_component_bitset add_bits = _get_bits(seq);

		_component_bitset  old_bits = ek->second._component_bits;

		ek->second._component_bits |= add_bits;

		_component_bitset& new_bits = ek->second._component_bits;

		const std::size_t cc = _component_count();
		_component_key_vector tmp_keys(cc);
		_component_adder adder = { _storage, tmp_keys };
		mp::for_each(seq, adder);
		
		_component_key_vector  new_keys(new_bits.count());
		_component_key_vector& old_keys = ek->second._component_keys;
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

		_finish_class_update(ek, updates);

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
	manager& add(entity_key k, Components ... c)
	{
		return add_seq(k, mp::make_tuple(c...));
	}

	/// Adds the specified components to the specified entity
	/**
	 *  @see add_seq
	 *  @pre !has_some<Components...>(e)
	 *  @post has_all<Components...>(e)
	 */
	template <typename ... Components>
	manager& add(entity_type e, Components ... c)
	{
		return add_seq(e, mp::make_tuple(c...));
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
		auto updates = _begin_class_update(ek);

		_component_bitset rem_bits = _get_bits(seq);
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

		_finish_class_update(ek, updates);

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
		return remove_seq<mp::typelist<Components...> >(k);
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
		return remove_seq<mp::typelist<Components...> >(e);
	}

	/// Replaces the specified components in the specified entity
	/**
	 *  @see replace
	 *  @pre has_all_seq< Sequence >(ek)
	 *  @post has_all_seq< Sequence >(ek)
	 */
	template <typename Sequence>
	manager& replace_seq(entity_key ek, const Sequence& seq = Sequence())
	{
		auto updates = _begin_class_update(ek);

		_component_bitset rep_bits = _get_bits(seq);
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

		const typename _component_index_map::_index_vector
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
		mp::for_each<Sequence>(replacer);

		for(std::size_t i=0; i!=cc; ++i)
		{
			if(new_bits.test(i))
			{
				new_keys[new_map[i]] = tmp_keys[i];
			}
		}


		_finish_class_update(ek, updates);

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
	manager& replace(entity_key k, Components ... c)
	{
		return replace_seq(k, mp::make_tuple(c...));
	}

	/// Replaces the specified components in the specified entity
	/**
	 *  @see replace_seq
	 *  @pre has_all<Components...>(e)
	 *  @post has_all<Components...>(e)
	 */
	template <typename ... Components>
	manager& replace(entity_type e, Components ... c)
	{
		return replace_seq(e, mp::make_tuple(c...));
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

		const typename _component_index_map::_index_vector
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
		const Sequence& seq=Sequence()
	)
	{
		auto updates = _begin_class_update(t);

		_entity_info& fei = f->second;
		_entity_info& tei = t->second;

		_component_bitset cpy_bits = _get_bits(seq);

		tei._component_bits |= cpy_bits;
		tei._component_keys.resize(tei._component_bits.count());

		const typename _component_index_map::_index_vector &src_map =
			_component_indices.get(fei._component_bits);
		const typename _component_index_map::_index_vector &dst_map =
			_component_indices.get(tei._component_bits);

		_component_copier copier = {
			_storage,
			fei._component_keys,
			tei._component_keys,
			src_map,
			dst_map
		};
		mp::for_each<Sequence>(copier);

		_finish_class_update(t, updates);

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
		const Sequence&seq=Sequence()
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
		return copy_seq(f, t, mp::typelist<Components...>());
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
		return copy_seq(from, to, mp::typelist<Components...>());
	}

	/// Returns true if the specified entity has the specified Component
	template <typename Component>
	bool has(entity_key ek)
	{
		const std::size_t cid = component_id<Component, Group>::value;
		assert(ek != _entities.end());
		return ek->second._component_bits.test(cid);
	}

	/// Returns true if the specified entity has the specified Component
	template <typename Component>
	bool has(entity_type e)
	{
		const std::size_t cid = component_id<Component, Group>::value;
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
		return has_all_seq(ek, mp::typelist<Components...>());
	}

	/// Returns true if the specified entity has all the specified Components
	template <typename ... Components>
	bool has_all(entity_type e)
	{
		return has_all_seq(e, mp::typelist<Components...>());
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
		return has_some_seq(ek, mp::typelist<Components...>());
	}

	/// Returns true if the specified entity has some of the Components
	template <typename ... Components>
	bool has_some(entity_type e)
	{
		return has_some_seq(e, mp::typelist<Components...>());
	}

	/// Gets a shared reference to entity's component
	/**
	 *  The manager that created this reference must not be destroyed
	 *  while any copies of the reference are still valid. Then the
	 *  instance of the referenced component remains valid even if
	 *  if is removed from the entity.
	 *
	 *  @pre has<Component>(ek)
	 */
	template <typename Component>
	shared_component<Component, Group> ref(entity_key ek)
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

		return shared_component<Component, Group>(*this, ek, _storage, key);
	}

	/// Gets a shared reference to entity's component
	/**
	 *  The manager that created this reference must not be destroyed
	 *  while any copies of the reference are still valid. Then the
	 *  instance of the referenced component remains valid even if
	 *  if is removed from the entity.
	 *
	 *  @pre has<Component>(e)
	 */
	template <typename Component>
	shared_component<Component, Group> ref(entity_type e)
	{
		return ref<Component>(_find_entity(e));
	}

	/// Returns a const reference to the component of the specified entity
	/**
	 *  @pre has<Component>(ek)
	 */
	template <typename Component>
	const Component& read(entity_key ek)
	{
		return ref<Component>(ek).read();
	}

	/// Returns a const reference to the component of the specified entity
	/**
	 *  @pre has<Component>(e)
	 */
	template <typename Component>
	const Component& read(entity_type e)
	{
		return read<Component>(_find_entity(e));
	}

	/// Returns a reference to the component of the specified entity
	/**
	 *  @pre has<Component>(ek)
	 */
	template <typename Component>
	typename shared_component<Component, Group>::component_ref
	write(entity_key ek)
	{
		return ref<Component>(ek).write();
	}

	/// Returns a reference to the component of the specified entity
	/**
	 *  @pre has<Component>(e)
	 */
	template <typename Component>
	typename shared_component<Component, Group>::component_ref
	write(entity_type e)
	{
		return write<Component>(_find_entity(e));
	}

	template <typename Visitor>
	struct _visitor_wrapper
	{
		_component_storage& _storage;
		manager& _manager;
		entity_key _key;
		entity_type _entity;
		Visitor& _visitor;

		template <typename Component>
		void operator()(mp::identity<Component>) const
		{
			if(_manager.has<Component>(_entity))
			{
				typename _component_storage::component_key ck =
					_manager._get_component_key<
						Component
					>(_key);

				_visitor(
					_manager,
					_key,
					_entity,
					_storage.template access<Component>(ck)
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

		while(i != e)
		{
			_visitor_wrapper<Visitor> vw = {
				_storage,
				*this,
				i,
				i->first,
				visitor
			};
			if(visitor(*this, i, i->first))
			{
				typedef typename components<Group>::type
					component_seq;
				mp::for_each<component_seq>(vw);
				visitor();
			}
			++i;
		}
		return *this;
	}

	/// Calls the specified function on each entity having the Components
	template <typename ... Components>
	manager& for_each(
		const std::function<void (
			manager&,
			entity_key,
			entity_type,
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

	/// Calls the specified function on each entity having the Components
	template <typename ... Components>
	manager& for_each(
		const std::function<void (
			manager&,
			entity_type,
			Components& ...
		)>& function
	)
	{
		std::function<
			void (manager&, entity_key, entity_type, Components& ...)
		> wf = [&function](
			manager& m,
			entity_key,
			entity_type e,
			Components& ... components
		)
		{
			function(m, e, components...);
		};
		return for_each(wf);
	}

	/// Calls the specified function on each entity having the Components
	template <typename ... Components>
	manager& for_each(const std::function<void (Components& ...)>& function)
	{
		std::function<
			void (manager&, entity_key, entity_type, Components& ...)
		> wf = [&function](
			manager&,
			entity_key,
			entity_type,
			Components& ... components
		)
		{
			function(components...);
		};
		return for_each(wf);
	}

	/// Calls the specified functor on each entity having the Components
	template <typename ... Components, typename Func>
	manager& for_each_mkec(Func functor)
	{
		std::function<
			void (manager&, entity_key, entity_type, Components& ...)
		> wf = [&functor](
			manager& m,
			entity_key k,
			entity_type e,
			Components& ... components
		)
		{
			functor(m, k, e, components...);
		};
		return for_each(wf);
	}

	/// Calls the specified functor on each entity having the Components
	template <typename ... Components, typename Func>
	manager& for_each_mec(Func functor)
	{
		std::function<
			void (manager&, entity_key, entity_type, Components& ...)
		> wf = [&functor](
			manager& m,
			entity_key,
			entity_type e,
			Components& ... components
		)
		{
			functor(m, e, components...);
		};
		return for_each(wf);
	}

	/// Calls the specified functor on each entity having the Components
	template <typename ... Components, typename Func>
	manager& for_each_c(Func functor)
	{
		std::function<
			void (manager&, entity_key, entity_type, Components& ...)
		> wf = [&functor](
			manager&,
			entity_key,
			entity_type,
			Components& ... components
		)
		{
			functor(components...);
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

		/// Returns the Component of the entity at front of the range
		template <typename Component>
		shared_component<Component> front_component(void) const
		{
			return _m.template ref<Component>(front());
		}

		template <typename Component>
		const Component& read(void) const
		{
			return front_component<Component>().read();
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
	struct _with_components
	{
		bool operator ()(manager& m, entity_key key) const
		{
			return m.has_all_seq(key, Sequence());
		}
	};

	/// Returns an entity_range containing entities with the components
	template <typename Sequence>
	entity_range select_with_seq(Sequence seq = Sequence())
	{
		return select(_with_components<Sequence>());
	}

	/// Returns an entity_range containing entities with the Components
	template <typename ... Components>
	entity_range select_with(void)
	{
		return select_with_seq(mp::typelist<Components...>());
	}
};

namespace aux {

template <typename Component, typename Group>
inline void manager_replace_component_at(
	manager<Group>& mngr,
	typename manager<Group>::entity_key ek,
	typename component_storage<Group>::component_key ck,
	Component&& component
)
{
	mngr.replace_component_at(ek, ck, std::move(component));
}

} // namespace aux
} // namespace exces

#endif //include guard

