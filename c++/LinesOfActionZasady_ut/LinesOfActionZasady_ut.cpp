#define BOOST_TEST_MODULE test_module
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>	//required for BOOST_DATA_TEST_CASE
#include <boost/test/data/monomorphic.hpp>	//required for boost::unit_test::data
#include <boost/mpl/list.hpp>

#define _CRTDBG_MAP_ALLOC  
#define UNIT_TEST

#include "../LinesOfActionZasady/LinesOfActionZasady.cpp"

/*
63 62 61 60 59 58 57 56
55 54 53 52 51 50 49 48
47 46 45 44 43 42 41 40
39 38 37 36 35 34 33 32
31 30 29 28 27 26 25 24
23 22 21 20 19 18 17 16
15 14 13 12 11 10 09 08
07 06 05 04 03 02 01 00 */
namespace ut = boost::unit_test;
namespace butd = boost::unit_test::data;
using namespace LinesOfAction;

#define BOOST_TEST_EQ_UINT64_t(x,y) BOOST_TEST((uint64_t)x == (uint64_t)y)

BOOST_AUTO_TEST_SUITE(LinesOfAction_GameState)
BOOST_AUTO_TEST_CASE(create)
{
	LinesOfActionGameRules loa;
}
static const long	  Positions[]	   = { 0,		1,		7,		8,			9,			31,				40,					44,					56,						63};
static const uint64_t NeighbourMasks[] = { 0x0302,	0x0705,	0xC040,	0x030203,	0x070507,	0xC040C00000,	0x03020300000000u,	0x38283800000000,	0x0203000000000000u,	0x40C0000000000000};
BOOST_DATA_TEST_CASE(getNeighbourMask, butd::make(Positions) ^ butd::make(NeighbourMasks), position, mask)
{
	BOOST_TEST_EQ_UINT64_t(mask, GameState::getNeighbourMask(position));
}

static const uint64_t Boards[] = {0x1,	0x3,	0x5,	0x020701,	0x010704,	0x07,	0x040404,	0x040201,	0x10040201 };
static const bool IsTerminal[] = {true, true,	false,	true,		true,		true,	true,		true,		false		};
BOOST_DATA_TEST_CASE(calcIfTerminal, butd::make(Boards) ^ butd::make(IsTerminal), board, isTerminal)
{
	BOOST_TEST_EQ_UINT64_t(isTerminal, GameState::calcIfTerminal(board));
}


static const long	  Positions1[]	= { 0,					1,					7,					8,					26,						45,					63 };
static const uint64_t ColumnMasks[] = { 0x0101010101010101,	0x0202020202020202,	0x8080808080808080,	0x0101010101010101,	 0x0404040404040404,	0x2020202020202020, 0x8080808080808080 };
BOOST_DATA_TEST_CASE(getColumnMask, butd::make(Positions1) ^ butd::make(ColumnMasks), position, mask)
{
	BOOST_TEST_EQ_UINT64_t(mask, GameState::getColumnMask(position));
}

static const uint64_t RowMasks[] = { 0xff,	0xff,	0xff,	0xff00,	 0xff000000, 0xff0000000000, 0xff00000000000000 };
BOOST_DATA_TEST_CASE(getRowMask, butd::make(Positions1) ^ butd::make(RowMasks), position, mask)
{
	BOOST_TEST_EQ_UINT64_t(mask, GameState::getRowMask(position));
}

static const long	  DPositions[] = { 0,1, 2, 3, 4, 5, 6, 7, 8,16,24,32,40,48,56 };
static const uint64_t LeftDiagonalMasks[] = {	0x8040201008040201, 0x80402010080402, 0x804020100804, 0x8040201008, 0x80402010, 0x804020, 0x8040, 0x80,
												0x4020100804020100, 0x2010080402010000, 0x1008040201000000, 0x804020100000000, 0x402010000000000,
												0x201000000000000, 0x100000000000000 };
BOOST_DATA_TEST_CASE(getLeftDiagonalMask, butd::make(DPositions) ^ butd::make(LeftDiagonalMasks), position, mask)
{
	BOOST_TEST_EQ_UINT64_t(mask, GameState::getLeftDiagonalMask(position));
}

