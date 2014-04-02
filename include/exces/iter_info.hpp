/**
 *  @file exces/iter_info.hpp
 *  @brief Implements the iter_info class
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef EXCES_ITER_INFO_1212101457_HPP
#define EXCES_ITER_INFO_1212101457_HPP

#include <cstddef>

namespace exces {

/// Class providing information about iteration progress
class iter_info
{
private:
	std::size_t _i, _n;
public:
	iter_info(std::size_t n)
	 : _i(0)
	 , _n(n)
	{ }

	// called inside for_each, etc.
	void step(void)
	{
		assert(_i != _n);
		++_i;
	}

	/// Returns the current position in the iteration
	std::size_t pos(void) const
	{
		return _i;
	}

	/// Returns the total number of steps in the iteration
	std::size_t count(void) const
	{
		return _n;
	}

	/// Whether this is the first step in the iteration
	bool is_first(void) const
	{
		return _i == 0;
	}

	/// Whether this is the last step in the iteration
	bool is_last(void) const
	{
		return _i == _n-1;
	}
};

} // namespace exces

#endif //include guard

