#pragma once
#include <vector>
#include "GamePlayer.h"
#include <unordered_map>

using std::string;
using GameStats_t = std::unordered_map<string, string>;

struct Metric
{
	string name;
	union {
		int		ival;
		float	fval;
	};
};
struct IGameController
{
	static IGameController* create();
	virtual void run(const std::vector<IGamePlayer*>& players, int numGames, int roundLimit, unsigned seeds[], int score[], GameStats_t&) = 0;
	virtual void release() = 0;
protected:
	virtual ~IGameController(){}
};
