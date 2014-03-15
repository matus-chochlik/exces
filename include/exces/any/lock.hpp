/**
 *  @file exces/any/lock.hpp
 *  @brief Type erasure for Lockable types
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_ANY_LOCK_1403152314_HPP
#define EXCES_ANY_LOCK_1403152314_HPP

#include <exces/threads.hpp>

namespace exces {

/// Type erasure for lockable types
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

