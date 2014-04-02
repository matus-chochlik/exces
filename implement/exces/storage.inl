/**
 *  @file exces/storage.inl
 *  @brief Implementation of component storage functions
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <exces/aux_/metaprog.hpp>
#include <thread>
#include <vector>
#include <array>
#include <map>

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
// entry_vector
//------------------------------------------------------------------------------
template <typename Component>
class component_entry_vector
{
private:
	typedef std::size_t component_key;

	std::vector<component_storage_entry<Component> > _ents;
	std::vector<component_key> _gc_keys;
	int _next_free;
	int _vector_refs;

public:
	component_entry_vector(void)
	 : _next_free(-1)
	 , _vector_refs(0)
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
			_ents.at(result)._component = std::move(component);
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

	void do_release(component_key key)
	{
		_ents.at(key)._neg_rc_or_nf = _next_free;
		_next_free = int(key);
	}

	bool release(component_key key)
	{
		assert(_ents.at(key)._neg_rc_or_nf < 0);
		if(++_ents.at(key)._neg_rc_or_nf == 0)
		{
			if(_vector_refs)
			{
				_gc_keys.push_back(key);
			}
			else do_release(key);
			return true;
		}
		return false;
	}

	void for_each(const std::function<bool (Component&)>& function)
	{
		for(auto& ent : _ents)
		{
			if(!function(ent._component))
			{
				break;
			}
		}
	}

	void gc(void)
	{
		for(auto key: _gc_keys)
		{
			do_release(key);
		}
	}

	void lock(void)
	{
		++_vector_refs;
	}

	void unlock(void)
	{
		if(_vector_refs-- == 0)
		{
			gc();
		}
	}
};
//------------------------------------------------------------------------------
// normal_storage_vector
//------------------------------------------------------------------------------
template <typename Group, typename Component>
class normal_storage_vector
 : public component_storage_vector<Group, Component>
{
public:
	typedef std::size_t component_key;
private:
	component_entry_vector<Component> _ents;

	typedef component_locking<Group, Component> _locking;
	typedef typename _locking::shared_lock shared_lock;
	typedef typename _locking::unique_lock unique_lock;

	typedef typename _locking::shared_mutex _mutex;
	typedef typename _locking::template lock_guard<_mutex> _mutex_guard;

	_mutex _acc_mutex;
	_mutex _mod_mutex;
public:
	shared_lock read_lock(void)
	{
		return shared_lock(_acc_mutex, std::defer_lock);
	}

	unique_lock write_lock(void)
	{
		return unique_lock(_acc_mutex, std::defer_lock);
	}

	Component& at(component_key key)
	{
		return _ents.at(key);
	}

	void reserve(std::size_t size)
	{
		_mutex_guard l(_mod_mutex);
		_ents.reserve(size);
	}

	component_key store(Component&& component)
	{
		_mutex_guard l(_mod_mutex);
		return _ents.store(std::move(component));
	}

	component_key replace(component_key key, Component&& component)
	{
		return _ents.replace(key, std::move(component));
	}

	component_key copy(component_key key)
	{
		_mutex_guard l(_mod_mutex);
		return _ents.copy(key);
	}

	void add_ref(component_key key)
	{
		_mutex_guard l(_mod_mutex);
		_ents.add_ref(key);
	}

	bool release(component_key key)
	{
		_mutex_guard l(_mod_mutex);
		return _ents.release(key);
	}

	void for_each(const std::function<bool (Component&)>& function)
	{
		_ents.for_each(function);
	}

	void lock(void)
	{
		_mutex_guard l(_mod_mutex);
		_ents.lock();
	}

	bool try_lock(void)
	{
		if(_mod_mutex.try_lock())
		{
			_mutex_guard l(_mod_mutex, std::adopt_lock);
			_ents.lock();
			return true;
		}
		return false;
	}

	void unlock(void)
	{
		_mutex_guard l(_mod_mutex);
		_ents.unlock();
	}
};
//------------------------------------------------------------------------------
// backbuf_storage_vector
//------------------------------------------------------------------------------
template <typename Group, typename Component, std::size_t N>
class backbuf_storage_vector
 : public component_storage_vector<Group, Component>
{
public:
	typedef std::size_t component_key;
private:
	typedef component_locking<Group, Component> _locking;
	typedef typename _locking::mutex _mutex;
	typedef typename _locking::template lock_guard<_mutex> _mutex_guard;
	
	std::array<component_entry_vector<Component>, N> _ents;
	std::map<std::thread::id, std::size_t> _thread_buffs;
	std::size_t _current;

	std::size_t _next(void)
	{
		return (_current+1)%N;
	}

	void _swap_buf(void)
	{
		if(++_current == N)
		{
			_current = 0;
		}
	}

	_mutex _rd_mutex;
	_mutex _wr_mutex;

	void _locking_error(void)
	{
		assert(!"Component access locking error!");
	}

	void _begin_read(void)
	{
		auto tid = std::this_thread::get_id();
		auto res = _thread_buffs.insert({tid, _current});
		if(!res.second) _locking_error();
	}

	void _begin_write(void)
	{
		_ents[_next()] = _ents[_current];
		auto tid = std::this_thread::get_id();
		auto res = _thread_buffs.insert({tid, _next()});
		if(!res.second) _locking_error();
	}

	void _finish_read(void)
	{
		auto pos = _thread_buffs.find(std::this_thread::get_id());
		if(pos == _thread_buffs.end()) _locking_error();
		_thread_buffs.erase(pos);
	}

	void _finish_write(void)
	{
		auto pos = _thread_buffs.find(std::this_thread::get_id());
		if(pos == _thread_buffs.end()) _locking_error();
		_thread_buffs.erase(pos);
		_swap_buf();
	}

	struct _read_lock : lock_intf
	{
		backbuf_storage_vector* _parent;

		backbuf_storage_vector& _p(void)
		{
			assert(_parent);
			return *_parent;
		}

		void lock(void)
		{
			_mutex_guard l(_p()._rd_mutex);
			_p()._begin_read();
		}

		bool try_lock(void)
		{
			if(_p()._rd_mutex.try_lock())
			{
				_mutex_guard l(_p()._rd_mutex, std::adopt_lock);
				_p()._begin_read();
				return true;
			}
			return false;
		}

		void unlock(void)
		{
			_mutex_guard l(_p()._rd_mutex);
			_p()._finish_read();
		}
	} _rd_lock;
	friend struct _read_lock;

	struct _write_lock : lock_intf
	{
		backbuf_storage_vector* _parent;

		backbuf_storage_vector& _p(void)
		{
			assert(_parent);
			return *_parent;
		}

		void lock(void)
		{
			_mutex_guard l(_p()._rd_mutex);
			_p()._wr_mutex.lock();
			_p()._begin_write();
		}

		bool try_lock(void)
		{
			if(_p()._rd_mutex.try_lock())
			{
				_mutex_guard l(_p()._rd_mutex, std::adopt_lock);
				if(_p()._wr_mutex.try_lock())
				{
					_p()._begin_write();
					return true;
				}
			}
			return false;
		}

		void unlock(void)
		{
			_mutex_guard l(_p()._rd_mutex);
			_p()._finish_write();
			_p()._wr_mutex.unlock();
		}
	} _wr_lock;
	friend struct _write_lock;

	component_entry_vector<Component>& _curr_ents(void)
	{
		_mutex_guard l(_rd_mutex);
		auto p = _thread_buffs.find(std::this_thread::get_id());
		if(p == _thread_buffs.end()) _locking_error();

		return _ents[p->second];
	}
public:
	backbuf_storage_vector(void)
	 : _current(0)
	{
		_rd_lock._parent = this;
		_wr_lock._parent = this;
	}

	poly_lock read_lock(void)
	{
		return poly_lock(&_rd_lock);
	}

	poly_lock write_lock(void)
	{
		return poly_lock(&_wr_lock);
	}

	Component& at(component_key key)
	{
		return _curr_ents().at(key);
	}

	void reserve(std::size_t size)
	{
		_curr_ents().reserve(size);
	}

	component_key store(Component&& component)
	{
		return _curr_ents().store(std::move(component));
	}

	component_key replace(component_key key, Component&& component)
	{
		return _curr_ents().replace(key, std::move(component));
	}

	component_key copy(component_key key)
	{
		return _curr_ents().copy(key);
	}

	void add_ref(component_key key)
	{
		_curr_ents().add_ref(key);
	}

	bool release(component_key key)
	{
		return _curr_ents().release(key);
	}

	void for_each(const std::function<bool (Component&)>& function)
	{
		_curr_ents().for_each(function);
	}

	void lock(void)
	{
		_curr_ents().lock();
	}

	bool try_lock(void)
	{
		_curr_ents().lock();
		return true;
	}

	void unlock(void)
	{
		_curr_ents().unlock();
	}
};
//------------------------------------------------------------------------------
// flyweight_storage_vector
//------------------------------------------------------------------------------
template <typename Group, typename Component>
class flyweight_storage_vector
 : public component_storage_vector<Group, Component>
{
public:
	typedef std::size_t component_key;
private:
	normal_storage_vector<Group, Component> _ents;
	std::map<Component, component_key> _index;

	typedef component_locking<Group, Component> _locking;
	typedef typename _locking::shared_lock shared_lock;
	typedef typename _locking::unique_lock unique_lock;

	typedef typename _locking::shared_mutex _mutex;
	typedef typename _locking::template lock_guard<_mutex> _mutex_guard;

	_mutex _mod_mutex;
public:
	shared_lock read_lock(void)
	{
		return _ents.read_lock();
	}

	unique_lock write_lock(void)
	{
		return _ents.write_lock();
	}

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
		_mutex_guard l(_mod_mutex);
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
		_mutex_guard l(_mod_mutex);
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
			_mutex_guard l(_mod_mutex);
			_index.erase(_ents.at(key));
			return true;
		}
		return false;
	}

	void for_each(const std::function<bool (Component&)>& function)
	{
		_ents.for_each(function);
	}

	void lock(void)
	{
		_ents.lock();
	}

	bool try_lock(void)
	{
		return _ents.try_lock();
	}

	void unlock(void)
	{
		_ents.unlock();
	}
};
//------------------------------------------------------------------------------
// component_storage
//------------------------------------------------------------------------------
template <typename Component, typename Group>
flyweight_storage_vector<Group, Component>
storage_vector_type(component_kind_flyweight);
//------------------------------------------------------------------------------
template <typename Component, typename Group>
backbuf_storage_vector<Group, Component, 2> // TODO: multiple buffers?
storage_vector_type(component_kind_backbuf);
//------------------------------------------------------------------------------
template <typename Component, typename Group>
normal_storage_vector<Group, Component>
storage_vector_type(component_kind_normal);
//------------------------------------------------------------------------------
template <typename Group>
struct component_storage_init
{
	component_storage<Group>& storage;

	template <typename Component>
	void operator()(mp::identity<Component>) const
	{
		typedef decltype(storage_vector_type<Component, Group>(
			typename component_kind<Component, Group>::type()
		)) csv_t;

		component_storage_vector<Group, Component>* pcsv = new csv_t();
		
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
