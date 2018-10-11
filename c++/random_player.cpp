#include "pch.h"
#include "GamePlayer.h"
#include "GameRules.h"
#include <random>

struct RandomPlayer : IGamePlayer
{
	RandomPlayer(int pn) : playerNum(pn) {}
	IMove*	selectMove(const IGameState*) override;
	void	release() override { delete this; }

	const int playerNum;
	std::default_random_engine generator;
};

IMove* RandomPlayer::selectMove(const IGameState* s)
{
	MoveList moves = s->GetPlayerLegalMoves(playerNum);
	if (1 == moves.size()) return moves[0];
	std::uniform_int_distribution<int> distribution(0, int(moves.size())-1);
	const int selected = distribution(generator);
	for (auto i = 0; i < moves.size(); ++i) {
		if (i != selected) moves[i]->release();
	}
	return moves[selected];
}

IGamePlayer* createRandomGamePlayer(int playerNum, int numPlayers)
{
	return new RandomPlayer(playerNum);
}
