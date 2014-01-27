/**
 *  @example exces/003_classifier.cpp
 *  @brief Simple example of classifier usage.
 *
 *  Copyright 2012-2013 Matus Chochlik. Distributed under the Boost
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

	const std::size_t n = 200;
	std::vector<excess::entity> e(n);
	excess::manager m;
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

	excess::classification<std::string> named(
		m,
		[](excess::manager& m, excess::manager::entity_key k) -> bool
		{
			return m.has<name>(k);
		},
		[](excess::manager& m, excess::manager::entity_key k) -> std::string
		{
			return m.ref<name>(k).read();
		}
	);

	for(std::size_t i=0; i!=nn; ++i)
	{
		std::cout << names[i] << ": ";
		named.for_each(
			names[i],
			exces::adapt_func_c<surname>(
				[](const surname& the_surname) -> void
				{
					std::cout << the_surname << " ";
				}
			)
		);
		std::cout << std::endl;
	}

	return 0;
}
