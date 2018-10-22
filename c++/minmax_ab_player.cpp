#include "pch.h"
#include "GamePlayer.h"
#include "GameRules.h"
#include <deque>
#include <vector>
#include <functional>
#include "GamePlayer.h"

struct MinMaxABPlayer_2p : IGamePlayer
{
	struct Node
	{
		IGameState *state;
		int currentPlayer, depth, bestValue, alpha, beta, currMove;
		IMove *bestMove = nullptr;
		MoveList mvList;

		~Node()
		{
			state->Release();
			for (auto *mv : mvList) mv->release();
		}
	};
	struct Action
	{
		IMove *mv;
		int value;
	};
	MinMaxABPlayer_2p(int pn, int maxDepth, std::function<void(const IGameState*,int[])> eval) : PlayerNum(pn), MaxDepth(maxDepth), Eval(eval) {}
	void	release() override { delete this; }
	void	getGameStats(PlayerStats_t& ps) override {}
	Action selectMoveRec(const IGameState* gs, int curPlayer, int depth, int alpha, int beta, bool Maximize)
	{
		if (gs->IsTerminal())
		{
			int score[2];
			gs->Score(score);
			return { nullptr, zeroSumValue(score) };
		}
		if (depth >= MaxDepth)
		{
			int value[2];
			Eval(gs, value);
			return { nullptr, zeroSumValue(value) };
		}
		
		MoveList moves = gs->GetPlayerLegalMoves(curPlayer);
		if (0 == depth && moves.size() == 1)
		{
			return {moves[0], 0};
		}
		Action bestAction{ nullptr, Maximize ? -1000 : 1000 };
		for (auto *mv : moves)
		{
			auto *ngs = gs->Apply(mv, curPlayer);
			//alpha = highest value ever - best choice for max player
			//beta  =  lowest value ever - best choice for min player
			Action a = selectMoveRec(ngs, 1 - curPlayer, depth + 1, alpha, beta, !Maximize);
			if (a.mv) a.mv->release();
			ngs->Release();
			if (Maximize)
			{
				if (a.value > bestAction.value)
				{
					bestAction = {mv, a.value};
				}
				if (bestAction.value >= beta) break;
				alpha = __max(alpha, bestAction.value);
			}
			else
			{
				if (a.value < bestAction.value)
				{
					bestAction = {mv, a.value};
				}
				if (bestAction.value <= alpha) break;
				beta = __min(beta, bestAction.value);
			}
		}
		for (auto *mv : moves)
			if (mv != bestAction.mv)
				mv->release();

		return bestAction;
	}

	IMove* selectMove(const IGameState* gs, PlayerStats_t* ps) override
	{
		return selectMoveRec(gs, PlayerNum, 0, -1000, 1000, true).mv;
	}

#if 0
	IMove* selectMove2(const IGameState* s)
	{
		std::vector<Node*>	searchTree;

		searchTree.push_back(new Node{ s, 0, -100, -101, 101, 0 });
		Node *top = searchTree.back();
		top->mvList = top->state->GetPlayerLegalMoves( top->currentPlayer );

		while (!searchTree.empty())
		{
			Node * top = searchTree.back();
			bool stop = false;
			int score[2];
			if (top->state->IsTerminal())
			{
				top->state->Score(score);
				stop = true;
			}
			else if (top->depth >= MaxDepth)
			{
				Eval(top->state, score);
				stop = true;
			}

			if (stop)
			{
				delete top;
				searchTree.pop_back();
				top = searchTree.back();
				const int value = zeroSumValue(score);
				if (value > top->bestValue )
				{
					top->bestMove = top->mvList[ top->currMove ];
					top->bestValue = value;
				}
				++top->currMove;
			}
			else
			{
				if (top->currMove < top->mvList.size())
				{
					auto *mv = top->mvList[top->currMove];
					auto ns = top->state->Apply(mv, top->currentPlayer);
					auto np = (top->currentPlayer + 1) % 2;
					Node *next = new Node{ ns, np, top->depth + 1, top->bestValue, top->alpha, top->beta, 0 };
					next->mvList = ns->GetPlayerLegalMoves(np);
					searchTree.push_back(next);
				}
				else
				{
					searchTree.pop_back();
					if (searchTree.empty()) break;
					const int bestV = top->bestValue;
					delete top;
					top = searchTree.back();
					if (bestV > top->bestValue)
					{
						top->bestValue = bestV;
						top->bestMove = top->mvList[top->currMove];
					}
				}
			}
		}
		for (auto mv : top->mvList)	{
			if (mv != top->bestMove) mv->release();
		}
		auto bm = top->bestMove;
		top->mvList.clear();
		delete top;
		return bm;
	}
#endif
	int zeroSumValue(int utility[]) const
	{
		return utility[PlayerNum] - utility[1 - PlayerNum];
	}

	const int	PlayerNum;
	const int	MaxDepth;
	std::function<void(const IGameState*, int*)> Eval;
};

IGamePlayer* createMinMaxABGamePlayer(int playerNum, int numPlayers, int maxDepth, std::function<void(const IGameState*,int[])> eval)
{
	return new MinMaxABPlayer_2p(playerNum, maxDepth, eval);
}
