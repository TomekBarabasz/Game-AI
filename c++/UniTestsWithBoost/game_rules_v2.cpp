﻿#include "pch.h"
#define BOOST_TEST_MODULE test_module
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>	//required for BOOST_DATA_TEST_CASE
#include <boost/test/data/monomorphic.hpp>	//required for boost::unit_test::data
#include <boost/mpl/list.hpp>

#define _CRTDBG_MAP_ALLOC  
#define UNIT_TEST

#include "../GraWPanaZasadyV2/GraWPanaZasadyV2.cpp"

namespace ut = boost::unit_test;
namespace butd = boost::unit_test::data;
using namespace GraWPanaV2;

BOOST_AUTO_TEST_SUITE(GraWPanaZasadyV2);
BOOST_AUTO_TEST_CASE(makeStackMasks)
{
	uint32_t stack;
	{
	stack = 0b0001;
	auto[allowed_cards, first_allowed_quad, take_cards_mask] = GraWPanaGameRules::makeStackMasks(stack);
	//0b111111111111111111111111
	//0b000000000000000000000000
	BOOST_TEST(0b111111111111111111111110 == allowed_cards);	//allowed cards mask
	BOOST_TEST(0b000000000000000000001111 == first_allowed_quad);	//first allowed quad mask
	BOOST_TEST(0b000000000000000000000000 == take_cards_mask);	//take cards mask
	} {
	stack = 0b1001;
	auto[allowed_cards, first_allowed_quad, take_cards_mask] = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(0b111111111111111111110110 == allowed_cards);	//allowed cards mask
	BOOST_TEST(0b000000000000000000001111 == first_allowed_quad);	//first allowed quad mask
	BOOST_TEST(0b000000000000000000001000 == take_cards_mask);	//take cards mask
	} {
	stack = 0b00010001;
	auto[allowed_cards, first_allowed_quad, take_cards_mask] = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(0b111111111111111111100000 == allowed_cards);	//allowed cards mask
	BOOST_TEST(0b000000000000000011110000 == first_allowed_quad);	//first allowed quad mask
	BOOST_TEST(0b000000000000000000010000 == take_cards_mask);	//take cards mask
	} {
	stack = 0b000100000001;
	auto[allowed_cards, first_allowed_quad, take_cards_mask] = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(0b111111111111111000000000 == allowed_cards);	//allowed cards mask
	BOOST_TEST(0b000000000000111100000000 == first_allowed_quad);	//first allowed quad mask
	BOOST_TEST(0b000000000000000100000000 == take_cards_mask);	//take cards mask
	} {
	stack = 0b100000000001;
	auto[allowed_cards, first_allowed_quad, take_cards_mask] = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(0b111111111111011100000000 == allowed_cards);	//allowed cards mask
	BOOST_TEST(0b000000000000111100000000 == first_allowed_quad);	//first allowed quad mask
	BOOST_TEST(0b000000000000100000000000 == take_cards_mask);	//take cards mask
	} {
	stack = 0b000100010001;
	auto[allowed_cards, first_allowed_quad, take_cards_mask] = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(0b111111111111111000000000 == allowed_cards);	//allowed cards mask
	BOOST_TEST(0b000000000000111100000000 == first_allowed_quad);	//first allowed quad mask
	BOOST_TEST(0b000000000000000100010000 == take_cards_mask);	//take cards mask
	} {
	stack = 0b010100010001;
	auto[allowed_cards, first_allowed_quad, take_cards_mask] = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(0b111111111111101000000000 == allowed_cards);	//allowed cards mask
	BOOST_TEST(0b000000000000111100000000 == first_allowed_quad);	//first allowed quad mask
	BOOST_TEST(0b000000000000010100010000 == take_cards_mask);	//take cards mask
	}
}

struct CreateGameRules
{
	GraWPanaGameRules gr;
	CreateGameRules() : gr(2) {}
};
#define BOOST_TEST_EQ_UINT32_t(x,y) BOOST_TEST((uint32_t)x == (uint32_t)y)

