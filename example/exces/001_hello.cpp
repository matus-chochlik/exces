/**
 *  @example excess/001_hello.cpp
 *  @brief Introductory example showing the basic usage of exces.
 *
 *  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <exces/exces.hpp>

#include <iostream>
#include <vector>

struct greeting
{
	greeting(const std::string& s) : str(s) { }

	std::string str;
};
EXCES_REG_COMPONENT(greeting)


struct location
{
	location(const std::string& s) : str(s) { }

	std::string str;
};
EXCES_REG_COMPONENT(location)

struct name
{
	name(const std::string& s) : str(s) { }

	std::string str;
};
EXCES_REG_COMPONENT(name)

struct ble
{
	template <typename T>
	struct apply
	{
		typedef std::vector<T> type;
	};
};

int main(void)
{
	std::vector<exces::entity> e(4);

	exces::manager<> m;

	m.add(e[0], greeting("Heya"));
	m.add(e[1], location("World"), greeting("Hello"));
	m.add(e[2], greeting("Hi"), location("there"));
	m.add(e[3], greeting("Yo"), name("Frankie"));

	std::function<void (greeting&, location&)> func =
		[](greeting& g, location& l) -> void
		{
			std::cout << g.str << " " << l.str << "!" << std::endl;
		};

	m.for_each(func);
	return 0;
}
