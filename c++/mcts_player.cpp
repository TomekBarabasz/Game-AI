#include "pch.h"
#include "GamePlayer.h"
#include "GameState.h"
#include <vector>
#include <functional>
#include <random>
#include "object_pool.h"
#include <map>
#include <math.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>

#ifdef UNIT_TEST
#define _CRTDBG_MAP_ALLOC  
#include "CppUnitTestLogger.h"
using Microsoft::VisualStudio::CppUnitTestFramework::Logger;
#define TRACE(fmt, ...){\
	static wchar_t tmp[256];\
	swprintf_s(tmp, fmt, ##__VA_ARGS__);\
	Logger::WriteMessage(tmp);}
#else
#define TRACE(fmt, ...)
#endif
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
	MCTS_GamePlayer(int playerNum, std::function<void(const IGameState*, int*)> eval, int nodesToAppendDuringExp=-1, unsigned seed=0) :
		PlayerNum(playerNum), 
		NodesToAppendDuringExpansion(nodesToAppendDuringExp),
		Eval(eval)
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
	const int NodesToAppendDuringExpansion = 1;
	std::default_random_engine	m_generator;
	ObjectPool<StateNode, FreeObjectByDelete, TrackAliveObjects> m_nodePool;
	StateNode *m_root=nullptr;
	std::function<void(const IGameState*, int*)> Eval;

	IMove* selectMove(const IGameState* gs, PlayerStats_t* ps) override
	{
		if (gs->CurrentPlayer() != PlayerNum) {
			return gs->GetPlayerLegalMoves(PlayerNum)[0];
		}
		gs->AddRef(); //only once, will be stored in game search tree
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

	IMove* runNSimulations(const IGameState* gs, int totNumSimulations)
	{
		if (gs->CurrentPlayer() != PlayerNum) {
			return gs->GetPlayerLegalMoves(PlayerNum)[0];
		}

		while(totNumSimulations-- > 0) {
			runSingleSimulation(gs);
		}
		return selectMove(m_root, 0)->mv;
	}

	StateNode* findRootNode(const IGameState * s, StateNode *node)
	{
		if (nullptr == node) {
			std::cout << "root empty, creating new one" << std::endl;
			return makeTreeNode(s);
		}
		auto rn = find(node, s->CurrentPlayer(), s->Hash());
		if (rn==nullptr) {
			//std::cout << "state not found, create new root" << std::endl;
			return makeTreeNode(s);
		}else {
			//std::cout << "state found" << std::endl;
		}
		return rn;
	}

	Path_t selection(StateNode* node)
	{
		const double C = 1.0;
		Path_t path;
		do {
			MoveNode *mn = selectMove(node, C);
			path.push_back({ node, mn });
			node = mn != nullptr ? mn->next : nullptr;
		} while (node != nullptr);
		return path;
	}

	void playOut(Path_t& path)
	{
		const int MaxPlayoutDepth = 20;
		int playoutDepth = 0;
		StateNode *node = path.back().first;
		MoveNode *move = path.back().second;
		if (!move) return;
		TRACE(L"starting playout @ state %s", node->state->ToString().c_str());
		for (;;)
		{
			node = makeTreeNode(node->state->Apply(move->mv, node->currentPlayer));
			move->next = node;
			if (!node->state->IsTerminal() && ++playoutDepth < MaxPlayoutDepth)
			{
				const auto idx = selectOneOf(0, node->moves.size() - 1);
				move = &node->moves[idx];
				path.push_back({ node, move });
			}
			else {
				path.push_back({ node, nullptr });
				break;
			}
		}
	}

	void backpropagation(Path_t& path)
	{
		int score[4];
		float value[4];
		auto & finalNode = *path.rbegin()->first;
		if (finalNode.state->IsTerminal()) {
			finalNode.state->Score(score);
		}else {
			Eval(finalNode.state, score);
		}
		for (int i = 0; i < 4; ++i) { value[i] = score[i] / 100.0f; }
		++finalNode.numVisited;
		for (auto it = path.rbegin() + 1; it != path.rend(); ++it)
		{
			++it->first->numVisited;
			++it->second->numVisited;
			for (int i = 0; i < 4; ++i) {
				it->second->value[i] += value[i];
			}
		}
	}

	void expansion(Path_t& path, size_t lastStoredNode)
	{
		if (NodesToAppendDuringExpansion > 0) 
		{
			auto & last = path[lastStoredNode];
			const auto LastIdx = __min(lastStoredNode + NodesToAppendDuringExpansion, path.size() - 1);
			path[LastIdx].second->next = nullptr;
			for (auto i = LastIdx + 1; i < path.size(); ++i) {
				freeStateNode(path[i].first);
			}
		}else{
			//all nodes will be added
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

	void traceSelectMove(const wstring& state, const std::multimap<double, MoveNode*>& moves, const wstring& selected)
	{
		std::wstringstream ss;
		ss << state << L" moves : ";
		for (auto & it : moves)
		{
			ss << it.second->mv->toString() << L" : " << it.first << L" | ";
		}
		ss << L"selected " << selected << std::endl;
		TRACE(ss.str().c_str());
	}

	MoveNode* selectMove(StateNode* node, double C)
	{
		const auto num_moves = node->moves.size();
		if (1 == num_moves) { return &node->moves[0]; }
		if (0 == num_moves) { return nullptr; }
		std::multimap<double, MoveNode*> moves;
		const auto cp = node->currentPlayer;

		for (auto & mv : node->moves)
		{
			//double value = mv.numVisited != 0 ? mv.value[cp] / mv.numVisited + C * sqrt(log(node->numVisited) / mv.numVisited) : 0.0;
			const double oo_mvVisited = 1.0  /(mv.numVisited + 1);
			const double value = mv.value[cp] * oo_mvVisited + C * sqrt(log(node->numVisited + 1) * oo_mvVisited);
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
		traceSelectMove(node->state->ToString(), moves, it->second->mv->toString());
		return it->second;
	}

	StateNode* makeTreeNode(const IGameState* gs)
	{
		auto node = m_nodePool.alloc();
		TRACE(L"++ node %p\n", node);
		node->state = gs;
		//gs->AddRef();
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
		TRACE(L"-- node %p\n", node);
		node->free();
		m_nodePool.free(node);
	}

	void dumpTree(const string & filename)
	{
		//dot.exe -Tsvg mcts_tree_dump.gv -o mcts_tree_dump.svg -Goverlap=prism
		std::wofstream out(filename);
		out << L"digraph g {" << std::endl;
		dumpTreeNode(out, m_root);
		out << L"}" << std::endl;
	}
	void dumpTreeNode(std::wofstream& out, StateNode* sn)
	{
		const wstring name = sn->state->ToString();
		out << L"\"" << name << L"\" [label = \"name=" << name << L"\\nnum_visited = " << sn->numVisited << L"\\ncurrent_player = " << sn->state->CurrentPlayer();
		if (sn->state->IsTerminal()) {
			int score[4];
			sn->state->Score(score);
			out << L"\\nscore = " << score[0] << L" , " << score[1];
		}
		out << "\"]" << std::endl;
		for (auto & mv : sn->moves)
		{
			if (!mv.next) continue;
			const wstring tname = mv.next->state->ToString();
			out << L"\"" << name << L"\" -> \"" << tname << L"\" [label = \"name=" << mv.mv->toString() << L"\\nnum_visited = " << mv.numVisited;
			out << L"\\nvalue = " << mv.value[0] << L" , " << mv.value[1] << "\"]" << std::endl;
			dumpTreeNode(out, mv.next);
		}
	}
};

IGamePlayer* createMCTSPlayer(int playerNum, int numPlayers, std::function<void(const IGameState*, int*)> eval, unsigned seed)
{
	if (0==seed) {
		seed = unsigned(CLK::now().time_since_epoch().count());
	}
	return new MCTS_GamePlayer(playerNum, eval, 1, seed);
}
