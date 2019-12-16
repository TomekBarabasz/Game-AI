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

BOOST_AUTO_TEST_SUITE(LinesOfActionGameState)
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
