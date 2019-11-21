#include "pch.h"
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

uint64_t mkm(std::initializer_list<int> il)
{
	uint64_t hand = 0;
	auto shift = (il.size() - 1) * 2;
	for (auto v : il)
	{
		hand |= v << shift;
		shift -= 2;
	}
	return hand;
}
uint64_t mkmb(std::initializer_list<int> il)
{
	uint64_t hand = 0;
	auto shift = (il.size() - 1) * 2;
	for (uint64_t v : il)
	{
		v = 2*(v / 10) + v % 10;
		hand |= v << shift;
		shift -= 2;
	}
	return hand;
}
uint64_t ones(int pos)
{
	uint64_t ret = ~0;
	return (ret << (2 * pos)) & ~(~0ull<<48);
}
BOOST_AUTO_TEST_SUITE(GraWPanaZasadyV2);
BOOST_AUTO_TEST_CASE(makeHand)
{
	BOOST_TEST(0b001000010011 == mkm ( {0, 2,0, 1,0, 3} ));
	BOOST_TEST(0b001000010011 == mkmb( {0,10,0,01,0,11} ));
	BOOST_TEST(mkmb({ 11,00,00,00,10,00,00,00,01,00,00,00,11,00,00,00,10,00,00,00,01,00,00,00 }) == 0b110000001000000001000000110000001000000001000000);
	BOOST_TEST(0b111111111111111111111111111111111111111111111111 == ones(0));
	BOOST_TEST(0b111111111111111111111111111111111111111111111100 == ones(1));
	BOOST_TEST(0b111111111111111111111111111111111111111111110000 == ones(2));
	BOOST_TEST(0b111111111111111111111111111111111111111111000000 == ones(3));
	BOOST_TEST(0b111111111111111111111111111111111111111100000000 == ones(4));
}
BOOST_AUTO_TEST_CASE(makeStackMasks)
{
	uint64_t stack;
	{
	stack = mkm({0,0,0b11});
	auto[allowed_cards, first_allowed_quad, take_cards_mask] = GraWPanaGameRules::makeStackMasks(stack);
	//0b111111111111111111111111
	//0b000000000000000000000000
	BOOST_TEST(ones(1)		== allowed_cards);		//allowed cards mask
	BOOST_TEST(mkmb({ 11,11,11,11 }) == first_allowed_quad);	//first allowed quad mask
	BOOST_TEST(0			== take_cards_mask);	//take cards mask
	} {
	stack = mkmb({11,0,0,11});
	auto[allowed_cards, first_allowed_quad, take_cards_mask] = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(ones(4) + mkmb({00,11,11,00}) == allowed_cards);	//allowed cards mask
	BOOST_TEST(mkmb({ 11,11,11,11 }) == first_allowed_quad);	//first allowed quad mask
	BOOST_TEST(mkmb({ 11,00,00,00 }) == take_cards_mask);		//take cards mask
	} {
	stack = mkmb({0,0,0,11,0,0,0,11});
	auto[allowed_cards, first_allowed_quad, take_cards_mask] = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(ones(5) == allowed_cards);							//allowed cards mask		
	BOOST_TEST(mkmb({11,11,11,11,0,0,0,0}) == first_allowed_quad);	//first allowed quad mask
	BOOST_TEST(mkmb({00,00,00,11,0,0,0,0}) == take_cards_mask);		//take cards mask
	} {
	stack = mkmb({0,0,0,11,0,0,0,0,0,0,0,11});
	auto[allowed_cards, first_allowed_quad, take_cards_mask] = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(ones(9) == allowed_cards);									//allowed cards mask
	BOOST_TEST(mkmb({11,11,11,11, 0,0,0,0, 0,0,0,0}) == first_allowed_quad);//first allowed quad mask
	BOOST_TEST(mkmb({ 0,0,0,11,0,0,0,0,0,0,0,0 }) == take_cards_mask);		//take cards mask
	} {
	stack = mkmb({11,0,0,0, 0,0,0,0, 0,0,0,11});
	auto[allowed_cards, first_allowed_quad, take_cards_mask] = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(ones(12)+mkmb({00,11,11,11, 0,0,0,0, 0,0,0,0}) == allowed_cards);//allowed cards mask
	BOOST_TEST(mkmb({11,11,11,11, 0,0,0,0, 0,0,0,0}) == first_allowed_quad);//first allowed quad mask
	BOOST_TEST(mkmb({11,00,00,00, 0,0,0,0, 0,0,0,0}) == take_cards_mask);	//take cards mask
	} {
	stack = mkmb({11,0,0,0,11,0,0,0,11});
	auto[allowed_cards, first_allowed_quad, take_cards_mask] = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(ones(9) == allowed_cards);									//allowed cards mask
	BOOST_TEST(mkmb({11,11,11,11, 0,0,0,00, 0,0,0,0}) == first_allowed_quad);//first allowed quad mask
	BOOST_TEST(mkmb({00,00,00,11, 0,0,0,11, 0,0,0,0}) == take_cards_mask);	//take cards mask
	} {
	stack = mkmb({11,0,11,0,0,0,11,0,0,0,11});
	auto[allowed_cards, first_allowed_quad, take_cards_mask] = GraWPanaGameRules::makeStackMasks(stack);
	BOOST_TEST(ones(11) + mkmb({0,11,0,0,0,0,0,0,0,0,0}) == allowed_cards);	//allowed cards mask
	BOOST_TEST(mkmb({ 11,11,11,11, 0,0,0,00, 0,0,0,0}) == first_allowed_quad);//first allowed quad mask
	BOOST_TEST(mkmb({ 00,11,00,11, 0,0,0,11, 0,0,0,0}) == take_cards_mask);	//take cards mask
	}
}
BOOST_AUTO_TEST_CASE(make11)
{
	BOOST_TEST(0b11001100 == GraWPanaGameRules::make11(0b10000100));
	BOOST_TEST(0b11001100 == GraWPanaGameRules::make11(0b01001000));
	BOOST_TEST(0b11001100 == GraWPanaGameRules::make11(0b11001100));
}
struct CreateGameRules
{
	GraWPanaGameRules gr;
	CreateGameRules() : gr(2) {}
};
#define BOOST_TEST_EQ_UINT64_t(x,y) BOOST_TEST((uint64_t)x == (uint64_t)y)

