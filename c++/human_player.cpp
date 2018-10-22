#include "pch.h"
#include "GamePlayer.h"
#include "GameRules.h"
#include <iostream>
#include "GamePlayer.h"
#include "GamePlayer.h"

struct HumanPlayer : IGamePlayer
{
	HumanPlayer(int pn) : playerNum(pn) {}
	IMove*	selectMove(const IGameState*gs, PlayerStats_t* ps) override;
	void	getGameStats(PlayerStats_t& ps) override {};
	void	release() override { delete this; }

	const int playerNum;
};

IMove* HumanPlayer::selectMove(const IGameState* gs, PlayerStats_t* ps)
{
	MoveList moves = gs->GetPlayerLegalMoves(playerNum);
	std::wcout << L"Game State is :" << std::endl;
	std::wcout << gs->ToString() << std::endl;
	std::wcout << L"Valid Moves ares : " << std::endl;

	for (auto i = 0; i < moves.size(); ++i)
	{
		std::wcout << L"Choice " << i << L" : " << moves[i]->toString() << std::endl;
	}
	wstring input;
	std::getline(std::wcin, input);
	const int selected = std::stoi(input.c_str());
	for (auto i = 0; i < moves.size(); ++i)
	{
		if (i != selected) moves[i]->release();
	}
	return moves[selected];
}

IGamePlayer* createHumanGamePlayer(int playerNum, int numPlayers)
{
	return new HumanPlayer(playerNum);
}

