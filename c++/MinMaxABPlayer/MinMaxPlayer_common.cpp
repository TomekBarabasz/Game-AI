#include "pch.h"
#include "GamePlayer.h"
#include <intrin.h>

auto evNumCards = [](const GameState* s, int value[], int num_players)
{
	const uint32_t *p = reinterpret_cast<const uint32_t*>(s);
	++p;
	for (int i = 0; i < num_players; ++i, ++p) {
		value[i] = 2 * (24 - __popcnt(*p));
	}
};

auto evNumCardsWeighted = [](const GameState* s, int value[], int num_players)
{
	static constexpr int weights[] = { 6,6,6,6,5,5,5,5,4,4,4,4,3,3,3,3,2,2,2,2,1,1,1,1 };
	const uint32_t *p = reinterpret_cast<const uint32_t*>(s);
	++p;
	for (int i = 0; i < num_players; ++i, ++p) 
	{
		int sum = 0;
		uint32_t mask = 1;
		for (int j=0; j<24; ++j){
			sum += (*p & mask) * weights[j];
			mask <<= 1;
		}
		value[i] = 84 - sum;
	}
};

EvalFunction_t createEvalFunction(const char* name)
{
	if (name == "num_cards") {
		return evNumCards;
	}
	if (name == "num_cards_weighted") {
		return evNumCardsWeighted;
	}
	//default
	return evNumCards;
}
