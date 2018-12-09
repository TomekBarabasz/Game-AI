#include "pch.h"
#define BOOST_TEST_MODULE test_module
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>	//required for BOOST_DATA_TEST_CASE
#include <boost/test/data/monomorphic.hpp>	//required for boost::unit_test::data
#include <boost/mpl/list.hpp>
#include <GraWZombiakiZasady.h>

namespace ut = boost::unit_test;
namespace butd = boost::unit_test::data;
using namespace GraWZombiaki;

BOOST_AUTO_TEST_SUITE(Testy_Plansza);
BOOST_AUTO_TEST_CASE(makeEmpty)
{
	Plansza p;
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Testy_Karty);
BOOST_AUTO_TEST_CASE(makeStackMasks)
{
}
BOOST_AUTO_TEST_SUITE_END()

struct CreateGameRules
{
	GraWZombiakiZasady gr;
};

BOOST_FIXTURE_TEST_SUITE(Testy_Zasady, CreateGameRules);
BOOST_AUTO_TEST_CASE(noop)
{
}
BOOST_AUTO_TEST_SUITE_END()
