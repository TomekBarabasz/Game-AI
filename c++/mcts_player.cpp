#include "pch.h"
#include "GamePlayer.h"
#include "GameRules.h"
#include <vector>
#include <functional>
#include <random>
#include "object_pool.h"
#include <map>
#include <math.h>
#include <chrono>
#include <iostream>

using std::vector;
using CLK = std::chrono::high_resolution_clock;

struct MCTS_GamePlayer : IGamePlayer
{
	struct StateNode;
	struct MoveNode
	{
		IMove		*mv;
		StateNode	*next;
		int			numVisited;
		float		value[4];
	};
	struct StateNode
	{
		const IGameState *state;
		int				 currentPlayer;
		int				 numVisited;
		vector<MoveNode> moves;
		
		void free()
		{
			state->Release();
			for (auto m : moves) m.mv->release();
		}
	};
	using Path_t = vector< std::pair<StateNode*, MoveNode*> >;
	MCTS_GamePlayer(int playerNum, std::function<void(const IGameState*, int*)> eval, unsigned seed) : 
		PlayerNum(playerNum), Eval(eval)
	{
		m_generator.seed(seed);
	}

	~MCTS_GamePlayer()
	{
		freeTree(m_root);
		m_nodePool.release();
	}
	void release() override { delete this; }
	void getGameStats(PlayerStats_t& ps) override {}

	const int PlayerNum;
	std::default_random_engine	m_generator;
	ObjectPool<StateNode, FreeObjectByDelete, TrackAliveObjects> m_nodePool;
	StateNode *m_root=nullptr;
	std::function<void(const IGameState*, int*)> Eval;

	IMove* selectMove(const IGameState* gs, PlayerStats_t* ps) override
	{
		if (gs->CurrentPlayer() != PlayerNum) {
			return gs->GetPlayerLegalMoves(PlayerNum)[0];
		}
		
		const auto t0 = CLK::now();
		int numSimulationsDone = 0;
		do {
			std::cout << "starting simulation " << numSimulationsDone + 1 << std::endl;
			runSingleSimulation(gs);
			++numSimulationsDone;
		//} while ((CLK::now() - t0).count() < 10);
		} while (numSimulationsDone < 10);
		return selectMove(m_root, 0)->mv;
	}

	void runSingleSimulation(const IGameState *s)
	{
		m_root = findRootNode(s, m_root);
		Path_t path = selection(m_root);
		const auto currentTreeSize = path.size()-1;
		playOut(path);
		backpropagation(path);
		expansion(path, currentTreeSize);
	}

	StateNode* findRootNode(const IGameState * s, StateNode *node)
	{
		if (nullptr == node) {
			std::cout << "root empty, creating new one" << std::endl;
			return makeTreeNode(s);
		}
		auto rn = find(node, s->CurrentPlayer(), s->Hash());
		if (rn==nullptr) {
			std::cout << "state not found, create new root" << std::endl;
			return makeTreeNode(s);
		}else {
			std::cout << "state found" << std::endl;
		}
		return rn;
	}

	Path_t selection(StateNode* node)
	{
		const double C = 0.2;
		Path_t path;
		std::multimap<double, MoveNode*> moves;
		do
		{
			MoveNode *mn = selectMove(node, C);
			path.push_back({ node, mn });
			++node->numVisited;
			++mn->numVisited;
			node = mn->next;
		} while (node != nullptr);
		return path;
	}

	void playOut(Path_t& path)
	{
		const int MaxPlayoutDepth = 50;
		int playoutDepth = 0;
		StateNode *node = path.back().first;
		MoveNode *move = path.back().second;
		int score[4];
		for (;;)
		{
			node = makeTreeNode(node->state->Apply(move->mv, node->currentPlayer));
			move->next = node;
			if (!node->state->IsTerminal())
			{
				if (++playoutDepth < MaxPlayoutDepth) {
					const auto idx = selectOneOf(0, node->moves.size() - 1);
					move = &node->moves[idx];
					path.push_back({ node, move });
				}
				else
				{
					Eval(node->state, score);
					freeStateNode(node);
					m_nodePool.free(node);
					break;
				}
			}
			else {
				node->state->Score(score);
				freeStateNode(node);
				break;
			}
		}
		for (int i = 0; i < 4; ++i) { move->value[i] = (float)score[i]; }
	}

	void backpropagation(Path_t& path)
	{
		const auto value = path.rbegin()->second->value;
		for (auto it = path.rbegin() + 1; it != path.rend(); ++it)
		{
			for (int i = 0; i < 4; ++i) {
				it->second->value[i] += value[i];
			}
		}
	}

	void expansion(Path_t& path, size_t lastStoredNode)
	{
		const int NodesToAppend = 1;
		auto & last = path[lastStoredNode];
		const auto LastIdx = __min(lastStoredNode + NodesToAppend, path.size() - 1);
		path[LastIdx].second->next = nullptr;
		for (auto i=LastIdx;i<path.size();++i) {
			freeStateNode(path[i].first);
		}
		path.clear();
	}

	StateNode* find(StateNode*node, int playerNum, const GameStateHash_t& hash)
	{
		if (node->currentPlayer == playerNum && node->state->Hash() == hash) {
			return node;
		}
		StateNode *found = nullptr;
		for (auto & mv : node->moves) {
			found = mv.next != nullptr ? find(mv.next, playerNum, hash) : nullptr;
			if (nullptr != found) break;
		}
		freeStateNode(node);
		return found;
	}
	
	size_t selectOneOf(size_t first, size_t last)
	{
		std::uniform_int_distribution<size_t> distribution(first, last);
		return distribution(m_generator);
	}
	
	MoveNode* selectMove(StateNode* node, double C)
	{
		if (node->moves.size()==1) {
			return &node->moves[0];
		}
		std::multimap<double, MoveNode*> moves;
		const auto cp = node->currentPlayer;

		for (auto & mv : node->moves)
		{
			double value = mv.numVisited != 0 ? mv.value[cp] / mv.numVisited + C * sqrt(log(node->numVisited) / mv.numVisited) : 0.0;
			moves.insert({ value, &mv });
		}
		auto it = moves.rbegin();
		auto it2 = it;
		int cnt = 1;
		for (++it2; it2 != moves.rend(); ++it2, ++cnt) {
			if (it2->first != it->first) break;
		}
		auto idx = selectOneOf(0, cnt - 1);
		//return (it + idx)->second;
		while (idx-- > 0) ++it;
		return it->second;
	}

	StateNode* makeTreeNode(const IGameState* gs)
	{
		auto node = m_nodePool.alloc();
		node->state = gs;
		gs->AddRef();
		node->numVisited = 0;
		node->moves.clear();
		node->currentPlayer = gs->CurrentPlayer();
		MoveList mv = gs->GetPlayerLegalMoves(node->currentPlayer);
		for (auto m : mv) {
			node->moves.push_back({ m, nullptr, 0 });
		}
		return node;
	}

	void freeTree(StateNode* node)
	{
		if (node == nullptr) return;
		for (auto & mv : node->moves) {
			freeTree(mv.next);
		}
		freeStateNode(node);
	}

	void freeStateNode(StateNode* node)
	{
		node->free();
		m_nodePool.free(node);
	}
};

IGamePlayer* createMCTSPlayer(int playerNum, int numPlayers, std::function<void(const IGameState*, int*)> eval, unsigned seed)
{
	if (0==seed) {
		seed = unsigned(CLK::now().time_since_epoch().count());
	}
	return new MCTS_GamePlayer(playerNum, eval, seed);
}