BOOST_FIXTURE_TEST_SUITE(GetPlayerLegalMoves, CreateGameRules);
BOOST_AUTO_TEST_CASE(noop)
{
	GameState s;
	s.current_player = 1;
	s.is_terminal = gr.checkIfTerminal(&s);
	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 1);
	auto [mv,p] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT32_t(mv->operation , Move::noop);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(play_3x9)
{
	GameState s;
	s.is_terminal = false;
	s.current_player = 0;
	s.stack = 0b0001;
	s.hand[0] = 0b1110;
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 2);
	auto [mv1, p1] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT32_t(mv1->operation , Move::play_cards);
	BOOST_TEST_EQ_UINT32_t(mv1->cards , 0b1110);
	auto [mv2, p2] = gr.GetMoveFromList(ml, 1);
	BOOST_TEST_EQ_UINT32_t(mv2->operation , Move::play_cards);
	BOOST_TEST_EQ_UINT32_t(mv2->cards , 0b0010);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(play_1_card_lower)
{
	GameState s;
	s.current_player = 0;
	s.stack = 0b0001;
	s.hand[0] = 0b01010000;
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 1);
	auto [mv, p] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT32_t(mv->operation , Move::play_cards);
	BOOST_TEST_EQ_UINT32_t(mv->cards , 0b00010000);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(play_quad)
{
	GameState s;
	s.current_player = 0;
	s.stack = 0b0001;
	s.hand[0] = 0b11110000;
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 2);
	auto [mv1, p1] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT32_t(mv1->operation , Move::play_cards);
	BOOST_TEST_EQ_UINT32_t(mv1->cards , 0b00010000);

	auto [mv2, p2] = gr.GetMoveFromList(ml, 1);
	BOOST_TEST_EQ_UINT32_t(mv2->operation , Move::play_cards);
	BOOST_TEST_EQ_UINT32_t(mv2->cards , 0b11110000);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(take_1_card)
{
	GameState s;
	s.current_player = 0;
	s.stack = 0b00010001;
	s.hand[0] = 0b1000;
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 1);
	auto [mv, p] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT32_t(mv->operation , Move::take_cards);
	BOOST_TEST_EQ_UINT32_t(mv->cards , 0b00010000);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(take_2_cards)
{
	GameState s;
	s.current_player = 0;
	s.stack = 0b000100100001;
	s.hand[0] = 0b0010;
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 1);
	auto [mv, p] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT32_t(mv->operation , Move::take_cards);
	BOOST_TEST_EQ_UINT32_t(mv->cards , 0b000100100000);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(take_3_cards)
{
	GameState s;
	s.current_player = 0;
	s.stack = 0b010101010001;
	s.hand[0] = 0b0010;
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 1);
	auto [mv, p] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT32_t(mv->operation , Move::take_cards);
	BOOST_TEST_EQ_UINT32_t(mv->cards , 0b010101000000);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(play_cards_or_take_cards)
{
	GameState s;
	s.current_player = 0;
	s.stack = 0b000101010001;
	s.hand[0] = 0b001000000010;
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 2);
	auto [mv1, p1] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT32_t(mv1->operation , Move::play_cards);
	BOOST_TEST_EQ_UINT32_t(mv1->cards , 0b001000000000);
	auto [mv2, p2] = gr.GetMoveFromList(ml, 1);
	BOOST_TEST_EQ_UINT32_t(mv2->operation , Move::take_cards);
	BOOST_TEST_EQ_UINT32_t(mv2->cards , 0b000101010000);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(is_terminal_1)
{
	GameState s;
	s.current_player = 0;
	s.stack = 0b000101010001;
	s.hand[0] = 0b001000000010;
	s.hand[1] = 0;
	s.is_terminal = gr.checkIfTerminal(&s);
	BOOST_TEST(true == gr.IsTerminal(&s));
}
BOOST_AUTO_TEST_CASE(is_terminal_2)
{
	GameState s;
	s.current_player = 1;
	s.stack = 0b000101010001;
	s.hand[1] = 0b001000000010;
	s.hand[0] = 0;
	s.is_terminal = gr.checkIfTerminal(&s);
	BOOST_TEST(true == gr.IsTerminal(&s));
}
BOOST_AUTO_TEST_SUITE_END();
BOOST_FIXTURE_TEST_SUITE(CreateStateFromString, CreateGameRules);
BOOST_AUTO_TEST_CASE(from_wstring_1)
{
	//SUITS ORDER ♦ ♣ ♠ ♥;
	GameState *s = gr.CreateStateFromString(L"S=9♥10♥10♠W♥W♠A♦|P0=9♠10♣|P1=9♣9♦10♦W♣W♦D♥D♠D♣D♦K♥K♠K♣K♦A♥A♠A♣|CP=1");
	BOOST_TEST(1 == gr.GetCurrentPlayer(s));
	BOOST_TEST(0b100000000000001100110001 == uint32_t(s->stack));
	BOOST_TEST(0b01000010 == s->hand[0]);
	BOOST_TEST(0b011111111111110010001100 == s->hand[1]);
	gr.ReleaseGameState(s);
}
BOOST_AUTO_TEST_CASE(from_wstring_2)
{
	//SUITS ORDER ♦ ♣ ♠ ♥;
	GameState *s = gr.CreateStateFromString(L"S=|P0=|P1=9♦10♦W♦D♦K♦A♦|CP=0");
	BOOST_TEST(0 == gr.GetCurrentPlayer(s));
	BOOST_TEST(0 == uint32_t(s->stack));
	BOOST_TEST(0 == s->hand[0]);
	BOOST_TEST(0b100010001000100010001000 == s->hand[1]);
	gr.ReleaseGameState(s);
}
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

