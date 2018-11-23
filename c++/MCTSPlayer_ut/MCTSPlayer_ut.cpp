#include "pch.h"
#define BOOST_TEST_MODULE test_module
#include <boost/test/unit_test.hpp>
#include "mem_check.h"

#define UNIT_TEST
#include "test_game_rules.h"

#define TRACE(fmt, ...){\
	wchar_t txt[256];\
	swprintf_s(txt, fmt, ##__VA_ARGS__); \
	BOOST_TEST_MESSAGE(txt);\
}

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
namespace boost {
	namespace unit_test {
		bool init_unit_test()
		{
			return false;
		}
	}
}

struct CreateMCTSPlayer
{
	Config				cfg{ 0,2,1,10,1.0,1234,"" };
	MCTSPlayer			p;
	MemoryLeakChecker	mlc;
	
	CreateMCTSPlayer() : p(cfg, new NumSimMoveLimit(10))
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
		p.freeTree(p.m_root); p.m_root = nullptr;
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
	p.m_root = p.findRootNode(s);
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
	p.m_root = p.findRootNode(s);
	auto path = p.selection(p.m_root);
	BOOST_TEST(1 == path.size());
	BOOST_TEST(&rules.m_tree[0] == path[0].first->state);
	BOOST_TEST(rules.m_tree[0].name == p.m_root->state->name);
	p.playOut(path);
	BOOST_TEST(4 == path.size());
}
BOOST_AUTO_TEST_CASE(run_1_simulation)
{
	auto *s = rules.CreateRandomInitialState(nullptr);
	p.runNSimulations(s,1);
	BOOST_TEST(rules.m_tree[0].name == p.m_root->state->name);
}
BOOST_AUTO_TEST_CASE(run_15_simulations_expand_one)
{
	auto *s = rules.CreateRandomInitialState(nullptr);
	p.runNSimulations(s, 1);
	p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s1.gv", p.m_root);

	p.runNSimulations(s, 1);
	p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s2.gv", p.m_root);

	p.runNSimulations(s, 1);
	p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s3.gv", p.m_root);

	p.runNSimulations(s, 1);
	p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s4.gv", p.m_root);

	p.runNSimulations(s, 1);
	p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s5.gv", p.m_root);

	p.runNSimulations(s, 5);
	p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s10.gv", p.m_root);

	p.runNSimulations(s, 5);
	p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s15.gv", p.m_root);
}

BOOST_AUTO_TEST_CASE(run_15_simulations_expand_all)
{
	Config				ncfg{ 0,2,-1,10,1.0,1234,"" };
	MCTSPlayer			p1(ncfg, new NumSimMoveLimit(10));
	p1.setGameRules(&rules);
	p1.setEvalFunction(eval);

	auto *s = rules.CreateRandomInitialState(nullptr);
	p1.runNSimulations(s, 1);
	p1.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s1_ea.gv", p.m_root);

	p1.runNSimulations(s, 1);
	p1.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s2_ea.gv", p.m_root);

	p1.runNSimulations(s, 1);
	p1.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s3_ea.gv", p.m_root);

	p1.runNSimulations(s, 1);
	p1.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s4_ea.gv", p.m_root);

	p1.runNSimulations(s, 1);
	p1.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s5_ea.gv", p.m_root);

	p1.runNSimulations(s, 5);
	p1.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s10_ea.gv", p.m_root);

	p1.runNSimulations(s, 5);
	p1.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s15_ea.gv", p.m_root);
}

BOOST_AUTO_TEST_SUITE_END();