BOOST_FIXTURE_TEST_SUITE(GetPlayerLegalMoves, CreateGameRules);
BOOST_AUTO_TEST_CASE(noop)
{
	GameState s;
	s.current_player = 1;
	s.is_terminal = gr.checkIfTerminal(&s);
	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 1);
	auto [mv,p] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT64_t(mv->operation , Move::noop);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(play_3x9)
{
	GameState s;
	s.is_terminal = false;
	s.current_player = 0;
	s.stack =   mkmb({00,00,00,00,00,00,11});
	s.hand[0] = mkmb({00,00,00,11,11,11,00});
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 2);
	auto [mv1, p1] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT64_t(mv1->operation , Move::play_cards);
	BOOST_TEST_EQ_UINT64_t(mv1->cards , mkmb({11,11,11,00}));
	auto [mv2, p2] = gr.GetMoveFromList(ml, 1);
	BOOST_TEST_EQ_UINT64_t(mv2->operation , Move::play_cards);
	BOOST_TEST_EQ_UINT64_t(mv2->cards , mkmb({00,00,11,00}));
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(play_1_card_lower)
{
	GameState s;
	s.current_player = 0;
	s.stack = mkmb({00,00,00,11});
	s.hand[0] = mkmb({ 00,11,00,11,00,00,00,00 });
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 1);
	auto [mv, p] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT64_t(mv->operation , Move::play_cards);
	BOOST_TEST_EQ_UINT64_t(mv->cards, mkmb({ 00,00,00,11,00,00,00,00 }));
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(play_quad)
{
	GameState s;
	s.current_player = 0;
	s.stack = mkmb({00,00,00,11});
	s.hand[0] = mkmb({11,11,11,11,00,00,00,00});
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 2);
	auto [mv1, p1] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT64_t(mv1->operation , Move::play_cards);
	BOOST_TEST_EQ_UINT64_t(mv1->cards , mkmb({ 00,00,00,11,00,00,00,00 }));

	auto [mv2, p2] = gr.GetMoveFromList(ml, 1);
	BOOST_TEST_EQ_UINT64_t(mv2->operation , Move::play_cards);
	BOOST_TEST_EQ_UINT64_t(mv2->cards , mkmb({ 11,11,11,11,00,00,00,00 }));
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(take_1_card)
{
	GameState s;
	s.current_player = 0;
	s.stack = mkmb({00,00,00,11,00,00,00,11});
	s.hand[0] = mkmb({11,00,00,00});
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 1);
	auto [mv, p] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT64_t(mv->operation , Move::take_cards);
	BOOST_TEST_EQ_UINT64_t(mv->cards , mkmb({ 00,00,00,11,00,00,00,00 }));
	BOOST_TEST(p == 1.0);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(take_2_cards)
{
	//SUITS ORDER ♦ ♣ ♠ ♥;
	GameState s;
	s.current_player = 0;
	s.stack = mkmb({00,00,00,11, 00,00,11,00, 00,00,00,11});

	BOOST_TEST_EQ_UINT64_t(s.stack , GraWPanaGameRules::handFromString(L"W.3♥10.3♠9.3♥"));
	s.hand[0] = mkmb({00,00,11,00});
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 1);
	auto [mv, p] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT64_t(mv->operation , Move::take_cards);
	BOOST_TEST_EQ_UINT64_t(mv->cards , mkmb({ 00,00,00,11, 00,00,11,00, 00,00,00,00 }));
	BOOST_TEST(p == 1.0);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(take_3_cards)
{
	GameState s;
	s.current_player = 0;
	s.stack = mkmb({ 00,11,00,11, 00,11,00,11, 00,00,00,11 });
	s.hand[0] = mkmb({00,00,11,00});
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 1);
	auto [mv, p] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT64_t(mv->operation , Move::take_cards);
	BOOST_TEST_EQ_UINT64_t(mv->cards , mkmb({ 00,11,00,11, 00,11,00,00, 00,00,00,00 }));
	BOOST_TEST(p == 1.0);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(play_cards_or_take_cards)
{
	GameState s;
	s.current_player = 0;
	s.stack = mkmb({00,00,00,11, 00,11,00,11, 00,00,00,11});
	s.hand[0] = mkmb({00,00,11,00, 00,00,00,00, 00,00,11,00});
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList *ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 2);
	auto [mv1, p1] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT64_t(mv1->operation , Move::play_cards);
	BOOST_TEST_EQ_UINT64_t(mv1->cards , mkmb({ 00,00,11,00, 00,00,00,00, 00,00,00,00 }));
	auto [mv2, p2] = gr.GetMoveFromList(ml, 1);
	BOOST_TEST_EQ_UINT64_t(mv2->operation , Move::take_cards);
	BOOST_TEST_EQ_UINT64_t(mv2->cards , mkmb({ 00,00,00,11, 00,11,00,11, 00,00,00,00 }));
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(play_cards_or_take_cards_iig)
{
	GameState s;
	s.current_player = 0;
	s.stack = mkmb({ 00,00,00,11, 00,11,00,11, 00,00,00,11 });
	s.hand[0] = mkmb({ 00,00,10,00, 00,00,00,00, 00,00,01,00 });
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList* ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 2);
	auto [mv1, p1] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT64_t(mv1->operation, Move::play_cards);
	BOOST_TEST_EQ_UINT64_t(mv1->cards, mkmb({ 00,00,11,00, 00,00,00,00, 00,00,00,00 }));
	BOOST_TEST(p1 == 0.5f);
	
	auto [mv2, p2] = gr.GetMoveFromList(ml, 1);
	BOOST_TEST_EQ_UINT64_t(mv2->operation, Move::take_cards);
	BOOST_TEST_EQ_UINT64_t(mv2->cards, mkmb({ 00,00,00,11, 00,11,00,11, 00,00,00,00 }));
	BOOST_TEST(p2 == 1.0);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(play_1_card_iig)
{
	GameState s;
	s.current_player = 0;
	s.stack = mkmb({ 00,00,00,11 });
	s.hand[0] = mkmb({ 10,00,00,00, 00,00,00,01, 00,00,00,00 });
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList* ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 2);
	auto [mv, p] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT64_t(mv->operation, Move::play_cards);
	BOOST_TEST_EQ_UINT64_t(mv->cards, mkmb({ 00,00,00,00, 00,00,00,11, 00,00,00,00 }));
	BOOST_TEST(p == 1.0f/3.0f);
	
	auto [mv1, p1] = gr.GetMoveFromList(ml, 1);
	BOOST_TEST_EQ_UINT64_t(mv1->operation, Move::play_cards);
	BOOST_TEST_EQ_UINT64_t(mv1->cards, mkmb({ 11,00,00,00, 00,00,00,00, 00,00,00,00 }));
	BOOST_TEST(p1 == 0.5f);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(play_quad_iig)
{
	GameState s;
	s.current_player = 0;
	s.stack = mkmb({ 00,00,00,11 });
	s.hand[0] = mkmb({ 11,10,01,10,00,00,00,00 });
	s.is_terminal = gr.checkIfTerminal(&s);

	MoveList* ml = gr.GetPlayerLegalMoves(&s, 0);
	BOOST_TEST(gr.GetNumMoves(ml) == 2);
	auto [mv1, p1] = gr.GetMoveFromList(ml, 0);
	BOOST_TEST_EQ_UINT64_t(mv1->operation, Move::play_cards);
	BOOST_TEST_EQ_UINT64_t(mv1->cards, mkmb({ 00,00,00,11,00,00,00,00 }));
	BOOST_TEST(p1 == 0.5f);
	
	auto [mv2, p2] = gr.GetMoveFromList(ml, 1);
	BOOST_TEST_EQ_UINT64_t(mv2->operation, Move::play_cards);
	BOOST_TEST_EQ_UINT64_t(mv2->cards, mkmb({ 11,11,11,11,00,00,00,00 }));
	BOOST_TEST(p2 == 1.0f/3.0f);
	gr.ReleaseMoveList(ml);
}
BOOST_AUTO_TEST_CASE(is_terminal_1)
{
	//SUITS ORDER ♦ ♣ ♠ ♥;
	GameState s;
	s.current_player = 0;
	s.stack = GraWPanaGameRules::handFromString(L"10.2♥9.2♦9.3♣9.1♥");	//mkmb({00,01,01,01,00,01});
	s.hand[0] = GraWPanaGameRules::handFromString(L"10.1♥9.2♥");			//mkmb({00,10,00,00,00,10});
	s.hand[1] = 0;
	s.is_terminal = gr.checkIfTerminal(&s);
	BOOST_TEST(true == gr.IsTerminal(&s));
}
BOOST_AUTO_TEST_CASE(is_terminal_2)
{
	GameState s;
	s.current_player = 1;
	s.stack = mkmb({ 00,01,01,01,00,01 });
	s.hand[1] = mkmb({ 00,10,00,00,00,10 });
	s.hand[0] = 0;
	s.is_terminal = gr.checkIfTerminal(&s);
	BOOST_TEST(true == gr.IsTerminal(&s));
}
BOOST_AUTO_TEST_SUITE_END();
BOOST_FIXTURE_TEST_SUITE(CreateStateFromString, CreateGameRules);
BOOST_AUTO_TEST_CASE(from_wstring_1)
{
	//SUITS ORDER ♦ ♣ ♠ ♥;
	GameState *s = gr.CreateStateFromString(L"S=9.1♥10.2♥10.3♠W.1♥W.2♠A.3♦|P0=9.1♠10.2♣|P1=9.1♣9.2♦10.3♦W.1♣W.2♦D.3♥D.1♠D.2♣D.3♦K.1♥K.2♠K.3♣K.1♦A.2♥A.3♠A.1♣|CP=1");
	BOOST_TEST(1 == gr.GetCurrentPlayer(s));
	BOOST_TEST(mkmb({ 11,00,00,00,00,00,00,00,00,00,00,00,00,00,10,01,00,00,11,10,00,00,00,01 }) == uint64_t(s->stack));
	BOOST_TEST(mkmb({ 00,10,00,00,00,00,01,00 }) == s->hand[0]);
	BOOST_TEST(mkmb({ 00,01,11,10,01,11,10,01,11,10,01,11,10,01,00,00,11,00,00,00,10,01,00,00 }) == s->hand[1]);
	gr.ReleaseGameState(s);
}
BOOST_AUTO_TEST_CASE(from_wstring_2)
{
	//SUITS ORDER ♦ ♣ ♠ ♥;
	GameState *s = gr.CreateStateFromString(L"S=|P0=|P1=9.1♦10.2♦W.3♦D.1♦K.2♦A.3♦|CP=0");
	BOOST_TEST(0 == gr.GetCurrentPlayer(s));
	BOOST_TEST(0 == uint32_t(s->stack));
	BOOST_TEST(0 == s->hand[0]);
	BOOST_TEST(0b110000001000000001000000110000001000000001000000 == s->hand[1]);
	BOOST_TEST(mkmb({ 11,00,00,00,10,00,00,00,01,00,00,00,11,00,00,00,10,00,00,00,01,00,00,00 }) == s->hand[1]);
	gr.ReleaseGameState(s);
}
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();

