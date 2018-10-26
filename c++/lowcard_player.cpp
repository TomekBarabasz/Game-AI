#include "pch.h"
#include "GamePlayer.h"
#include "GameState.h"

struct LowCardPlayer : IGamePlayer
{
	LowCardPlayer(int pn) : playerNum(pn){}
	IMove*	selectMove(const IGameState*gs, PlayerStats_t* ps) override;
	void	getGameStats(PlayerStats_t& ps) override {}
	void	release() override { delete this; }

	const int playerNum;
};

IMove* LowCardPlayer::selectMove(const IGameState* gs, PlayerStats_t* ps)
{
	MoveList moves = gs->GetPlayerLegalMoves(playerNum);
	const int selected = 0;
	for(auto i=0;i<moves.size();++i) {
		if (i != selected) moves[i]->release();
	}
	return moves[selected];
}

IGamePlayer* createLowcardGamePlayer(int playerNum, int numPlayers)
{
	return new LowCardPlayer(playerNum);
}