static const uint64_t RightDiagonalMasks[] = {	0x1,0x102,0x10204,0x1020408,0x102040810,0x10204081020,0x1020408102040,0x102040810204080,0x102,
												0x10204, 0x1020408, 0x102040810, 0x10204081020, 0x1020408102040, 0x102040810204080 };
BOOST_DATA_TEST_CASE(getRightDiagonalMask, butd::make(DPositions) ^ butd::make(RightDiagonalMasks), position, mask)
{
	BOOST_TEST_EQ_UINT64_t(mask, GameState::getRightDiagonalMask(position));
}
BOOST_AUTO_TEST_SUITE_END();

template <typename T>
int n2p(T n)
{
	return n[0] - 'A' + (n[1] - '1') * 8;
}


uint64_t bfp(std::initializer_list<string> positions)
{
	uint64_t board = 0;
	for (auto p : positions)
	{
		const int ipos = n2p(p);
		board |= 1ull << ipos;
	}
	return board;
}

void test_moves(MoveList *moveList, std::initializer_list<const char*> expectedMoves)
{
	vector<int> mvInd;
	for (int i = 0; i < moveList->size; ++i) mvInd.push_back(i);
	
	for (auto mvi : expectedMoves)
	{
		const auto from = n2p(mvi);
		const auto to   = n2p(mvi + 4);
		BOOST_TEST(!mvInd.empty());
		bool found=false;
		for (auto it=mvInd.begin(); it!=mvInd.end(); ++it) 
		{
			auto& mv = moveList->move[*it];
			if (mv.from == from && mv.to == to) {
				mvInd.erase(it);
				found = true;
				break;
			}
		}
		if (!found) {
			BOOST_TEST_MESSAGE( string("move ") + mvi + " not found" );
		}
		BOOST_TEST(found);
	}
	for (auto i : mvInd) {
		BOOST_TEST_MESSAGE(string("unexpected move ") + moveList->move[i].toString());
	}
	BOOST_TEST(mvInd.empty());
}

BOOST_AUTO_TEST_SUITE(LinesOfAction_Moves)
/*   ABCDEFGH
 * 8|........|
 * 7|........|
 * 6|........|
 * 5|........|
 * 4|........|
 * 3|........|
 * 2|........|
 * 1|........|
 *   ABCDEFGH */

 /*   ABCDEFGH
  * 2|........|
  * 1|o.......|
  *   ABCDEFGH */
BOOST_AUTO_TEST_CASE(move_1)
{
	LinesOfActionGameRules gr;
	GameState gs(bfp({"A1"}), 0, GameState::Whites);
	
	auto ml = gr.GetPlayerLegalMoves(&gs, 0);
	test_moves(ml, { "A1->B1", "A1->A2", "A1->B2" });
}

/*   ABCDEFGH
 * 8|........|
 * 7|........|
 * 6|........|
 * 5|........|
 * 4|........|
 * 3|........|
 * 2|...oo...|
 * 1|........|
 *   ABCDEFGH */
BOOST_AUTO_TEST_CASE(move_2)
{
	LinesOfActionGameRules gr;
	GameState gs(bfp({ "D2","E2" }), 0, GameState::Whites);

	auto ml = gr.GetPlayerLegalMoves(&gs, 0);
	test_moves(ml, {	"D2->B2", "D2->F2", "D2->D1", "D2->D3", "D2->C3", "D2->E3", "D2->E1", "D2->C1",
								"E2->C2", "E2->G2", "E2->E1", "E2->E3", "E2->D3", "E2->F3", "E2->F1", "E2->D1" });
}

/*   ABCDEFGH
 * 8|........|
 * 7|........|
 * 6|.......o|
 * 5|........|
 * 4|........|
 * 3|o.......|
 * 2|........|
 * 1|........|
 *   ABCDEFGH */
