/**
 *  @example exces/002_classifier.cpp
 *  @brief Simple example of classifier usage.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <exces/simple.hpp>
#include <exces/func_adaptors.hpp>

#include <iostream>
#include <vector>

struct person
{
	std::string name;
	std::string surname;

	person(const char* n, const char* sn)
	 : name(n)
	 , surname(sn)
	{ }
};
EXCES_REG_COMPONENT(person)

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
	m.reserve<person>(n);

	for(std::size_t i=0; i!=n; ++i)
	{
		m.add(
			e[i],
			person(names[std::rand()%nn], surnames[std::rand()%ns])
		);
	}

	excess::classification<std::string> namesakes(m, &person::name);

	for(std::size_t i=0; i!=nn; ++i)
	{
		std::cout << names[i] << ": ";
		namesakes.for_each(
			names[i],
			exces::adapt_func_c<person>(
				[](const person& p) -> bool
				{
					std::cout << p.surname << " ";
					return true;
				}
			)
		);
		std::cout << std::endl;
	}

	return 0;
}
