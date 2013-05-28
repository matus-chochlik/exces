/**
 *  .file test/exces/metaprog.cpp
 *  .brief Test case for meta-programming utilities
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

#include <exces/metaprog.hpp>

BOOST_AUTO_TEST_SUITE(MetaProg)

BOOST_AUTO_TEST_CASE(MetaProg_typelist_construction)
{
	exces::typelist<> t0;
	exces::typelist<int> t1;
	exces::typelist<short, long> t2;
	exces::typelist<bool, int, char> t3;
	exces::typelist<bool, int, float, double, char> t5;
}

BOOST_AUTO_TEST_CASE(MetaProg_tuple_construction)
{
	exces::tuple<int> t1(1);
	exces::tuple<bool, short> t2(0, 1);
	exces::tuple<int, short, long> t3(0, 1, 2);
	exces::tuple<int, short, long, float> t4(0, 1, 2, 3.4f);
	exces::tuple<int, short, long, float, double> t5(0, 1, 2, 3.4f, 56.789);
}

BOOST_AUTO_TEST_CASE(MetaProg_make_tuple)
{
	auto t1 = exces::make_tuple(1);
	auto t2 = exces::make_tuple(1, "23");
	auto t3 = exces::make_tuple(1u, "2", 34.56);
	auto t4 = exces::make_tuple(1u, "2", 34.56, 789);
	auto t5 = exces::make_tuple(true, 2, 3l, 4.5f, 567.89);
}

BOOST_AUTO_TEST_CASE(MetaProg_tuple_get)
{
	auto t = exces::make_tuple(true, 2, 3l, 4.5f, 567.89);
	BOOST_CHECK_EQUAL(exces::get<0>(t), true);
	BOOST_CHECK_EQUAL(exces::get<1>(t), 2);
	BOOST_CHECK_EQUAL(exces::get<2>(t), 3l);
	BOOST_CHECK_EQUAL(exces::get<3>(t), 4.5f);
	BOOST_CHECK_EQUAL(exces::get<4>(t), 567.89);
}

BOOST_AUTO_TEST_SUITE_END()