BOOST_AUTO_TEST_CASE(move_3)
{
	LinesOfActionGameRules gr;
	GameState gs(bfp({ "A3","H6" }), 0, GameState::Whites);

	auto ml = gr.GetPlayerLegalMoves(&gs, 0);
	test_moves(ml, {	"A3->A4","A3->B4","A3->B3","A3->B2","A3->A2",
								"H6->H7","H6->G7","H6->G6","H6->G5","H6->H5"});
}

/*   ABCDEFGH
 * 8|........|
 * 7|........|
 * 6|........|
 * 5|*.*.....|
 * 4|........|
 * 3|o.*.....|
 * 2|........|
 * 1|..*.....|
 *   ABCDEFGH */
BOOST_AUTO_TEST_CASE(move_kill)
{
	LinesOfActionGameRules gr;
	GameState gs(bfp({ "A3"}), bfp({"A5","C1","C3","C5"}), GameState::Whites);

	auto ml = gr.GetPlayerLegalMoves(&gs, 0);
	test_moves(ml, { "A3->A5","A3->C5","A3->C3","A3->C1","A3->A1"});
}

/*   ABCDEFGH
 * 8|........|
 * 7|........|
 * 6|........|
 * 5|........|
 * 4|........|
 * 3|.******.|
 * 2|........|
 * 1|........|
 *   ABCDEFGH */
BOOST_AUTO_TEST_CASE(move_row)
{
	LinesOfActionGameRules gr;
	GameState gs(bfp({ "B3","C3","D3","E3","F3","G3" }), 0, GameState::Whites);

	auto ml = gr.GetPlayerLegalMoves(&gs, 0);
	test_moves(ml, {	"B3->C4","B3->B4","B3->A4","B3->A2","B3->B2","B3->C2","B3->H3",
								"C3->D4","C3->C4","C3->B4","C3->B2","C3->C2","C3->D2",
								"D3->E4","D3->D4","D3->C4","D3->C2","D3->D2","D3->E2",
								"E3->F4","E3->E4","E3->D4","E3->D2","E3->E2","E3->F2",
								"F3->E4","F3->F4","F3->G4","F3->E2","F3->F2","F3->G2",
								"G3->F4","G3->G4","G3->H4","G3->F2","G3->G2","G3->H2", "G3->A3" });
}

BOOST_AUTO_TEST_CASE(move_noop)
{
	LinesOfActionGameRules gr;
	GameState gs(bfp({ "B3","C3","D3","E3","F3","G3" }), 0, GameState::Whites);
	auto ml = gr.GetPlayerLegalMoves(&gs, GameState::Blacks);
	auto [mv, p] = gr.GetMoveFromList(ml, 0);
	auto ns = gr.ApplyMove(&gs,mv, GameState::Blacks);
	BOOST_TEST(gr.AreEqual(&gs, ns));
	gr.ReleaseMoveList(ml);
	gr.ReleaseGameState(ns);
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(LinesOfAction_misc)
BOOST_AUTO_TEST_CASE(state_to_string)
{
	LinesOfActionGameRules gr;
	auto gs = gr.CreateRandomInitialState(nullptr);
	auto sstring = gr.ToString(gs);
	//BOOST_TEST_MESSAGE(sstring);
	string exp = ".******.\no......o\no......o\no......o\no......o\no......o\no......o\n.******.\ncp=1";
	//BOOST_TEST_MESSAGE(exp);
	BOOST_TEST(exp == gr.ToString(gs));
	gr.ReleaseGameState(gs);
}
BOOST_AUTO_TEST_CASE(score_win)
{
	LinesOfActionGameRules gr;
	GameState gs(bfp({ "B1","B2","C3"}), bfp({ "C5","C7" }), GameState::Whites);

	int score[2];
	gr.Score(&gs, score);
	BOOST_TEST(score[GameState::Whites] == 100);
	BOOST_TEST(score[GameState::Blacks] == 0);
}
BOOST_AUTO_TEST_CASE(score_draw)
{
	LinesOfActionGameRules gr;
	
	GameState gs(bfp({ "B1" }), bfp({"C5"}), GameState::Whites);
	int score[2];
	gr.Score(&gs, score);
	BOOST_TEST(score[0] == 50);
	BOOST_TEST(score[1] == 50);
}
BOOST_AUTO_TEST_SUITE_END();
