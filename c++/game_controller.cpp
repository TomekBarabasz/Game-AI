#include "pch.h"
#include "GameController.h"
#include "GameRules.h"
#include <sstream>
#include <functional>
#include <chrono>

using std::vector;
using std::string;
extern IGameState* createGraWPanaGameState();
extern void initGraWPanaMemoryMgmt();
extern void cleanupGraWPanaMemoryMgmt();

struct GameController : IGameController
{
	void run(std::function<IGameState*(int)> createGameState, const std::vector<IGamePlayer*>& players, int numGames, int roundLimit, unsigned seeds[], int score[], GameStats_t& stats) override
	{
		auto NumPlayers = (int)players.size();
		int single_score[4];
		int tot_rounds = 0;
		for (int i = 0; i < NumPlayers; ++i) { score[i] = 0; }
		using CLK = std::chrono::high_resolution_clock;

		for (int ng=0;ng<numGames;++ng)
		{
			const unsigned seed = seeds != nullptr ? seeds[ng] : unsigned(CLK::now().time_since_epoch().count());
			tot_rounds += runSingle(createGameState, players, roundLimit, seed, single_score, stats);
			for (int i=0;i<NumPlayers;++i) {
				score[i] += single_score[i];
			}
		}

		for (int i = 0; i < NumPlayers; ++i) {
			score[i] /= numGames;
		}
		stats["total rounds"] = std::to_string(tot_rounds);
		std::stringstream ss;
	}
	void release() override { delete this; }

	int runSingle(std::function<IGameState*(int)> createGameState, const std::vector<IGamePlayer*>& players, int roundLimit, unsigned seed, int score[], GameStats_t& stats)
	{
		IGameState *gs = createGameState(players.size());
		gs->Initialize((int)players.size(), seed);
		//std::wcout << L"initial state   :" << std::endl << gs->ToString() << std::endl;
		int numRounds = 0;
		//std::wcout << L"state :" << std::endl << gs->ToString() << std::endl;
		//std::cout << "hash :" << gs->HashS() << std::endl;
		while(!gs->IsTerminal())
		{
			MoveList ml;
			
			for (auto *player : players) {
				ml.push_back(player->selectMove( gs ));
			}
			//std::wcout << L"state   :" << std::endl << gs->ToString() << std::endl;
			//std::wcout << L"p0 move :" << ml[0]->toString() << std::endl;
			//std::wcout << L"p1 move :" << ml[1]->toString() << std::endl;
			auto gsn = gs->Next(ml);
			gs->Release();
			gs = gsn;
			//std::wcout << L"state :" << std::endl << gs->ToString() << std::endl;
			//std::cout << "hash :" << gs->HashS() << std::endl;
			if (++numRounds >= roundLimit) 
			{
				break;
			}
		}
		//std::wcout << L"final state   :" << std::endl << gs->ToString() << std::endl << std::endl;
		gs->Score(score);
		gs->Release();
		return numRounds;
	}
};

IGameController* IGameController::create()
{
	return new GameController();
}
