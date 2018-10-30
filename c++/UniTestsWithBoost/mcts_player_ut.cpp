#include "pch.h"
#include <boost/test/unit_test.hpp>
#include "mem_check.h"

#define UNIT_TEST
//#include "test_game_rules.h"
#include "test_game_rules.cpp"
#include "..\MCTSPlayer\MCTSPlayer.cpp"

namespace but = boost::unit_test;
//namespace butd = boost::unit_test::data;
TestGameRules rules;
bool rules_loaded = false;
static void eval(const GameState*, int value[], int) { value[0] = value[1] = 50; }
namespace std {
	inline std::ostream& operator<<(std::ostream& out, const std::wstring& value)
	{
		out << string(value.begin(), value.end());
		return out;
	}
}

struct CreateMCTSPlayer
{
	MCTSPlayer			p;
	MemoryLeakChecker	mlc;
	CreateMCTSPlayer() : p(0,2,20,11234)
	{
		if (!rules_loaded)
		{
			rules = Load("C:\\MyData\\Projects\\gra_w_pana\\test_state_game.txt");
			rules_loaded = true;
		}
		p.setGameRules(&rules);
		p.setEvalFunction(eval);
		mlc.checkpoint();
	}
	~CreateMCTSPlayer()
	{
		p.freeTree(p.m_root);
		p.m_nodePool.release();

		mlc.check();
		/*BOOST_TEST(mlc.diff == 0);
		if (mlc.diff)
		{
			BOOST_TEST_MESSAGE("Memory leak of " << mlc.diff << "bytes detected");
		}*/
	}
};

BOOST_FIXTURE_TEST_SUITE(MCTS_Player_UT, CreateMCTSPlayer);
BOOST_AUTO_TEST_CASE(createUsingFactory)
{
	PlayerConfig_t pc;
	auto p1 = createMCTSPlayer(0, pc);
	p1->release();
}
BOOST_AUTO_TEST_CASE(selection)
{
	auto *s = rules.CreateRandomInitialState(nullptr);
	p.m_root = p.findRootNode(s, nullptr);
	auto path = p.selection(p.m_root);
}
BOOST_AUTO_TEST_CASE(makeTreeNode)
{
	auto *s = rules.CreateRandomInitialState(nullptr);
	p.m_root = p.makeTreeNode(s);
}

BOOST_AUTO_TEST_CASE(selection_playOut)
{
	auto *s = rules.CreateRandomInitialState(nullptr);
	p.m_root = p.findRootNode(s, nullptr);
	auto path = p.selection(p.m_root);
	BOOST_TEST(1 == path.size());
	BOOST_TEST(&rules.m_tree[0] == path[0].first->state);
	BOOST_TEST(rules.m_tree[0].name == p.m_root->state->name);
	p.playOut(path);
	BOOST_TEST(4 == path.size());
}
BOOST_AUTO_TEST_CASE(run_1_Simulation)
{
	auto *s = rules.CreateRandomInitialState(nullptr);
	p.runSingleSimulation(s);
	BOOST_TEST(rules.m_tree[0].name == p.m_root->state->name);
}
BOOST_AUTO_TEST_CASE(run_15_Simulations)
{
	auto *s = rules.CreateRandomInitialState(nullptr);
	p.runNSimulations(s, 1);
	p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s1.gv");

	p.runNSimulations(s, 1);
	p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s2.gv");

	p.runNSimulations(s, 1);
	p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s3.gv");

	p.runNSimulations(s, 1);
	p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s4.gv");

	p.runNSimulations(s, 1);
	p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s5.gv");

	p.runNSimulations(s, 5);
	p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s10.gv");

	p.runNSimulations(s, 5);
	p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s15.gv");
}
BOOST_AUTO_TEST_SUITE_END();