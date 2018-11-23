#include "pch.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>	//required for BOOST_DATA_TEST_CASE
#include <boost/test/data/monomorphic.hpp>	//required for boost::unit_test::data

#define _CRTDBG_MAP_ALLOC  
#define UNIT_TEST
#include <object_pool_multisize.h>

namespace but = boost::unit_test;
namespace butd = boost::unit_test::data;

BOOST_AUTO_TEST_SUITE(bit_mask);
#if 0
static const int NumBlocks[]        = { 1, 50,			120,				256,			1024 };
static const int LastBitBlockMask[] = { 1, (1ull<<50)-1,	(1ull<<(120-64))-1,	long long(-1),	long long(-1) };
BOOST_DATA_TEST_CASE(clear_, butd::make(NumBlocks)^butd::make(LastBitBlockMask), num_blocks, last_bitblock_mask)
{
	OccupancyMask<num_blocks> om;
	BOOST_TEST(om.m_mask[OccupancyMask<1>::NumBitBlocks - 1] == last_bitblock_mask);
}
#endif
BOOST_AUTO_TEST_CASE(clear)
{
	OccupancyMask<1> om1;
	BOOST_TEST(om1.m_mask[OccupancyMask<1>::NumBitBlocks - 1] == 1);

	OccupancyMask<50> om2;
	BOOST_TEST(om2.m_mask[OccupancyMask<50>::NumBitBlocks - 1] == (1ull << 50)-1);

	OccupancyMask<120> om3;
	BOOST_TEST(om3.m_mask[OccupancyMask<120>::NumBitBlocks - 1] == (1ull << (120 - 64)) - 1);

	OccupancyMask<256> om4;
	BOOST_TEST(om4.m_mask[OccupancyMask<256>::NumBitBlocks - 1] == long long (-1));

	OccupancyMask<1022> om5;
	BOOST_TEST(om5.m_mask[OccupancyMask<1022>::NumBitBlocks - 1] == (1ull << 62 )-1);

	OccupancyMask<1024> om6;
	BOOST_TEST(om6.m_mask[OccupancyMask<1024>::NumBitBlocks - 1] == long long (-1));
}
BOOST_AUTO_TEST_CASE(find_1)
{
	OccupancyMask<64> om;
	om.m_mask[0] = 0b0000111000;
	auto offset = om.findChunkBlock(3);
	BOOST_TEST(offset == 3);
}
BOOST_AUTO_TEST_CASE(find_2)
{
	OccupancyMask<64> om;
	om.m_mask[0] = 0b1110101000;
	auto offset = om.findChunkBlock(3);
	BOOST_TEST(offset == 7);
}
BOOST_AUTO_TEST_CASE(find_3)
{
	OccupancyMask<64> om;
	om.m_mask[0] = 0b011011011;
	auto offset = om.findChunkBlock(3);
	BOOST_TEST(offset == -1);
}
BOOST_AUTO_TEST_CASE(find_4)
{
	OccupancyMask<128> om;
	om.m_mask[0] = 0b011011011;
	om.m_mask[1] = 0b111000000000000;
	auto offset = om.findChunkBlock(3);
	BOOST_TEST(offset == 64 + 12);
}
BOOST_AUTO_TEST_CASE(free_1)
{
	OccupancyMask<64> om;
	om.m_mask[0] = 0b0001110001;
	om.freeChunkBlock(1, 3);
	BOOST_TEST(om.m_mask[0] == 0b0001111111);
}
BOOST_AUTO_TEST_CASE(free_2)
{
	OccupancyMask<128> om;
	om.m_mask[0] = 0b011011011;
	om.m_mask[1] = 0b111000000000000;
	om.freeChunkBlock(70, 5);
	BOOST_TEST(om.m_mask[0] == 0b011011011);
	BOOST_TEST(om.m_mask[1] == 0b111011111000000);
}
BOOST_AUTO_TEST_SUITE_END();