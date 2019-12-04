#include "pch.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/dll/import.hpp>
#include "mem_check.h"
#include <Trace.h>
#include "mcts_player_ut_common.h"

namespace ut = boost::unit_test;
namespace bdata = boost::unit_test::data;

static void eval(const GameState*, int value[], int) { value[0] = value[1] = 50; }
namespace std {
	inline std::ostream& operator<<(std::ostream& out, const std::wstring& value)
	{
		//out << value;	recursive on all control paths, function will cause runtime stack overflow
		return out;
	}
}

/*		    root			: player 0
 *	   0.5/	 |0.7  \0.3
 *  100,0	100,0	100,0
 */
static TestGameRules makeGameTree_type0()
{
	TestGameRules r;
	r.m_tree.resize(4);
	auto* root = r.m_tree.data();
	root = r.m_tree.data();
	auto p = root + 1;
	auto t1 = new_state(p++, L"t1", 0).score({ 100,0 }).make();
	auto t2 = new_state(p++, L"t2", 0).score({ 100,0 }).make();
	auto t3 = new_state(p++, L"t3", 0).score({ 100,0 }).make();
	new_state(root, L"root", 0).move(L"left", t1, 0.5f).move(L"center", t2, 0.7f).move(L"right",t3, 0.3f).noop(1).make();
	return r;
}

/*		    root			: player 0
 *	   p=1/	 |p=1  \p=1
 *	   m1    m2    m3
 *	  /p=0.3 |p=0.5  |p=0.7
 *  100,0	100,0	100,0
 */
static TestGameRules makeGameTree_type1(std::initializer_list<std::pair<int,float>> init)
{
	TestGameRules r;
	r.m_tree.resize(7);
	auto* root = r.m_tree.data();
	root = r.m_tree.data();
	auto p = root + 1;
	auto initItr = init.begin();
	auto [v1, p1] = *initItr++;
	auto t1 = new_state(p++, L"t1", 0).score({ v1,0 }).make();
	auto m1 = new_state(p++, L"m1", 1).move(L"down", t1, p1).noop(0).make();
	auto [v2, p2] = *initItr++;
	auto t2 = new_state(p++, L"t2", 0).score({ v2,0 }).make();
	auto m2 = new_state(p++, L"m2", 1).move(L"down", t2, p2).noop(0).make();
	auto [v3, p3] = *initItr++;
	auto t3 = new_state(p++, L"t3", 0).score({ v3,0 }).make();
	auto m3 = new_state(p++, L"m3", 1).move(L"down", t3, p3).noop(0).make();
	
	new_state(root, L"root", 0).move(L"left", m1, 1.0f).move(L"center", m2, 1.0f).move(L"right", m3, 1.0f).noop(1).make();
	return r;
}

BOOST_FIXTURE_TEST_SUITE(MCTS_Player_IIG, CreateMCTSPlayer);

//NOTE: find most probable move (with highest expected value)
BOOST_AUTO_TEST_CASE(expvalue_graph_type0, *ut::tolerance(0.001))
{
	auto r = makeGameTree_type0();
	auto root = r.CreateRandomInitialState(nullptr);
	player.setGameRules(&r);

	auto [move, p] = player.runNSimulations(root, 10);
	BOOST_TEST(L"center" == r.ToWString(move));
}

//NOTE: find most probable move (with highest expected value)
BOOST_AUTO_TEST_CASE(expvalue_graph_type1, *ut::tolerance(0.001))
{
	auto r = makeGameTree_type1({{100,0.3f},{100,0.7f},{100,0.5f}});
	auto root = r.CreateRandomInitialState(nullptr);
	//NOTE: whole graph explored only with explore-exploit ratio = 1.0! 
	MC::MCTSConfig	cfg{ 0,2,1,false, 10,1.0,1234,-50,"c:\\MyData\\Projects\\gra_w_pana\\logs","","" };
	MC::Player player(cfg, new TestSimLimit(10), createInstance(""));
	player.setGameRules(&r);

	auto [move, p] = player.runNSimulations(root, 10);
	BOOST_TEST(L"center" == r.ToWString(move));
}

//NOTE: find most probable move (with highest expected value)
BOOST_AUTO_TEST_CASE(expvalue_graph_type1_1, *ut::tolerance(0.001))
{
	auto r = makeGameTree_type1({ {100,0.3f},{50,0.7f},{100,0.5f} });
	auto root = r.CreateRandomInitialState(nullptr);
	//NOTE: whole graph explored only with explore-exploit ratio = 1.0! 
	MC::MCTSConfig	cfg{ 0,2,1,false, 10,1.0,1234,-50,"c:\\MyData\\Projects\\gra_w_pana\\logs","","" };
	MC::Player player(cfg, new TestSimLimit(10), createInstance(""));
	player.setGameRules(&r);

	auto [move, p] = player.runNSimulations(root, 10);
	BOOST_TEST(L"right" == r.ToWString(move));
}

BOOST_AUTO_TEST_SUITE_END();
