/**
 *  @example exces/003_hello_any.cpp
 *  @brief Example showing the usage of exces any_manager.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <exces/simple.hpp>
#include <exces/any.hpp>

#include <iostream>
#include <vector>

struct greeting
{
	greeting(const std::string& s) : str(s) { }

	std::string str;
};
EXCES_REG_NAMED_COMPONENT(greeting)


struct location
{
	location(const std::string& s) : str(s) { }

	std::string str;
};
EXCES_REG_NAMED_COMPONENT(location)

struct name
{
	name(const std::string& s) : str(s) { }

	std::string str;
};
EXCES_REG_NAMED_COMPONENT(name)

#include <exces/implement.hpp>

namespace exces {

} // namespace exces

int main(void)
{
	using exces::simple::entity;
	using exces::simple::manager;
	using exces::any_entity_key;
	using exces::any_manager;

	std::vector<entity> e(4);
	manager m;
	any_manager<> am(m);

	am.add(e[0], greeting("Heya"));
	am.add(e[1], location("World"), greeting("Hello"));
	am.add(e[2], greeting("Hi"), location("there"));
	am.add(e[3], greeting("Yo"), name("Frankie"));

	std::function<bool (any_manager<>&, const any_entity_key&)> func1 =
		[](any_manager<>& mgr, const any_entity_key& aek) -> bool
		{
			if(mgr.has_all<greeting, name>(aek))
			{
				std::cout
					<< mgr.rw<greeting>(aek).str
					<< " "
					<< mgr.rw<name>(aek).str
					<< "!"
					<< std::endl;
			}
			return true;
		};

	am.for_each(func1);

	std::cout << std::endl;

	std::function<bool (greeting&)> func2 =
		[](greeting& x) -> bool
		{
			std::cout << x.str << std::endl;
			return true;
		};

	am.for_each(func2);
	return 0;
}
