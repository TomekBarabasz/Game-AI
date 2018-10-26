#include "pch.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>	//required for BOOST_DATA_TEST_CASE
#include <boost/test/data/monomorphic.hpp>	//required for boost::unit_test::data
#include <boost/mpl/list.hpp>

#define _CRTDBG_MAP_ALLOC  
#define UNIT_TEST

#include "GraWPanaZasady/GraWPanaZasady.cpp"
//#include "GraWPanaZasadyV2/GraWPanaZasadyV2.cpp"

namespace but = boost::unit_test;
namespace butd = boost::unit_test::data;

struct TestSuiteFixture
{
	TestSuiteFixture() { }//BOOST_TEST_MESSAGE("suite setup"); }
	~TestSuiteFixture() { }//BOOST_TEST_MESSAGE("suite teardown");
};
struct TestCaseFixture
{
	TestCaseFixture() { }//BOOST_TEST_MESSAGE("testcase setup"); }
	~TestCaseFixture() { }//BOOST_TEST_MESSAGE("testcase teardown"); }
};
BOOST_AUTO_TEST_SUITE(GraWPanaZasadyV1, *but::label("V1")*but::fixture<TestSuiteFixture>());
//BOOST_FIXTURE_TEST_SUITE(GraWPanaZasadyV1, TestSuiteFixture);

BOOST_AUTO_TEST_CASE(test_ref_cnt_after_create,*but::label("Memory")*but::label("GameState")*but::fixture<TestSuiteFixture>()*but::fixture<TestCaseFixture>())
//BOOST_FIXTURE_TEST_CASE(test_ref_cnt_after_create, TestCaseFixture)
{
	BOOST_TEST_MESSAGE("running test_ref_cnt_after_create");
	GraWPanaV1::GameState gs(2);
	BOOST_TEST_MESSAGE("refcnt value " << gs.m_refCnt << " has been sensed");
	BOOST_TEST(gs.m_refCnt == 1);
}
BOOST_AUTO_TEST_SUITE_END();

#if 0
BOOST_FIXTURE_TEST_SUITE(GraWPanaZasadyV2, TestSuiteFixture);
const int NumPlayers[] = { 2,3,4 };
BOOST_DATA_TEST_CASE(test_ref_cnt_after_create, butd::make(NumPlayers), num_players)
{
	BOOST_TEST_MESSAGE("running test_ref_cnt_after_create witn NumPlayers = " << num_players);
	GraWPanaV2::GameState gs(num_players);
	BOOST_TEST(gs.m_refCnt == 1);
}
BOOST_AUTO_TEST_SUITE_END();


BOOST_AUTO_TEST_SUITE(GraWPanaZasadyV1V2Combined);
using TestTypes = boost::mpl::list<GraWPanaV1::GameState, GraWPanaV2::GameState>;
BOOST_AUTO_TEST_CASE_TEMPLATE(test_ref_cnt_after_create, T, TestTypes)
{
	BOOST_TEST_MESSAGE("running test_ref_cnt_after_create for " << typeid(T).name());
	T gs(2);
	BOOST_TEST(gs.m_refCnt == 1);
}
BOOST_AUTO_TEST_CASE_TEMPLATE(test_ref_cnt_after_create_2, T, 
							BOOST_IDENTITY_TYPE((boost::mpl::list<GraWPanaV1::GameState, GraWPanaV2::GameState>)))
{
	BOOST_TEST_MESSAGE("running test_ref_cnt_after_create for " << typeid(T).name());
	T gs(2);
	BOOST_TEST(gs.m_refCnt == 1);
}
BOOST_AUTO_TEST_SUITE_END();

#endif