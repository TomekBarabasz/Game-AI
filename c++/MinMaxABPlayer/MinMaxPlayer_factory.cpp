#include "pch.h"
#include "GamePlayer.h"
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS  
#include <string>
#include "GameRules.h"

IGamePlayer* createMinMaxABPlayer_2p(int pn, int depth, const string& evalFunc);
IGamePlayer* createMinMaxPlayer_mp(int pn, int numPlayers, int depth, const string& evalFunc);
IGamePlayer* createMinMaxABPlayer_iterativeDeepening(int pn, float time_limit, const string& evalFunc);
EvalFunction_t createEvalFunction(const char*);

IGamePlayer* createMinMaxPlayer(int player_number, const PlayerConfig_t& pc)
{
	const int number_of_players = pc.get_optional<int>("number_of_players").get_value_or(2);
	const auto evalFunc = pc.get_optional<std::string>("eval_function").get_value_or("");
	if (2 == number_of_players) 
	{
		const auto max_depth = pc.get_optional<int>("search_depth");
		if (max_depth) {
			return createMinMaxABPlayer_2p(player_number, max_depth.get(), evalFunc);
		}else
		{
			const auto time_limit = pc.get_optional<float>("move_time_limit");
			return createMinMaxABPlayer_iterativeDeepening(player_number, time_limit.get(), evalFunc);
		}
	}
	else {
		const auto max_depth = pc.get_optional<int>("search_depth");
		return createMinMaxPlayer_mp(player_number, number_of_players, max_depth.get(), evalFunc);
	}
}

BOOST_DLL_ALIAS(
	createMinMaxPlayer,	// <-- this function is exported with...
	createPlayer		// <-- ...this alias name
)
