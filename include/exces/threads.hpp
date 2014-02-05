/**
 *  @file exces/threads.hpp
 *  @brief Multi-threading-related functionality
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_THREADS_1401272119_HPP
#define EXCES_THREADS_1401272119_HPP

#include <exces/group.hpp>
#include <mutex>
#include <cassert>

namespace exces {


struct fake_shared_mutex
{
	fake_shared_mutex(void) = default;
	fake_shared_mutex(fake_shared_mutex&&) = default;
	fake_shared_mutex(const fake_shared_mutex&) = delete;

	void lock(void) { }
	bool try_lock(void) { return true; }
	void unlock(void) { }
};

struct fake_unique_lock
{
	fake_unique_lock(void) = default;
	fake_unique_lock(fake_unique_lock&&) = default;
	fake_unique_lock(const fake_unique_lock&) = delete;
	fake_unique_lock(fake_shared_mutex&) { }
	fake_unique_lock(fake_shared_mutex&, std::defer_lock_t) { }
	fake_unique_lock& operator = (fake_unique_lock&&) { return *this; }
	operator bool (void) const { return true; }

	void lock(void) { }
	bool try_lock(void) { return true; }
	void unlock(void) { }
};

struct fake_shared_lock
{
	fake_shared_lock(void) = default;
	fake_shared_lock(fake_shared_lock&&) = default;
	fake_shared_lock(const fake_shared_lock&) = delete;
	fake_shared_lock(fake_shared_mutex&) { }
	fake_shared_lock(fake_shared_mutex&, std::defer_lock_t) { }
	fake_shared_lock& operator = (fake_shared_lock&&) { return *this; }
	operator bool (void) const { return true; }

	void lock(void) { }
	bool try_lock(void) { return true; }
	void unlock(void) { }
};

template <typename Lockable>
struct fake_lock_guard
{
	fake_lock_guard(fake_lock_guard&&) = default;
	fake_lock_guard(const fake_lock_guard&) = delete;
	fake_lock_guard(Lockable&) { }
};

struct fake_component_locking
{
	typedef fake_shared_mutex shared_mutex;
	typedef fake_unique_lock unique_lock;
	typedef fake_shared_lock shared_lock;

	template <typename Lockable>
	struct lock_guard : fake_lock_guard<Lockable>
	{
		lock_guard(lock_guard&&) = default;
		lock_guard(const lock_guard&) = delete;
		lock_guard(Lockable& l)
		 : fake_lock_guard<Lockable>(l)
		{ }
		lock_guard(Lockable& l, std::adopt_lock_t)
		 : fake_lock_guard<Lockable>(l)
		{ }
	};
};

struct std_component_locking
{
	// TODO std::shared_mutex where available
	typedef std::mutex shared_mutex;
	typedef std::unique_lock<shared_mutex> unique_lock;
	// TODO std::shared_lock where available
	typedef std::unique_lock<shared_mutex> shared_lock;

	template <typename Lockable>
	struct lock_guard : std::lock_guard<Lockable>
	{
		lock_guard(lock_guard&&) = default;
		lock_guard(const lock_guard&) = delete;
		lock_guard(Lockable& l)
		 : std::lock_guard<Lockable>(l)
		{ }
		lock_guard(Lockable& l, std::adopt_lock_t)
		 : std::lock_guard<Lockable>(l)
		{ }
	};
};

template <typename Group, typename Component>
struct component_locking : fake_component_locking
{ };

struct lock_intf
{
	virtual ~lock_intf(void) { }

	virtual void lock(void) = 0;
	virtual bool try_lock(void) = 0;
	virtual void unlock(void) = 0;
};

template <typename ... Lockables>
class tuple_lock
{
private:
	mp::tuple<Lockables...> _lockables;

	typedef typename mp::gen_seq<sizeof ... (Lockables)>::type _seq;

	template <std::size_t ... S>
	void _do_lock(mp::n_seq<S...>)
	{
		std::lock(mp::get<S>(_lockables)...);
	}

	void _do_lock(mp::n_seq<0>)
	{
		mp::get<0>(_lockables).lock();
	}

	template <std::size_t ... S>
	int _do_try_lock(mp::n_seq<S...>)
	{
		return std::try_lock(mp::get<S>(_lockables)...);
	}

	int _do_try_lock(mp::n_seq<0>)
	{
		return mp::get<0>(_lockables).try_lock()?-1:0;
	}

	struct _unlock
	{
		template <typename Lockable>
		void operator()(Lockable& lockable) const
		{
			lockable.unlock();
		}
	};
public:
	tuple_lock(void) = default;
	tuple_lock(tuple_lock&&) = default;
	tuple_lock(Lockables&& ... lockables)
	 : _lockables(std::move(lockables)...)
	{ }

	void lock(void)
	{
		_do_lock(_seq());
	}

	bool try_lock(void)
	{
		return _do_try_lock(_seq());
	}

	void unlock(void)
	{
		mp::for_each(_lockables, _unlock());
	}
};

template <typename ... Lockables>
inline tuple_lock<Lockables...> make_tuple_lock(Lockables&& ... lockables)
{
	return tuple_lock<Lockables...>(std::move(lockables)...);
}

class poly_lock
{
protected:
	lock_intf* _pimpl;
public:
	poly_lock(void)
	 : _pimpl(nullptr)
	{ }

	poly_lock(poly_lock&& tmp)
	 : _pimpl(tmp._pimpl)
	{
		tmp._pimpl = nullptr;
	}

	poly_lock(const poly_lock&) = delete;

	poly_lock(lock_intf* pimpl)
	 : _pimpl(pimpl)
	{
		assert(_pimpl);
	}

	void lock(void)
	{
		assert(_pimpl);
		_pimpl->lock();
	}

	bool try_lock(void)
	{
		assert(_pimpl);
		return _pimpl->try_lock();
	}

	void unlock(void)
	{
		assert(_pimpl);
		_pimpl->unlock();
	}
};

class any_lock
 : public poly_lock
{
private:
	template <typename Lockable>
	struct _impl : lock_intf
	{
		Lockable _lockable;

		_impl(Lockable&& lockable)
		 : _lockable(std::move(lockable))
		{ }

		void lock(void){ _lockable.lock(); }
		bool try_lock(void){ return _lockable.try_lock(); }
		void unlock(void){ _lockable.unlock(); }
	};
public:
	template <typename Lockable>
	any_lock(Lockable&& lockable)
	 : poly_lock(new _impl<Lockable>(std::move(lockable)))
	{
		assert(_pimpl);
	}

	any_lock(const any_lock&) = delete;

	any_lock(any_lock&& tmp)
	 : poly_lock(tmp._pimpl)
	{
		tmp._pimpl = nullptr;
	}

	~any_lock(void)
	{
		if(_pimpl) delete _pimpl;
	}
};

} // namespace exces

#endif //include guard

