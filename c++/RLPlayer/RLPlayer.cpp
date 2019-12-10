// RLPlayer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "GamePlayer.h"
#include "GameRules.h"
#include <functional>
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS  

namespace RL
{
	struct RLPlayer2P : IGamePlayer
	{
		RLPlayer2P(int player_number) : m_player_number(player_number) {}
		virtual ~RLPlayer2P()
		{
			m_game_rules->Release();
		}
		void startNewGame(GameState*) override
		{}
		void endGame(int score, GameResult result) override
		{}
		void setGameRules(IGameRules* gr) override
		{
			m_game_rules = gr;
			gr->AddRef();
		}
		MoveList* selectMove(GameState* pks) override
		{
			auto *moves = m_game_rules->GetPlayerLegalMoves(pks, m_player_number);
			auto *selected = m_game_rules->SelectMoveFromList(moves, 0);
			m_game_rules->ReleaseMoveList(moves);
			return selected;
		}
		NamedMetrics_t	getGameStats() override { return NamedMetrics_t(); }
		std::string getName() override { return "rl_player"; }
		void			resetStats() override
		{}
		void			release() override
		{
			delete this;
		}
		
		const int	m_player_number;
		IGameRules*	m_game_rules;
	};
	IGamePlayer* createRLPlayer(int player_number, const PlayerConfig_t& pc)
	{
		return new RLPlayer2P(player_number);
	}
}

BOOST_DLL_ALIAS(
	RL::createRLPlayer,	// <-- this function is exported with...
	createPlayer			// <-- ...this alias name
)
