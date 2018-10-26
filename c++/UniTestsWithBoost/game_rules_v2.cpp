#include "pch.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>	//required for BOOST_DATA_TEST_CASE
#include <boost/test/data/monomorphic.hpp>	//required for boost::unit_test::data
#include <boost/mpl/list.hpp>

#define _CRTDBG_MAP_ALLOC  
#define UNIT_TEST

#include "GraWPanaZasadyV2/GraWPanaZasadyV2.cpp"

namespace but = boost::unit_test;
namespace butd = boost::unit_test::data;
using namespace GraWPanaV2;

BOOST_AUTO_TEST_SUITE(GraWPanaZasadyV2);
BOOST_AUTO_TEST_CASE(makeStackMasks)
{
	uint32_t stack;
	stack = 0b0001;
	auto masks = GraWPanaGameRules::makeStackMasks(stack);
	//0b111111111111111111111111
	//0b000000000000000000000000
	BOOST_TEST(0b111111111111111111111110 == masks.get<0>());	//allowed cards mask
	BOOST_TEST(0b000000000000000000001111 == masks.get<1>());	//first allowed quad mask
	BOOST_TEST(0b000000000000000000000000 == masks.get<2>());	//take cards mask

	stack = 0b1001;
	masks = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(0b111111111111111111110110 == masks.get<0>());	//allowed cards mask
	BOOST_TEST(0b000000000000000000001111 == masks.get<1>());	//first allowed quad mask
	BOOST_TEST(0b000000000000000000001000 == masks.get<2>());	//take cards mask

	stack = 0b00010001;
	masks = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(0b111111111111111111100000 == masks.get<0>());	//allowed cards mask
	BOOST_TEST(0b000000000000000011110000 == masks.get<1>());	//first allowed quad mask
	BOOST_TEST(0b000000000000000000010000 == masks.get<2>());	//take cards mask

	stack = 0b000100000001;
	masks = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(0b111111111111111000000000 == masks.get<0>());	//allowed cards mask
	BOOST_TEST(0b000000000000111100000000 == masks.get<1>());	//first allowed quad mask
	BOOST_TEST(0b000000000000000100000000 == masks.get<2>());	//take cards mask

	stack = 0b100000000001;
	masks = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(0b111111111111011100000000 == masks.get<0>());	//allowed cards mask
	BOOST_TEST(0b000000000000111100000000 == masks.get<1>());	//first allowed quad mask
	BOOST_TEST(0b000000000000100000000000 == masks.get<2>());	//take cards mask

	stack = 0b000100010001;
	masks = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(0b111111111111111000000000 == masks.get<0>());	//allowed cards mask
	BOOST_TEST(0b000000000000111100000000 == masks.get<1>());	//first allowed quad mask
	BOOST_TEST(0b000000000000000100010000 == masks.get<2>());	//take cards mask

	stack = 0b010100010001;
	masks = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(0b111111111111101000000000 == masks.get<0>());	//allowed cards mask
	BOOST_TEST(0b000000000000111100000000 == masks.get<1>());	//first allowed quad mask
	BOOST_TEST(0b000000000000010100010000 == masks.get<2>());	//take cards mask
}

struct CreateGameRules
{
	GraWPanaV2::GraWPanaGameRules gr;
	CreateGameRules() : gr(2) {}
};

BOOST_FIXTURE_TEST_SUITE(GetPlayerLegalMoves, CreateGameRules);
BOOST_AUTO_TEST_CASE(noop)
{
	GameState s;
	s.current_player = 1;
	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 1);
	BOOST_TEST(gr.GetMoveFromList(ml,0)->operation == Move::noop);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(play_3x9)
{
	GameState s;
	s.current_player = 0;
	s.stack = 0b0001;
	s.hand[0] = 0b1110;
	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 2);
	const Move &mv1 = *gr.GetMoveFromList(ml, 0);
	BOOST_TEST(mv1.operation == Move::play_cards);
	BOOST_TEST(mv1.cards == 0b1110);
	const Move &mv2 = *gr.GetMoveFromList(ml, 1);
	BOOST_TEST(mv2.operation == Move::play_cards);
	BOOST_TEST(mv2.cards == 0b0010);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(play_1_card_lower)
{
	GameState s;
	s.current_player = 0;
	s.stack = 0b0001;
	s.hand[0] = 0b01010000;
	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 1);
	const Move &mv = *gr.GetMoveFromList(ml, 0);
	BOOST_TEST(mv.operation == Move::play_cards);
	BOOST_TEST(mv.cards == 0b00010000);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(play_quad)
{
	GameState s;
	s.current_player = 0;
	s.stack = 0b0001;
	s.hand[0] = 0b11110000;
	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 2);
	const Move &mv1 = *gr.GetMoveFromList(ml, 0);
	BOOST_TEST(mv1.operation == Move::play_cards);
	BOOST_TEST(mv1.cards == 0b00010000);

	const Move &mv2 = *gr.GetMoveFromList(ml, 1);
	BOOST_TEST(mv2.operation == Move::play_cards);
	BOOST_TEST(mv2.cards == 0b11110000);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(take_1_card)
{
	GameState s;
	s.current_player = 0;
	s.stack = 0b00010001;
	s.hand[0] = 0b1000;
	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 1);
	const Move &mv = *gr.GetMoveFromList(ml, 0);
	BOOST_TEST(mv.operation == Move::take_cards);
	BOOST_TEST(mv.cards == 0b00010000);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(take_2_cards)
{
	GameState s;
	s.current_player = 0;
	s.stack = 0b000100100001;
	s.hand[0] = 0b0010;
	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 1);
	const Move &mv = *gr.GetMoveFromList(ml, 0);
	BOOST_TEST(mv.operation == Move::take_cards);
	BOOST_TEST(mv.cards == 0b000100100000);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(take_3_cards)
{
	GameState s;
	s.current_player = 0;
	s.stack = 0b010101010001;
	s.hand[0] = 0b0010;
	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 1);
	const Move &mv = *gr.GetMoveFromList(ml, 0);
	BOOST_TEST(mv.operation == Move::take_cards);
	BOOST_TEST(mv.cards == 0b010101000000);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(play_cards_or_take_cards)
{
	GameState s;
	s.current_player = 0;
	s.stack = 0b000101010001;
	s.hand[0] = 0b001000000010;
	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 2);
	const Move &mv1 = *gr.GetMoveFromList(ml, 0);
	BOOST_TEST(mv1.operation == Move::play_cards);
	BOOST_TEST(mv1.cards == 0b001000000000);
	const Move &mv2 = *gr.GetMoveFromList(ml, 1);
	BOOST_TEST(mv2.operation == Move::take_cards);
	BOOST_TEST(mv2.cards == 0b000101010000);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

