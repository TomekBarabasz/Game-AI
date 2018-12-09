#include "pch.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/dll/import.hpp>

#define UNIT_TEST
#include "../MCTSPlayer/MCTSPlayer.h"
#include <Trace.h>
#include "test_game_rules.h"
//#include <GameRules.h>

namespace ut = boost::unit_test;
namespace bdata = boost::unit_test::data;

namespace std {
	inline std::ostream& operator<<(std::ostream& out, const std::wstring& value)
	{
		out << string(value.begin(), value.end());
		return out;
	}
}
struct TestSimLimit : MCTS::IMoveLimit
{
	TestSimLimit(int limit) : sim_limit(limit) {}
	int sim_done;
	const int sim_limit;
	void start() { sim_done = 0; }
	bool can_continue() { return ++sim_done < sim_limit; }
	void release() { delete this; }
};

struct CreateMCTSPlayerAndGameRules
{
	MCTS::Config		cfg{ 0,2,1,false, 10,2.0,1234,-50,"c:\\MyData\\Projects\\gra_w_pana\\logs","","" };
	MCTS::Player		player;
	IGameRules			*gr;
	std::function<IGameRules*(int)> createGameRules;
	CreateMCTSPlayerAndGameRules() : player(cfg, new TestSimLimit(30), ITrace::createInstance(""))
	{
		createGameRules = boost::dll::import_alias<IGameRules*(int number_of_players)>(
			"GraWPanaZasadyV2",
			"createGameRules",
			boost::dll::load_mode::append_decorations);
		gr = createGameRules(2);
		player.setGameRules(gr);
	}
	~CreateMCTSPlayerAndGameRules()
	{
		gr->Release();
	}
};
TestGameRules makeGameTree_type3();
BOOST_FIXTURE_TEST_SUITE(MCTS_Player_dump_load_tree, CreateMCTSPlayerAndGameRules, *ut::disabled());
BOOST_AUTO_TEST_CASE(dump_load_type1)
{
	auto r = makeGameTree_type3();
	auto root = r.CreateRandomInitialState(nullptr);
	player.setGameRules(&r);

	player.runNSimulations(root, 15);
	const char* filename = "C:\\MyData\\Projects\\gra_w_pana\\logs\\test_tree_dump.gv";
	player.dumpTree(filename, player.m_root);
	player.freeTree(player.m_root);
	auto [root2, path]= player.loadTreeFromFile(filename);
	player.m_root = root2;
	const char* filename2 = "C:\\MyData\\Projects\\gra_w_pana\\logs\\test_tree_dump_2.gv";
	player.dumpTree(filename2, player.m_root);
	boost::filesystem::remove(filename);
	boost::filesystem::remove(filename2);
}
BOOST_AUTO_TEST_CASE(expansion_case_1)
{
	player.setGameRules(gr);
	auto [root,path] = player.loadTreeFromFile("C:\\MyData\\Projects\\gra_w_pana\\dbg_logs\\@3-exp-loop\\mcts_g_tree_before_exp_g_1_mv_3.svg");
	player.m_root = root;
	player.backpropagation(path, false);
	size_t lastPermanentNode = 0;
	for (auto i=0; i < path.size(); ++i) {
		if (0==path[lastPermanentNode].first->temporary) {
			lastPermanentNode = __max(lastPermanentNode, i);
		}
	}
	player.expansion(path);
}
BOOST_AUTO_TEST_SUITE_END();
