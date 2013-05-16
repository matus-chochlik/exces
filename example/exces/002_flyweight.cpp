/**
 *  @example exces/002_flyweight.cpp
 *  @brief Flyweight components
 *
 *  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <exces/exces.hpp>

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
	std::vector<exces::entity<>::type> e(n);
	exces::manager<> m;
	m.reserve<name>(n);
	m.reserve<surname>(n);

	for(std::size_t i=0; i!=n; ++i)
	{
		m.add(
			e[i],
			name(names[std::rand()%nn]),
			surname(surnames[std::rand()%ns])
		);
	}

	std::cout << "---------------------------" << std::endl;
	std::cout << m.read<name>(e[0]) << " " << m.read<surname>(e[0]) << std::endl;
	std::cout << m.read<name>(e[1]) << " " << m.read<surname>(e[1]) << std::endl;
	std::cout << "---------------------------" << std::endl;
	for(auto r=m.select_with<name, surname>(); !r.empty(); r.next())
	{
		std::cout << r.read<name>() << " ";
		std::cout << r.read<surname>() << ".";
		std::cout << std::endl;
	};

	m.write<surname>(e[0]).append("-Doe");
	m.ref<surname>(e[1]).replace("Roe");
	std::cout << "---------------------------" << std::endl;
	std::cout << m.read<name>(e[0]) << " " << m.read<surname>(e[0]) << std::endl;
	std::cout << m.read<name>(e[1]) << " " << m.read<surname>(e[1]) << std::endl;
	std::cout << "---------------------------" << std::endl;

	for(auto r=m.select_with<name, surname>(); !r.empty(); r.next())
	{
		std::cout << r.read<name>() << " ";
		std::cout << r.read<surname>() << ".";
		std::cout << std::endl;
	};

	return 0;
}
