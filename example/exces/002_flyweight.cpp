/**
 *  @example exces/002_flyweight.cpp
 *  @brief Flyweight components
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <exces/simple.hpp>

#include <iostream>
#include <vector>

struct name : std::string
{
	name(const char* s) : std::string(s) { }
};
EXCES_REG_FLYWEIGHT_COMPONENT(name)

struct surname : std::string
{
	surname(const char* s) : std::string(s) { }
};
EXCES_REG_FLYWEIGHT_COMPONENT(surname)

#include <exces/implement.hpp>

int main(void)
{
	const char* names[] = {
		"John", "Paul", "Martin", "Joseph", "William",
		"Patrick", "Sean", "Bruce", "Richard", "Daniel",
		"Rick", "Donna", "Dana", "Paula", "Brenda",
		"Bonnie", "Sophie", "Barbara", "Jane", "Joanne"
	};
	const std::size_t nn = sizeof(names)/sizeof(names[0]);

	const char* surnames[] = {
		"Smith", "Jones", "Brown", "Lee", "Wilson",
		"Pattel", "Tailor", "Thompson"
	};
	const std::size_t ns = sizeof(surnames)/sizeof(surnames[0]);

	const std::size_t n = 500;
	excess::manager m;
	excess::implicit_manager im(m);

	std::vector<excess::implicit_entity> e(n);
	m.reserve<name>(n);
	m.reserve<surname>(n);

	for(std::size_t i=0; i!=n; ++i)
	{
		e[i].add(
			name(names[std::rand()%nn]),
			surname(surnames[std::rand()%ns])
		);
	}

	std::cout << "---------------------------" << std::endl;
	std::cout << e[0].rw<name>() << " " << e[0].rw<surname>() << std::endl;
	std::cout << e[1].rw<name>() << " " << e[1].rw<surname>() << std::endl;
	std::cout << "---------------------------" << std::endl;
	for(auto r=m.select_with<name, surname>(); !r.empty(); r.next())
	{
		std::cout << r.rw<name>() << " ";
		std::cout << r.rw<surname>() << ".";
		std::cout << std::endl;
	};

	e[0].rw<surname>().append("-Doe");
	e[1].ref<surname>().replace("Roe");
	std::cout << "---------------------------" << std::endl;
	std::cout << e[0].rw<name>() << " " << e[0].rw<surname>() << std::endl;
	std::cout << e[1].rw<name>() << " " << e[1].rw<surname>() << std::endl;
	std::cout << "---------------------------" << std::endl;

	for(auto r=m.select_with<name, surname>(); !r.empty(); r.next())
	{
		std::cout << r.rw<name>() << " ";
		std::cout << r.rw<surname>() << ".";
		std::cout << std::endl;
	};

	return 0;
}
