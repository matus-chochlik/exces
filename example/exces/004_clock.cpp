/**
 *  @example exces/004_clock.cpp
 *  @brief Example showing simple usage of backbuffered components
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <exces/entity.hpp>
#include <exces/entity/string.hpp>

EXCES_USE_ENTITY_TYPE(default, string)

#include <exces/simple.hpp>
#include <exces/func_adaptors.hpp>

#include <iostream>

namespace test {

struct clock
{
	double time, pace;

	clock(void)
	 : time(0)
	 , pace(1)
	{ }
};
} // namespace test

EXCES_REG_BACKBUF_COMPONENT(test::clock)

#include <exces/implement.hpp>

int main(void)
{
	using exces::simple::entity;
	using exces::simple::manager;
	using exces::adapt_func;
	using exces::iter_info;
	typedef exces::simple::manager::entity_key entity_key;

	auto
	print_sep = [](void) -> void
	{
		std::cout << std::string(55, '-') << std::endl;
	};

	std::function<bool (manager&, entity_key, const test::clock&)>
	print_clock = [](manager& m, entity_key k, const test::clock& c) -> bool
	{
		std::cout << "clock " << m.get_entity(k) << " time = " << c.time << std::endl;
		return true;
	};

	std::function<bool (test::clock&)>
	update_clock = [](test::clock& c) -> bool
	{
		c.time += c.pace;
		return true;
	};

	entity good("good"), late("late"), slow("slow"), fast("fast");
	manager m;

	auto rl = m.raw_access_lock<test::clock>();
	auto wl = m.raw_access_lock<test::clock&>();

	wl.lock();
	m.add(good, test::clock());
	m.add(slow, test::clock());
	m.add(fast, test::clock());
	wl.unlock();

	rl.lock();
	m.for_each(adapt_func(print_clock));
	print_sep();
	rl.unlock();

	wl.lock();
	m.for_each(adapt_func(update_clock));

	m.add(late, test::clock());
	m.rw<test::clock>(slow).pace = 0.5;
	m.rw<test::clock>(fast).pace = 2.0;
	wl.unlock();

	rl.lock();
	m.for_each(adapt_func(print_clock));
	print_sep();
	rl.unlock();

	for(int i=0; i<10; ++i)
	{
		wl.lock();
		m.for_each(adapt_func(update_clock));
		wl.unlock();

		rl.lock();
		m.for_each(adapt_func(print_clock));
		print_sep();
		rl.unlock();
	}

	return 0;
}
