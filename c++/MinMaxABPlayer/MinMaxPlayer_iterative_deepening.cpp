#include "pch.h"
#include "GamePlayer.h"
#include "GameRules.h"
#include <functional>
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS  
#include <intrin.h>
#include <array>

using CLK = std::chrono::high_resolution_clock;



struct MinMaxABPlayer_iterativeDeepening : IGamePlayer
{
	struct Action
	{
		MoveList *mv;
		int value;
	};

	const int	m_player_number;
	const float m_time_limit;
	EvalFunction_t m_eval_function;
	IGameRules* m_game_rules;
	Histogram<float> m_move_select_time;

	MinMaxABPlayer_iterativeDeepening(int pn, float time_limit, EvalFunction_t evalFcn) :
		m_player_number(pn),
		m_time_limit(time_limit),
		m_eval_function(evalFcn)
	{
		m_move_select_time.Rounding(2).Prefix('m');
	}
	void	release() override { delete this; }
	void	startNewGame() override {}
	void	endGame(int score, GameResult result) override {}
	void	setGameRules(IGameRules* gr) override { m_game_rules = gr; }
	void	setEvalFunction(EvalFunction_t ef) override { m_eval_function = ef; }
	NamedMetrics_t	getGameStats() override
	{
		NamedMetrics_t nm;
		nm["move_select_time_ms"] = m_move_select_time;
		return nm;
	}
	void	resetStats() override {}
	std::string getName() override { return "minmax ab id " + std::to_string(m_time_limit) + " sec"; }
	Action  selectMoveRec(const GameState* gs, int current_player, int depth, int alpha, int beta, bool Maximize)
	{
		if (m_game_rules->IsTerminal(gs))
		{
			int score[2];
			m_game_rules->Score(gs, score);
			return { nullptr, zeroSumValue(score) };
		}
		/*if (depth >= max_depth)
		{
			int value[2];
			m_eval_function(gs, value, 2);
			return { nullptr, zeroSumValue(value) };
		}*/

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
		const auto mseconds = (long)std::chrono::duration_cast<std::chrono::milliseconds>(CLK::now() - tp_start).count();
		m_move_select_time.insert(mseconds / 1000.0f);
		return ml;
	}
	int zeroSumValue(int utility[]) const
	{
		return utility[m_player_number] - utility[1 - m_player_number];
	}
};

IGamePlayer* createMinMaxABPlayer_iterativeDeepening(int pn, float time_limit, EvalFunction_t evalFcn)
{
	return new MinMaxABPlayer_iterativeDeepening(pn, time_limit, evalFcn);
}