/**
 *  @file exces/storage.inl
 *  @brief Implementation of component storage functions
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <exces/aux_/metaprog.hpp>
#include <vector>

namespace exces {

//------------------------------------------------------------------------------
// component_storage_entry
//------------------------------------------------------------------------------
template <typename Component>
struct component_storage_entry
{
	// negative reference count (if < 0)
	// or the next free component entry
	// in the vector
	int _neg_rc_or_nf;
	Component _component;

	component_storage_entry(Component&& component)
	 : _neg_rc_or_nf(0)
	 , _component(std::move(component))
	{ }
};
//------------------------------------------------------------------------------
// heavyweight_storage_vector
//------------------------------------------------------------------------------
template <typename Component>
class heavyweight_storage_vector
 : public component_storage_vector<Component>
{
private:
	typedef component_storage_entry<Component> _component_entry;

	std::vector<component_storage_entry<Component> > _ents;
	int _next_free;
public:
	typedef std::size_t component_key;

	heavyweight_storage_vector(void)
	 : _next_free(-1)
	{ }

	Component& at(component_key key)
	{
		return _ents.at(key)._component;
	}

	void reserve(std::size_t size)
	{
		_ents.reserve(size);
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
		return store(Component(at(key)));
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
//------------------------------------------------------------------------------
// flyweight_storage_vector
//------------------------------------------------------------------------------
template <typename Component>
class flyweight_storage_vector
 : public component_storage_vector<Component>
{
public:
	typedef std::size_t component_key;
private:
	heavyweight_storage_vector<Component> _ents;
	std::map<Component, component_key> _index;
public:
	Component& at(component_key key)
	{
		return _ents.at(key);
	}

	void reserve(std::size_t size)
	{
		_ents.reserve(size);
	}

	component_key store(Component&& component)
	{
		auto p = _index.find(component);
		if(p == _index.end())
		{
			auto k = _ents.store(std::move(component));
			_index[_ents.at(k)] = k;
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
		if(_ents.at(key) == component) return key;

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
			_index.erase(_ents.at(key));
			return true;
		}
		return false;
	}
};
//------------------------------------------------------------------------------
// component_storage
//------------------------------------------------------------------------------
template <typename Group>
struct component_storage_init
{
	component_storage<Group>& storage;

	template <typename Component>
	void operator()(mp::identity<Component>) const
	{
		typedef typename mp::if_c<
			flyweight_component<Component, Group>::value,
			flyweight_storage_vector<
				typename std::remove_reference<Component>::type
			>,
			heavyweight_storage_vector<
				typename std::remove_reference<Component>::type
			>
		>::type csv_t;

		component_storage_vector<Component>* pcsv = new csv_t();
		
		assert(pcsv != nullptr);

		typedef component_id<Component, Group> cid;
		mp::get<cid::value>(storage._store) = pcsv;
	}
};
//------------------------------------------------------------------------------
template <typename Group>
struct component_storage_cleanup
{
	component_storage<Group>& storage;

	template <typename Component>
	void operator()(mp::identity<Component>) const
	{
		typedef component_id<Component, Group> cid;
		delete mp::get<cid::value>(storage._store);
		mp::get<cid::value>(storage._store) = nullptr;
	}
};
//------------------------------------------------------------------------------
template <typename Group>
component_storage<Group>::
component_storage(void)
{
	component_storage_init<Group> init = {*this};
	mp::for_each<typename components<Group>::type>(init);
}
//------------------------------------------------------------------------------
template <typename Group>
component_storage<Group>::
~component_storage(void)
{
	component_storage_cleanup<Group> cleanup = {*this};
	mp::for_each<typename components<Group>::type>(cleanup);
}
//------------------------------------------------------------------------------
} // namespace exces
