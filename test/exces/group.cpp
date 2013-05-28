/**
 *  .file test/exces/group.cpp
 *  .brief Test case for component groups
 *
 *  .author Matus Chochlik
 *
 *  Copyright 2011-2013 Matus Chochlik. Distributed under the Boost
 *  Software License, Version 1.0. (See accompanying file
 *  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE EXCES_Group
#include <boost/test/unit_test.hpp>

#include <exces/group.hpp>

#include <cstring>

struct c1 { int i; };
EXCES_REG_COMPONENT(c1)

struct c2 { int j; };
EXCES_REG_FLYWEIGHT_COMPONENT(c2)

EXCES_REG_GROUP(test_group)

EXCES_REG_COMPONENT_IN_GROUP(c1, test_group)

struct c3 { int k; };
EXCES_REG_COMPONENT_IN_GROUP(c3, test_group)

struct c4 { int l; };
EXCES_REG_FLYWEIGHT_COMPONENT_IN_GROUP(c4, test_group)


BOOST_AUTO_TEST_SUITE(Group)

BOOST_AUTO_TEST_CASE(Group_size)
{
	BOOST_CHECK_EQUAL(
		exces::mp::size<
			exces::components<>
		>::value, 2
	);
	BOOST_CHECK_EQUAL(
		exces::mp::size<
			exces::components<exces::default_group>
		>::value, 2
	);
	BOOST_CHECK_EQUAL(
		exces::mp::size<
			exces::components<EXCES_GROUP_SEL(test_group)>
		>::value, 3
	);
}

BOOST_AUTO_TEST_CASE(Group_component_id)
{
	BOOST_CHECK((
		exces::component_id<c1>::value !=
		exces::component_id<c2>::value
	));
	BOOST_CHECK((
		exces::component_id<c1, EXCES_GROUP_SEL(test_group)>::value !=
		exces::component_id<c3, EXCES_GROUP_SEL(test_group)>::value
	));
	BOOST_CHECK((
		exces::component_id<c1, EXCES_GROUP_SEL(test_group)>::value !=
		exces::component_id<c4, EXCES_GROUP_SEL(test_group)>::value
	));
	BOOST_CHECK((
		exces::component_id<c3, EXCES_GROUP_SEL(test_group)>::value !=
		exces::component_id<c4, EXCES_GROUP_SEL(test_group)>::value
	));
}

BOOST_AUTO_TEST_CASE(Group_component_id2)
{
	BOOST_CHECK((exces::component_id<c1>::value < 2));
	BOOST_CHECK((exces::component_id<c2>::value < 2));

	typedef EXCES_GROUP_SEL(test_group) test_group;
	BOOST_CHECK((exces::component_id<c1, test_group>::value < 3));
	BOOST_CHECK((exces::component_id<c3, test_group>::value < 3));
	BOOST_CHECK((exces::component_id<c4, test_group>::value < 3));
}

BOOST_AUTO_TEST_CASE(Group_flyweight_component)
{
	BOOST_CHECK((!exces::flyweight_component<c1>::value));
	BOOST_CHECK(( exces::flyweight_component<c2>::value));

	typedef EXCES_GROUP_SEL(test_group) test_group;
	BOOST_CHECK((!exces::flyweight_component<c1, test_group>::value));
	BOOST_CHECK((!exces::flyweight_component<c3, test_group>::value));
	BOOST_CHECK(( exces::flyweight_component<c4, test_group>::value));
}

BOOST_AUTO_TEST_CASE(Group_component_name)
{
	BOOST_CHECK((std::strcmp(exces::component_name<c1>::c_str(), "c1") == 0));
	BOOST_CHECK((std::strcmp(exces::component_name<c2>::c_str(), "c2") == 0));

	typedef EXCES_GROUP_SEL(test_group) tg;
	BOOST_CHECK((std::strcmp(exces::component_name<c1, tg>::c_str(), "c1") == 0));
	BOOST_CHECK((std::strcmp(exces::component_name<c3, tg>::c_str(), "c3") == 0));
	BOOST_CHECK((std::strcmp(exces::component_name<c4, tg>::c_str(), "c4") == 0));
}

BOOST_AUTO_TEST_SUITE_END()
