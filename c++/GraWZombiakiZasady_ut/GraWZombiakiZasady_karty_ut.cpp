#include "pch.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>	//required for BOOST_DATA_TEST_CASE
#include <GraWZombiakiZasady.h>
#include "GraWZombiakiZasady_ut.h"
#include "Karty_if.h"

namespace ut = boost::unit_test;
using namespace GraWZombiaki;

BOOST_AUTO_TEST_SUITE(Test_Cards);
BOOST_AUTO_TEST_CASE(constructor)
{
	ZombieCard k(swit);

	BOOST_TEST_EQ_UINT8(k.getType(), TypKarty::akcja);
	BOOST_TEST_EQ_UINT8(k.typ, TypKarty::akcja);
	boost_test_eq<uint8_t>(k.typ, TypKarty::akcja);
	BOOST_TEST_EQ_UINT8(k.podtyp, AkcjaZombie::swit);
}
BOOST_AUTO_TEST_CASE(na_planszy)
{
	KartaNaPlanszy k;
	BOOST_TEST_EQ_UINT8(sizeof(k), sizeof(uint16_t));
}

BOOST_AUTO_TEST_CASE(zombiak_get_valid_moves)
{
	Zombiak_If zombiak;
	GameState gs;
	gs.current_player = Player::zombie;
	gs.zombieDeck.cards[0] = ZombieCard(zombiak_1);
	MoveList_t moves;
	
	zombiak.getValidUsage(&gs, 0, moves);
	BOOST_TEST(3 == moves.size());
	BOOST_TEST(true == findMove(moves, makeMove<Mv_PlayCard>(Position{0,0},0)));
	BOOST_TEST(true == findMove(moves, makeMove<Mv_PlayCard>(Position{0,1},0)));
	BOOST_TEST(true == findMove(moves, makeMove<Mv_PlayCard>(Position{0,2},0)));
}

BOOST_AUTO_TEST_CASE(zombiak_get_valid_moves_1)
{
	Zombiak_If zombiak;
	GameState gs;
	gs.current_player = Player::zombie;
	gs.zombieDeck.cards[0] = ZombieCard{ zombiak_1 };
	gs.plansza[{0, 1}] = KartaNaPlanszy{ Player::zombie, KartaNaPlanszy::zombiak };

	MoveList_t moves;
	zombiak.getValidUsage(&gs, 0, moves);
	BOOST_TEST(true == findMove(moves, makeMove<Mv_PlayCard>(Position{ 0,0 }, 0)));
	BOOST_TEST(true == findMove(moves, makeMove<Mv_PlayCard>(Position{ 0,2 }, 0)));
	BOOST_TEST(true == moves.empty());
}

BOOST_AUTO_TEST_CASE(cat_get_valid_moves)
{
	Kot_If zombiak;
	GameState gs;
	gs.current_player = Player::zombie;
	gs.zombieDeck.cards[0] = ZombieCard(kot);
	MoveList_t moves;

	gs.plansza[{0, 0}] = KartaNaPlanszy{ Player::zombie, KartaNaPlanszy::zombiak };
	
	zombiak.getValidUsage(&gs, 0, moves);
	BOOST_TEST(true == findMove(moves, makeMove<Mv_PlayCard>(Position{ 0,1 }, 0)));
	BOOST_TEST(true == findMove(moves, makeMove<Mv_PlayCard>(Position{ 0,2 }, 0)));
	BOOST_TEST(true == moves.empty());
}

BOOST_AUTO_TEST_CASE(cat_play)
{
	Kot_If kot;
	GameState gs;
	gs.current_player = Player::zombie;
	gs.zombieDeck.cards[0] = ZombieCard(ObiektZombie::kot);
	
	kot.place(&gs, { 0,0 }, 0);
	BOOST_TEST_EQ_UINT8(KartaNaPlanszy::kot, (gs.plansza[{0, 0}].bf_typ));
	BOOST_TEST_EQ_UINT8(KartaNaPlanszy::puste_miejsce, gs.plansza.karta_pod_kotem.bf_typ);
}

BOOST_AUTO_TEST_CASE(cat_move)
{
	Kot_If kot;
	GameState gs;
	gs.current_player = Player::zombie;

	gs.plansza[{1, 1}] = KartaNaPlanszy{ Player::zombie, KartaNaPlanszy::zombiak };
	gs.plansza[{0, 0}] = KartaNaPlanszy{ Player::zombie, KartaNaPlanszy::kot };
	gs.plansza.karta_pod_kotem = KartaNaPlanszy { Player::zombie, KartaNaPlanszy::krystyna };
	
	kot.move(&gs, { 0,0 }, {1,1});
	
	BOOST_TEST_EQ_UINT8(KartaNaPlanszy::kot, (gs.plansza[{1, 1}].bf_typ));
	BOOST_TEST_EQ_UINT8(KartaNaPlanszy::krystyna, (gs.plansza[{0, 0}].bf_typ));
	BOOST_TEST_EQ_UINT8(KartaNaPlanszy::zombiak, gs.plansza.karta_pod_kotem.bf_typ);
}

BOOST_AUTO_TEST_SUITE_END()
