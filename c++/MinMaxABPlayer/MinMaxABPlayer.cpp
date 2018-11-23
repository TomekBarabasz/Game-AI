// MinMaxABPlayer.cpp : Defines the exported functions for the DLL application.
//
#include "pch.h"
#include "GamePlayer.h"
#include "GameRules.h"
#include <functional>
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS  
#include <intrin.h>

using CLK = std::chrono::high_resolution_clock;

auto EvalFunctionNumCards = [](const GameState* s, int value[], int num_players)
{
	const uint32_t *p = reinterpret_cast<const uint32_t*>(s);
	++p;
	for (int i=0;i<num_players;++i,++p)	{
		value[i] = 2 * (24 - __popcnt(*p));
	}
};

struct MinMaxABPlayer_2p : IGamePlayer
{
	struct Action
	{
		MoveList *mv;
		int value;
	};

	const int	m_player_number;
	const int	max_depth;
	EvalFunction_t m_eval_function;
	IGameRules* m_game_rules;
	Histogram<float> m_move_select_time;

	MinMaxABPlayer_2p(int pn, int maxDepth) : 
		m_player_number(pn), 
		max_depth(maxDepth),
		m_eval_function(EvalFunctionNumCards)
	{}
	void	release() override { delete this; }
	void	startNewGame() override {}
	void	endGame() override {}
	void	setGameRules(IGameRules* gr) override { m_game_rules = gr; }
	void	setEvalFunction(EvalFunction_t ef) override { m_eval_function = ef; }
	NamedMetrics_t	getGameStats() override
	{
		NamedMetrics_t nm;
		nm["move_select_time_ms"] = m_move_select_time;
		return nm;
	}
	void	resetStats() override {}
	void	enableTrace(bool) override {}
	std::string getName() override { return "minmax ab depth " + std::to_string(max_depth); }
	Action  selectMoveRec(const GameState* gs, int current_player, int depth, int alpha, int beta, bool Maximize)
	{
		if (m_game_rules->IsTerminal(gs))
		{
			int score[2];
			m_game_rules->Score(gs, score);
			return { nullptr, zeroSumValue(score) };
		}
		if (depth >= max_depth)
		{
			int value[2];
			m_eval_function(gs, value, 2);
			return { nullptr, zeroSumValue(value) };
		}

		MoveList * moves = m_game_rules->GetPlayerLegalMoves(gs, current_player);
		const auto number_of_moves = m_game_rules->GetNumMoves(moves);
		if (0 == depth && number_of_moves == 1)
		{
			return { moves, 0 };
		}
		int best_value = Maximize ? -1000 : 1000;
		int best_move_idx = -1;
		for (int move_idx = 0; move_idx < number_of_moves; ++move_idx)
		{
			auto *ngs = m_game_rules->ApplyMove(gs, m_game_rules->GetMoveFromList(moves, move_idx), current_player);
			//alpha = highest value ever - best choice for max player
			//beta  =  lowest value ever - best choice for min player
			Action a = selectMoveRec(ngs, 1 - current_player, depth + 1, alpha, beta, !Maximize);
			m_game_rules->ReleaseGameState(ngs);
			if (Maximize)
			{
				if (a.value > best_value)
				{
					best_value = a.value;
					best_move_idx = move_idx;
				}
				if (best_value >= beta) break;
				alpha = __max(alpha, best_value);
			}
			else
			{
				if (a.value < best_value)
				{
					best_value = a.value;
					best_move_idx = move_idx;
				}
				if (best_value <= alpha) break;
				beta = __min(beta, best_value);
			}
		}
		auto *selected_move = depth > 0 ? nullptr : m_game_rules->SelectMoveFromList(moves, best_move_idx);
		m_game_rules->ReleaseMoveList(moves);
		return { selected_move, best_value };
	}
	MoveList* selectMove(GameState* gs) override
	{
		std::chrono::time_point<CLK> tp_start = CLK::now();
		MoveList* ml = selectMoveRec(gs, m_player_number, 0, -1000, 1000, true).mv;
		const auto mseconds = (long) std::chrono::duration_cast<std::chrono::milliseconds>(CLK::now() - tp_start).count();
		m_move_select_time.insert(mseconds / 1000.0f);
		return ml;
	}
	int zeroSumValue(int utility[]) const
	{
		return utility[m_player_number] - utility[1 - m_player_number];
	}
};

