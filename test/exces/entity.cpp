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
#define BOOST_TEST_MODULE EXCES_Angle
#include <boost/test/unit_test.hpp>

#include <exces/entity.hpp>

BOOST_AUTO_TEST_SUITE(Entity)

BOOST_AUTO_TEST_CASE(Entity_default_construction)
{
	exces::entity<>::type e;
	(void)(e);
}

BOOST_AUTO_TEST_SUITE_END()
