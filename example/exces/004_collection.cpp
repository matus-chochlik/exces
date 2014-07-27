/**
 *  @example exces/004_collection.cpp
 *  @brief Simple example of entity collection usage.
 *
 *  Copyright 2012-2014 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <exces/simple.hpp>
#include <exces/func_adaptors/ic.hpp>
#include <exces/func_adaptors/mkc.hpp>

#include <iostream>
#include <vector>

struct person
{
	std::string name;
	std::string surname;
	unsigned age;

	person(const char* n, const char* sn, unsigned a)
	 : name(n)
	 , surname(sn)
	 , age(a)
	{ }

	bool is_child(void) const
	{
		return age <= 12;
	}

	bool is_teenager(void) const
	{
		return (age > 12) && (age < 20);
	}

	bool is_adult(void) const
	{
		return age >= 18;
	}

	bool is_senior(void) const
	{
		return age >= 65;
	}
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

	const std::size_t n = 30;
	std::vector<excess::entity> e(n);
	excess::manager m;
	m.reserve<person>(n);

	for(std::size_t i=0; i!=n; ++i)
	{
		m.add(
			e[i],
			person(
				names[std::rand()%nn],
				surnames[std::rand()%ns],
				std::rand()%100
			)
		);
	}

	excess::collection children(m, &person::is_child);
	excess::collection teenagers(m, &person::is_teenager);
	excess::collection adults(m, &person::is_adult);
	excess::collection seniors(m, &person::is_senior);

	std::function<bool (const exces::iter_info&, const person&)>
	printer = [](const exces::iter_info& ii, const person& p) -> bool
	{
		std::cout
			<< p.name << " "
			<< p.surname << " ("
			<< p.age << ")";
		if(ii.is_last())
			std::cout << ".";
		else std::cout << ", ";
		return true;
	};

	std::function<bool (
		excess::manager&,
		excess::manager::entity_key,
		person&
	)> skip5years = [](
		excess::manager& m,
		excess::manager::entity_key ek,
		person& p
	) -> bool
	{
		auto mod_op = m.begin_update(ek);
		p.age += 5;
		m.finish_update(ek, mod_op);
		return true;
	};

	std::cout << "Children: ";
	children.for_each(exces::adapt_func(printer));
	std::cout << std::endl << std::endl;

	std::cout << "Teenagers: ";
	teenagers.for_each(exces::adapt_func(printer));
	std::cout << std::endl << std::endl;

	std::cout << "Adults: ";
	adults.for_each(exces::adapt_func(printer));
	std::cout << std::endl << std::endl;

	std::cout << "Seniors: ";
	seniors.for_each(exces::adapt_func(printer));
	std::cout << std::endl << std::endl;

	std::cout << "-----------------" << std::endl;
	std::cout << "Skipping 5 years." << std::endl;
	std::cout << "-----------------" << std::endl;
	m.for_each(exces::adapt_func(skip5years));

	std::cout << "Children: ";
	children.for_each(exces::adapt_func(printer));
	std::cout << std::endl << std::endl;

	std::cout << "Teenagers: ";
	teenagers.for_each(exces::adapt_func(printer));
	std::cout << std::endl << std::endl;

	std::cout << "Adults: ";
	adults.for_each(exces::adapt_func(printer));
	std::cout << std::endl << std::endl;

	std::cout << "Seniors: ";
	seniors.for_each(exces::adapt_func(printer));
	std::cout << std::endl << std::endl;

	return 0;
}
