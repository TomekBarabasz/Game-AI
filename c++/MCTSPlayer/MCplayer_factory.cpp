#include "pch.h"
#include "MCTSPlayer.h"
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS  
#include <Trace.h>

using namespace Trace;
EvalFunction_t createEvalFunction(const char*);
namespace MCRL {
	IGamePlayer* createMCRLPlayer(int player_number, const PlayerConfig_t& pc);
};
namespace MC
{
	struct NumSimMoveLimit : IMoveLimit
	{
		const int SimLimit;
		int numSim;
		NumSimMoveLimit(int simLimit) : SimLimit(simLimit) {}
		void start() override { numSim = 0; }
		bool can_continue() override { return ++numSim < SimLimit; }
		void release() override { delete this; }
	};

	struct TimeMoveLimit : IMoveLimit
	{
		const unsigned TimeLimitmiliSec;
		std::chrono::time_point<CLK> tp_start;
		TimeMoveLimit(double timeLimit) : TimeLimitmiliSec(unsigned(timeLimit * 1000)) {}
		void start() override { tp_start = CLK::now(); }
		bool can_continue() override
		{
			const auto mseconds = std::chrono::duration_cast<std::chrono::milliseconds>(CLK::now() - tp_start).count();
			return mseconds < TimeLimitmiliSec;
		}
		void release() override { delete this; }
	};

	IMoveLimit* createMoveLimit(const PlayerConfig_t& pc)
	{
		const auto move_time_limit = pc.get_optional<float>("move_time_limit");
		const auto move_sim_limit = pc.get_optional<int>("move_sim_limit");
		if (move_sim_limit) {
			return new NumSimMoveLimit(move_sim_limit.get());
		}
		return new TimeMoveLimit(move_time_limit.get_value_or(1.0));
	}
	IGamePlayer* createMCTSPlayer(int player_number, const PlayerConfig_t& pc)
	{
		MCTSConfig cfg;

		cfg.PlayerNumber = player_number;
		cfg.seed = pc.get_optional<unsigned long>("random_seed").get_value_or(unsigned(CLK::now().time_since_epoch().count()));
		cfg.NumberOfPlayers = pc.get_optional<int>("number_of_players").get_value_or(2);
		cfg.MaxPlayoutDepth = pc.get_optional<int>("playout_depth").get_value_or(20);
		cfg.NodesToAppendDuringExpansion = pc.get_optional<int>("expand_size").get_value_or(1);
		cfg.EERatio = pc.get_optional<float>("explore_exploit_ratio").get_value_or(1.0);
		cfg.traceMoveFilename = pc.get_optional<string>("trace_move_filename").get_value_or("");
		cfg.gameTreeFilename = pc.get_optional<string>("game_tree_filename").get_value_or("");
		cfg.outDir = pc.get_optional<string>("out_dir").get_value_or("");
		cfg.ExpandFromLastPermanentNode = pc.get_optional<int>("expand_from_last_permanent_node").get_value_or(0) == 1;
		cfg.WeightedBackprop = pc.get_optional<float>("weighted_backprop").get_value_or(0);
		cfg.EvalFcn = pc.get_optional<std::string>("eval_function").get_value_or("");
		cfg.BestMoveValueEps = pc.get_optional<float>("best_move_value_eps").get_value_or(0.005f);
		cfg.CycleScore = pc.get_optional<int>("cycle_score").get_value_or(50);
		cfg.Name = pc.get_optional<string>("fullname").get_value_or(pc.get<string>("name"));
		
		auto logger = createInstance(pc.get_optional<string>("trace").get_value_or(""), cfg.outDir);
		auto move_limit = createMoveLimit(pc);
		return new Player(cfg, move_limit, logger);
	}
	IGamePlayer* createMCPlayer(int player_number, const PlayerConfig_t& pc)
	{
		auto type = pc.get_optional<string>("type").get_value_or("mcts");
		if (type == "mcrl")	return MCRL::createMCRLPlayer(player_number, pc);
		if (type == "mcts")	return createMCTSPlayer(player_number, pc);
		return createMCTSPlayer(player_number, pc);
	}
}

BOOST_DLL_ALIAS(
	MC::createMCPlayer,	// <-- this function is exported with...
	createPlayer			// <-- ...this alias name
)