struct MinMaxABPlayer_mp : IGamePlayer
{
	const int		m_player_number;
	const int		m_number_of_players;
	const int		max_depth;
	EvalFunction_t	m_eval_function;
	IGameRules*		m_game_rules;
	Histogram<float> m_move_select_time;

	struct Action
	{
		MoveList *mv;
		int value[4];
	};

	MinMaxABPlayer_mp(int pn, int np, int maxDepth) :
		m_player_number(pn),
		m_number_of_players(np),
		max_depth(maxDepth),
		m_eval_function(EvalFunctionNumCards)
	{}
	void	release() override { delete this; }
	void	setGameRules(IGameRules* gr) override { m_game_rules = gr; }
	void	setEvalFunction(EvalFunction_t ef) override { m_eval_function = ef; }
	NamedMetrics_t	getGameStats() override
	{
		NamedMetrics_t nm;
		nm["move_select_time_ms"] = m_move_select_time;
		return nm;
	}
	void	resetStats() override {}
	void	enableTrace(bool) override {}
	void	startNewGame() override {}
	void	endGame() override {}
	std::string getName() override { return "minmax ab multiplayer depth " + std::to_string(max_depth); }

	MoveList* selectMove(GameState* gs) override
	{
		std::chrono::time_point<CLK> tp_start = CLK::now();
		MoveList* ml = selectMoveRec(gs, m_player_number, 0).mv;
		const auto mseconds = (long)std::chrono::duration_cast<std::chrono::milliseconds>(CLK::now() - tp_start).count();
		m_move_select_time.insert(mseconds / 1000.0f);
		return ml;
	}
	Action  selectMoveRec(const GameState* gs, int current_player, int depth)
	{
		if (m_game_rules->IsTerminal(gs))
		{
			Action a { nullptr };
			m_game_rules->Score(gs, a.value);
			return a;
		}
		if (depth >= max_depth)
		{
			Action a {nullptr};
			m_eval_function(gs, a.value, m_number_of_players);
			return a;
		}

		MoveList * moves = m_game_rules->GetPlayerLegalMoves(gs, current_player);
		const auto number_of_moves = m_game_rules->GetNumMoves(moves);
		if (0 == depth && number_of_moves == 1) {
			return { moves, 0 };
		}
		int best_move_idx = -1;
		int best_value = -1000;
		for (int move_idx = 0; move_idx < number_of_moves; ++move_idx)
		{
			auto *ngs = m_game_rules->ApplyMove(gs, m_game_rules->GetMoveFromList(moves, move_idx), current_player);
			const int next_player = m_game_rules->GetCurrentPlayer(ngs);
			//alpha = highest value ever - best choice for max player
			//beta  =  lowest value ever - best choice for min player
			Action a = selectMoveRec(ngs, next_player, depth + 1);
			m_game_rules->ReleaseGameState(ngs);
			if (a.value[current_player] > best_value)
			{
				best_value = a.value[current_player];
				best_move_idx = move_idx;
			}
		}
		auto *selected_move = depth > 0 ? nullptr : m_game_rules->SelectMoveFromList(moves, best_move_idx);
		m_game_rules->ReleaseMoveList(moves);
		return { selected_move, best_value };
	}
};

IGamePlayer* createMinMaxPlayer(int player_number, const PlayerConfig_t& pc)
{
	const int number_of_players = pc.get_optional<int>("number_of_players").get_value_or(2);
	const int max_depth = pc.get_optional<int>("search_depth").get_value_or(3);

	if (2 == number_of_players) {
		return new MinMaxABPlayer_2p(player_number, max_depth);
	}
	return new MinMaxABPlayer_mp(player_number, number_of_players, max_depth);
}

BOOST_DLL_ALIAS(
	createMinMaxPlayer,	// <-- this function is exported with...
	createPlayer		// <-- ...this alias name
)

