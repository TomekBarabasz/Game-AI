#include "pch.h"
#include "GamePlayer.h"
#include <intrin.h>

auto evNumCards = [](const GameState* s, int value[], int num_players)
{
	const auto *p = reinterpret_cast<const uint64_t*>(s);
	++p;	//skip stack
	const uint64_t odd_mask = 0xaaaaaaaaaaaaaaaa; //a=1010
	for (int i = 0; i < num_players; ++i, ++p) {
		value[i] = 2 * (24 - (int)__popcnt64(*p | (*p & odd_mask) >> 1));
	}
};

auto evNumCardsWeighted = [](const GameState* s, int value[], int num_players)
{
	static constexpr int weights[] = { 6,6,6,6,5,5,5,5,4,4,4,4,3,3,3,3,2,2,2,2,1,1,1,1 };
	const auto *p = reinterpret_cast<const uint64_t*>(s);
	++p;	//skip stack
	for (int i = 0; i < num_players; ++i, ++p) 
	{
		int sum = 0;
		uint64_t mask = 0b11;
		for (int j=0; j<24; ++j,mask<<=2) {
			if (*p & mask) {
				sum += weights[j];
			}
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
