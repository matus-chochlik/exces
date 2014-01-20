/**
 *  @file exces/storage.hpp
 *  @brief Implements component storage
 *
 *  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_STORAGE_1212101457_HPP
#define EXCES_STORAGE_1212101457_HPP

#include <exces/group.hpp>
#include <exces/metaprog.hpp>

#include <map>
#include <vector>
#include <cassert>

namespace exces {

template <typename Component, typename Group>
struct component_equal_to
{
	bool operator()(const Component& a, const Component& b) const
	{
		return a == b;
	}
};

template <typename Component, typename Group>
struct component_less_than
{
	bool operator()(const Component& a, const Component& b) const
	{
		return a < b;
	}
};

template <typename Group = default_group>
class component_storage
{
private:
	template <typename Component>
	struct _component_entry
	{
		// negative reference count (if < 0)
		// or the next free component entry
		// in the vector
		int _neg_rc_or_nf;
		Component _component;

		_component_entry(Component&& component)
		 : _neg_rc_or_nf(0)
		 , _component(std::move(component))
		{ }
	};

	template <typename Component>
	class _component_vector
	{
	private:
		std::vector<_component_entry<Component> > _ents;
		int _next_free;
	public:
		typedef std::size_t component_key;

		_component_vector(void)
		 : _next_free(-1)
		{ }

		void reserve(std::size_t size)
		{
			_ents.reserve(size);
		}

		typename std::vector<_component_entry<Component> >::reference
		at(component_key key)
		{
			return _ents.at(key);
		}

		component_key store(Component&& component)
		{
			component_key result;
			if(_next_free >= 0)
			{
				result = component_key(_next_free);
				_ents.at(result)._component =
					std::move(component);
				_next_free = _ents.at(result)._neg_rc_or_nf;
				_ents.at(result)._neg_rc_or_nf = -1;
			}
			else
			{
				result = _ents.size();
				_ents.push_back(std::move(component));
				_ents.back()._neg_rc_or_nf = -1;
			}
			return result;
		}

		component_key replace(component_key key, Component&& component)
		{
			_ents.at(key)._component = std::move(component);
			return key;
		}

		component_key copy(component_key key)
		{
			return store(Component(at(key)._component));
		}

		void add_ref(component_key key)
		{
			assert(_ents.at(key)._neg_rc_or_nf < 0);
			--_ents.at(key)._neg_rc_or_nf;
		}

		bool release(component_key key)
		{
			assert(_ents.at(key)._neg_rc_or_nf < 0);
			if(++_ents.at(key)._neg_rc_or_nf == 0)
			{
				_ents.at(key)._neg_rc_or_nf = _next_free;
				_next_free = int(key);
				return true;
			}
			return false;
		}
	};

	template <typename Component>
	class _flyweight_vector
	{
	public:
		typedef typename _component_vector<Component>::component_key
			component_key;
	private:
		_component_vector<Component> _ents;
		std::map<Component, component_key> _index;
		component_equal_to<Component, Group> _eq;
	public:
		void reserve(std::size_t size)
		{
			_ents.reserve(size);
		}

		typename std::vector<_component_entry<Component> >::reference
		at(component_key key)
		{
			return _ents.at(key);
		}

		component_key store(Component&& component)
		{
			auto p = _index.find(component);
			if(p == _index.end())
			{
				auto k = _ents.store(std::move(component));
				_index[_ents.at(k)._component] = k;
				return k;
			}
			else
			{
				add_ref(p->second);
				return p->second;
			}
		}

		component_key replace(component_key key, Component&& component)
		{
			if(_eq(_ents.at(key)._component, component)) return key;

			release(key);
			return store(std::move(component));
		}

		component_key copy(component_key key)
		{
			add_ref(key);
			return key;
		}

		void add_ref(component_key key)
		{
			_ents.add_ref(key);
		}

		bool release(component_key key)
		{
			if(_ents.release(key))
			{
				_index.erase(_ents.at(key)._component);
				return true;
			}
			return false;
		}
	};

	struct _add_component_vector_mf
	{
		template <typename C>
		struct apply
		{
			typedef typename mp::if_c<
				flyweight_component<C, Group>::value,
				_flyweight_vector<
					typename std::remove_reference<C>::type
				>,
				_component_vector<
					typename std::remove_reference<C>::type
				>
			>::type type;
		};
	};

	typedef typename mp::as_tuple<
		typename mp::transform<
			components< Group >,
			_add_component_vector_mf
		>::type
	>::type _store_type;

	_store_type _store;

	template <typename Component>
	typename mp::if_c<
		flyweight_component<Component, Group>::value,
		_flyweight_vector<Component>,
		_component_vector<Component>
	>::type& _store_of(void)
	{
		return mp::get<component_id<Component, Group>::value>(_store);
	}
public:
	/// The component key type
	typedef std::size_t component_key;

	/// Return a special NULL component key value
	static component_key null_key(void)
	{
		return component_key(-1);
	}

	/// Reserves space for n instances of Component
	template <typename Component>
	void reserve(std::size_t n)
	{
		_store_of<Component>().reserve(n);
	}

	/// Access the specified Component type by its key
	template <typename Component>
	Component& access(component_key key)
	{
		return _store_of<Component>().at(key)._component;
	}

	/// Stores the specified component and returns
	template <typename Component>
	component_key store(Component&& component)
	{
		return _store_of<Component>().store(std::move(component));
	}

	/// Replaces the value of Component at the specified key
	template <typename Component>
	component_key replace(component_key key, Component&& component)
	{
		return _store_of<Component>().replace(key,std::move(component));
	}

	/// Copies the component at the specified key returns the new key
	template <typename Component>
	component_key copy(component_key key)
	{
		return _store_of<Component>().copy(key);
	}

	/// Adds reference to the component at the specified key
	template <typename Component>
	void add_ref(component_key key)
	{
		_store_of<Component>().add_ref(key);
	}

	/// Releases (removes reference to) the component at the specified key
	template <typename Component>
	bool release(component_key key)
	{
		return _store_of<Component>().release(key);
	}

	template <typename Component>
	void mark_write(component_key key)
	{
		// TODO
	}
};

} // namespace exces

#endif //include guard

