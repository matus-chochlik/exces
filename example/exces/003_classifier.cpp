/**
 *  @example exces/003_classifier.cpp
 *  @brief Simple example of classifier usage.
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

	const std::size_t n = 200;
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

	exces::classification<std::string> named(
		m,
		[](exces::manager<>& m, exces::manager<>::entity_key k) -> std::string
		{
			if(m.has<name>(k)) return m.read<name>(k);
			else return std::string();
		},
		[](const std::string& str) -> bool { return !str.empty(); }
	);

	for(std::size_t i=0; i!=nn; ++i)
	{
		std::cout << names[i] << ": ";
		named.for_each_mk(
			names[i],
			[](exces::manager<>& m, exces::manager<>::entity_key k)->void
			{
				if(m.has<surname>(k))
					std::cout << m.read<surname>(k) << " ";
			}
		);
		std::cout << std::endl;
	}

	return 0;
}
