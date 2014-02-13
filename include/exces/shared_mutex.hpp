/**
 *  @file exces/shared_mutex.hpp
 *  @brief Implementation of shared mutex
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_SHARED_MUTEX_1401272119_HPP
#define EXCES_SHARED_MUTEX_1401272119_HPP

#include <exces/group.hpp>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <climits>
#include <cassert>

namespace exces {

// Implementation of shared-ownership mutex
class shared_mutex
{
private:
	std::mutex _mutex;
	std::condition_variable _cond_1;
	std::condition_variable _cond_2;

	typedef unsigned _state_type;
	_state_type _state;

	static constexpr _state_type _writing =
		1U << (sizeof(_state_type)*CHAR_BIT - 1);
	static constexpr _state_type _readers = ~_writing;
public:
	shared_mutex(void)
	 : _state(0)
	{ }

	~shared_mutex(void)
	{
		assert(_state == 0);
	}

	void lock(void)
	{
		std::unique_lock<std::mutex> l(_mutex);
		while(_state & _writing)
		{
			_cond_2.wait(l);
		}

		_state |= _writing;

		while(_state & _readers)
		{
			_cond_1.wait(l);
		}
	}

	bool try_lock(void)
	{
		std::unique_lock<std::mutex> l(_mutex, std::try_to_lock);
		if(l.owns_lock() && (_state == 0))
		{
			_state = _writing;
			return true;
		}
		return false;
	}

	void _clear_state(void)
	{
		std::lock_guard<std::mutex> lg(_mutex);

		assert(_state & _writing);
		_state = 0;
	}

	void unlock(void)
	{
		_clear_state();
		_cond_1.notify_all();
	}

	void lock_shared(void)
	{
		std::unique_lock<std::mutex> l(_mutex);
		while((_state & _writing) || (_state & _readers) == _readers)
		{
			_cond_1.wait(l);
		}

		_state_type num_readers = (_state & _readers) + 1;
		_state &= _writing;
		_state |= num_readers;
	}

	bool try_lock_shared(void)
	{
		std::unique_lock<std::mutex> l(_mutex, std::try_to_lock);
		if(l.owns_lock() && !(_state & _writing))
		{
			if((_state & _readers) != _readers)
			{
				return true;
			}
		}
		return false;
	}

	void unlock_shared(void)
	{
		std::lock_guard<std::mutex> lg(_mutex);

		assert(_state & _readers);
		_state_type num_readers = (_state & _readers) - 1;
		_state &= _writing;
		_state |= num_readers;

		if(_state & _writing)
		{
			if(num_readers == 0)
			{
				_cond_2.notify_one();
			}
		}

		if(num_readers == (_readers - 1))
		{
			_cond_1.notify_one();
		}
	}
};

template <typename Lockable>
class shared_lock
{
private:
	Lockable* _lockable;
	bool _locked;

	void _assert_lockable(void)
	{
		if(!_lockable)
		{
			throw std::system_error(
				std::errc::operation_not_permitted
			);
		}
		if(_locked)
		{
			throw std::system_error(
				std::errc::resource_deadlock_would_occur
			);
		}
	}

	void _assert_unlockable(void)
	{
		assert(_lockable);
		if(!_locked)
		{
			throw std::system_error(
				std::errc::resource_deadlock_would_occur
			);
		}
	}
public:
	shared_lock(void) noexcept
	 : _lockable(nullptr)
	 , _locked(false)
	{ }

	explicit shared_lock(Lockable& lockable)
	 : _lockable(&lockable)
	 , _locked(true)
	{
		_lockable->lock_shared();
	}

	shared_lock(Lockable& lockable, std::defer_lock_t) noexcept
	 : _lockable(&lockable)
	 , _locked(false)
	{ }

	shared_lock(const shared_lock&) = delete;
	shared_lock& operator = (const shared_lock&) = delete;

	shared_lock(shared_lock&& tmp) noexcept
	 : _lockable(tmp._lockable)
	 , _locked(tmp._locked)
	{
		tmp._lockable = nullptr;
		tmp._locked = false;
	}

	shared_lock& operator = (shared_lock&& tmp) noexcept
	{
		_lockable = tmp._lockable;
		tmp._lockable = nullptr;
		_locked = tmp._locked;
		tmp._locked = false;
	}

	~shared_lock(void)
	{
		if(_locked)
		{
			assert(_lockable);
			_lockable->unlock_shared();
		}
	}

	Lockable* mutex(void) const noexcept
	{
		return _lockable;
	}

	bool owns_lock(void) const noexcept
	{
		return _locked;
	}

	explicit operator bool (void) const noexcept
	{
		return _locked;
	}
	
	void lock(void)
	{
		_assert_lockable();
		_lockable->lock_shared();
		_locked = true;
	}

	bool try_lock(void)
	{
		_assert_lockable();
		_locked = _lockable->try_lock_shared();
		return _locked;
	}
	
	void unlock(void)
	{
		_assert_unlockable();
		_lockable->unlock_shared();
		_locked = false;
	}
};

} // namespace exces

#endif //include guard

