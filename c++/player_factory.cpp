#include "pch.h"
#include "GameRules.h"

#include <vector>
#include <functional>
#include <boost/algorithm/string.hpp>
#include <map>
#include <string>

using boost::algorithm::split;
using std::vector;
using std::string;
struct IGamePlayer;

IGamePlayer* createLowcardGamePlayer(int playerNum, int numPlayers);
IGamePlayer* createRandomGamePlayer(int playerNum, int numPlayers);
IGamePlayer* createMinMaxABGamePlayer(int playerNum, int numPlayers, int maxDepth, std::function<void(const IGameState*, int[])> eval);
IGamePlayer* createMCTSPlayer(int playerNum, int numPlayers, std::function<void(const IGameState*, int*)>, unsigned seed);

std::function<void(const IGameState*, int[])> createEvalFunction(const string& description)
{
	auto ev_dummy = [](const IGameState *gs, int score[]) {
		const int drawScore = 100 / gs->NumPlayers();
		for (int i=0;i<gs->NumPlayers();++i) {
			score[i] = drawScore;
		}
	};
	auto ev_numcards = [](const IGameState *gs, int score[]) {
		auto hash = gs->Hash();
		GameStateHash_t mask((1LL << 24) - 1);
		for (auto i = 0; i < gs->NumPlayers(); ++i) {
			hash >>= 24;
			auto masked = hash & mask;
			score[i] = 100 - 4 * (int)masked.count();
		}
	};
	auto ev_weighthed = [](const IGameState *gs, int score[])
	{
		auto hash = gs->Hash();
		for (auto i = 0; i < gs->NumPlayers(); ++i)
		{
			hash >>= 24;
			const int weights[] = { -10, -7, -5, -3, -2, -1 };
			int tot_weighted = 0;
			for (int j = 0; j < 24; ++j) {
				if (hash[j]) {
					tot_weighted += weights[j / 4];
				}
			}
			score[i] = 100 + tot_weighted;
		}
	};
	std::map<string, std::function<void(const IGameState*, int[])>> evalFcnFactory =
	{
		{"numcard", ev_numcards},
		{"weighted",ev_weighthed}
	};
	auto it = evalFcnFactory.find(description);
	return it != evalFcnFactory.end() ? it->second : ev_dummy;
}
IGamePlayer* createPlayer(const vector<string>& tokens, int playerNum, int numPlayers)
{
	std::map<string, std::function<IGamePlayer*(const vector<string>&, int, int)>> factory =
	{
		{ "none",	[](const vector<string>&,int,int) { return nullptr; } },
		{ "random",	[](const vector<string>&,int pn,int num) { return createRandomGamePlayer(pn,num); } },
		{ "lowcard",[](const vector<string>&,int pn,int num) { return createLowcardGamePlayer(pn,num); } },
		{ "minmax", [&](const vector<string>& t,int pn,int num) {
			const int maxdepth = t.size() > 1 ? std::stoi(t[1]) : 3;
			auto eval = createEvalFunction(t.size() > 2 ? t[2] : "");
			return createMinMaxABGamePlayer(pn, num, maxdepth, eval);
		}},
		{ "mcts", [](const vector<string>&t,int pn, int np)
		{
			auto eval = createEvalFunction(t.size() > 1 ? t[1] : "");
			return createMCTSPlayer(pn,np,eval,0);
		}}
	};
	
	const auto it = factory.find(tokens[0]);
	return it != factory.end() ? it->second(tokens, playerNum, numPlayers) : nullptr;
}
