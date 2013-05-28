/**
 *  .file test/exces/entity.cpp
 *  .brief Test case for entity type and related functionality.
 *
 *  .author Matus Chochlik
 *
 *  Copyright 2011-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE EXCES_Entity
#include <boost/test/unit_test.hpp>

#include <exces/entity.hpp>

#include <vector>

BOOST_AUTO_TEST_SUITE(Entity)

BOOST_AUTO_TEST_CASE(Entity_default_construction)
{
	exces::entity<>::type e;
	(void)(e);
}

BOOST_AUTO_TEST_CASE(Entity_copy_construction)
{
	exces::entity<>::type e1;
	exces::entity<>::type e2 = e1;
	(void)(e2);
}

BOOST_AUTO_TEST_CASE(Entity_identity)
{
	exces::entity<>::type e1, e2;
	BOOST_CHECK(e1 != e2);
	exces::entity<>::type e1_c = e1;
	BOOST_CHECK(e1 == e1_c);
}


BOOST_AUTO_TEST_CASE(Entity_identity_2)
{
	std::vector<exces::entity<>::type> ev(1000);
	auto b = ev.begin(), e = ev.end();
	auto i = b;
	while(i != e)
	{
		auto j = i;
		++j;
		while(j != e)
		{
			BOOST_CHECK(*i != *j);
			++j;
		}
		++i;
	}
}

BOOST_AUTO_TEST_SUITE_END()
