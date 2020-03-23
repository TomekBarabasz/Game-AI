#include "pch.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>	//required for BOOST_DATA_TEST_CASE
#include <GraWZombiakiZasady.h>
#include "GraWZombiakiZasady_ut.h"

namespace ut = boost::unit_test;
using namespace GraWZombiaki;

BOOST_AUTO_TEST_SUITE(Test_Board);
BOOST_AUTO_TEST_CASE(findByType)
{
	const Position exp_pos{ 3,1 };
	Plansza plansza;
	KartaNaPlanszy kot(Player::zombie, KartaNaPlanszy::krystyna);
	KartaNaPlanszy pies(Player::zombie, KartaNaPlanszy::pies);
	plansza[exp_pos] = kot;

	auto [kpos, kfound] = plansza.findByType(KartaNaPlanszy::krystyna);
	BOOST_TEST(true == kfound);
	BOOST_TEST(exp_pos == kpos);

	auto [ppos, pfound] = plansza.findByType(KartaNaPlanszy::pies);
	BOOST_TEST(false == pfound);
}

BOOST_AUTO_TEST_CASE(findByMask)
{
	const Position exp_pos{ 3,1 };
	Plansza plansza;
	KartaNaPlanszy z(Player::zombie, KartaNaPlanszy::zombiak);
	z.bf_jest_bossem = 1;
	plansza[exp_pos] = z;

	auto [kpos, kfound] = plansza.findByMask(z.value);
	BOOST_TEST(true == kfound);
	BOOST_TEST(exp_pos == kpos);

	KartaNaPlanszy z_misiem;
	z_misiem.bf_ma_misia = 1;
	auto [ppos, pfound] = plansza.findByMask(z_misiem.value);
	BOOST_TEST(false == pfound);
}
BOOST_AUTO_TEST_CASE(isEmpty)
{
	const Position pos{ 3,1 };
	Plansza plansza;
	KartaNaPlanszy z(Player::zombie, KartaNaPlanszy::zombiak);
	plansza[pos] = z;
	BOOST_TEST(false == plansza.isEmpty(pos));
	const Position pos1{ 4,1 };
	BOOST_TEST(true == plansza.isEmpty(pos1));
}
BOOST_AUTO_TEST_SUITE_END()
