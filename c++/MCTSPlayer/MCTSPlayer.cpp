// MCTSPlayer.cpp : Defines the exported functions for the DLL application.
//
#include "pch.h"
#include "GamePlayer.h"
#include "GameRules.h"
#include <functional>
#include <chrono>
#include <random>
#include <map>
#include <fstream>
#include <sstream>
#include <math.h>
#include <object_pool.h>
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS  
#include <intrin.h>
#include <codecvt>
#include "object_pool_multisize.h"
#include <iostream>

#ifndef UNIT_TEST
#define TRACE(fmt, ...)
#endif

using CLK = std::chrono::high_resolution_clock;

auto EvalFunctionNumCards = [](const GameState* s, int value[], int num_players)
{
	const uint32_t *p = reinterpret_cast<const uint32_t*>(s);
	++p;
	for (int i = 0; i < num_players; ++i, ++p) {
		value[i] = 2 * (24 - __popcnt(*p));
	}
};

struct IMoveLimit
{
	virtual void start() = 0;
	virtual bool can_continue() = 0;
	virtual void release() = 0;
protected:
	virtual ~IMoveLimit(){}
};
struct MCTSPlayer : IGamePlayer
{
	struct StateNode;
	struct MoveNode
	{
		StateNode	*next;		//8
		short		numVisited;	//2
		short		moveIdx;	//2
		float		value[4];	//16
	};
	struct StateNode
	{
		GameState*	state;			//8
		MoveList*	moveList;		//8
		int			numVisited;		//4
		int			currentPlayer;	//4
		int			numMoves;		//4
		MoveNode	moves[1];		//next will follow

		void free(IGameRules *gr)
		{
			gr->ReleaseGameState(state);
			gr->ReleaseMoveList(moveList);
		}
	};
	using Path_t = std::vector< std::pair<StateNode*, MoveNode*> >;

	const int		PlayerNumber;
	const int		NumberOfPlayers;
	const int		NodesToAppendDuringExpansion = 1;
	const int		MaxPlayoutDepth = 20;
	const float		EERatio;
	const unsigned	SelectMoveTimeLimit = 0;
	const int		SelectMoveSimLimit = -1;
	EvalFunction_t	m_eval_function;
	IMoveLimit		*m_mv_limit;
	IGameRules		*m_game_rules;
	StateNode		*m_root = nullptr;
	std::default_random_engine	m_generator;
	ObjectPoolMultisize<sizeof(MoveNode), 1024> m_nodePool;
	int move_nbr = 0;
	Metric_t m_nodePool_usage = Histogram<long>();

	MCTSPlayer(int pn, int np, int max_depth, int expand_size, float ee_ratio, IMoveLimit *mv_limit) :
		PlayerNumber(pn),
		NumberOfPlayers(np),
		MaxPlayoutDepth(max_depth),
		NodesToAppendDuringExpansion(expand_size),
		m_eval_function(EvalFunctionNumCards),
		EERatio(ee_ratio),
		m_mv_limit(mv_limit)
	{
	}
	~MCTSPlayer()
	{
		m_mv_limit->release();
	}
	void	seed(unsigned long seed) { m_generator.seed(seed); }
	void	release() override { delete this; }
	void	startNewGame() override {}
	void	endGame() override
	{
		freeTree(m_root);
		m_root = nullptr;
		boost::get<Histogram<long>>(m_nodePool_usage).insert((long)m_nodePool.get_max_usage());
		m_nodePool.reset_stats();
	}
	void	setGameRules(IGameRules* gr)       override { m_game_rules = gr; }
	void	setEvalFunction(EvalFunction_t ef) override { m_eval_function = ef; }
	NamedMetrics_t	getGameStats()override
	{
		NamedMetrics_t nm;
		nm["node_pool_usage"] = m_nodePool_usage;
		return nm;
	}
	void	resetStats() override {}
	void	enableTrace(bool) override {}
	std::string getName() override { return "mcts"; }
	MoveList* selectMove(GameState* gs) override
	{
		if (m_game_rules->GetCurrentPlayer(gs) != PlayerNumber) {
			return m_game_rules->GetPlayerLegalMoves(gs, PlayerNumber);
		}
		m_root = findRootNode(m_game_rules->CopyGameState(gs));
		m_mv_limit->start();
		do {
			runSingleSimulation();
		} while (m_mv_limit->can_continue());

		if(0){
			static int move_nbr = 0;
			string filename = "C:\\MyData\\Projects\\gra_w_pana\\logs\\mcts_move_" + std::to_string(move_nbr) + ".gv";
			dumpTree(filename);
			++move_nbr;
		}
		return m_game_rules->SelectMoveFromList( m_root->moveList, selectMove(m_root, 0)->moveIdx );
	}

	void runSingleSimulation()
	{
		Path_t path = selection(m_root);
		const auto currentTreeSize = path.size() - 1;
		playOut(path);
		backpropagation(path);
		expansion(path, currentTreeSize);
	}

	MoveList* runNSimulations(GameState* gs, int totNumSimulations)
	{
		if (m_game_rules->GetCurrentPlayer(gs) != PlayerNumber) {
			return m_game_rules->GetPlayerLegalMoves(gs, PlayerNumber);
		}
		m_root = findRootNode(gs);
		while (totNumSimulations-- > 0) {
			runSingleSimulation();
		}
		return m_game_rules->SelectMoveFromList(m_root->moveList, selectMove(m_root, 0)->moveIdx);
	}

	StateNode* findRootNode(GameState * s)
	{
		if (nullptr == m_root) {
			TRACE(L"root empty, creating new one");
			return makeTreeNode(s);
		}
		auto rn = find(m_root, s);
		if (rn == nullptr) {
			TRACE(L"state not found, create new root");
			return makeTreeNode(s);
		}
		else {
			TRACE(L"state found");
			m_game_rules->ReleaseGameState(s);
		}
		return rn;
	}

	Path_t selection(StateNode* node)
	{
		const double C = 1.0;
		Path_t path;
		do {
			MoveNode *mn = selectMove(node, EERatio);
			path.push_back({ node, mn });
			node = mn != nullptr ? mn->next : nullptr;
		} while (node != nullptr);
		return path;
	}

	void playOut(Path_t& path)
	{
		int playoutDepth = 0;
		StateNode *node = path.back().first;
		MoveNode *move = path.back().second;
		if (!move) return;
		TRACE(L"starting playout @ state %s", m_game_rules->ToWString(node->state).c_str());
		for (;;)
		{
			Move* mv = m_game_rules->GetMoveFromList(node->moveList, move->moveIdx);
			node = makeTreeNode(m_game_rules->ApplyMove(node->state, mv, node->currentPlayer));
			move->next = node;
			if (!m_game_rules->IsTerminal(node->state) && ++playoutDepth < MaxPlayoutDepth)
			{
				const auto idx = selectOneOf(0, node->numMoves - 1);
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
		if ( m_game_rules->IsTerminal(finalNode.state)) {
			 m_game_rules->Score(finalNode.state,score);
		}
		else {
			m_eval_function(finalNode.state, score, 2);
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
			auto *pm = path[LastIdx].second;
			if (pm) pm->next = nullptr;
			for (auto i = LastIdx + 1; i < path.size(); ++i) {
				freeStateNode(path[i].first);
			}
		}
		else {
			//all nodes will be added
		}
		path.clear();
	}

	StateNode* find(StateNode*node, const GameState *s)
	{
		if (m_game_rules->AreEqual( node->state, s)) {
			return node;
		}
		StateNode *found = nullptr;
		for (auto & mv : node->moves) {
			found = mv.next != nullptr ? find(mv.next, s) : nullptr;
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
		const auto num_moves = node->numMoves;
		if (1 == num_moves) { return &node->moves[0]; }
		if (0 == num_moves) { return nullptr; }
		std::multimap<double, MoveNode*> moves;
		const auto cp = node->currentPlayer;

		for (auto & mv : node->moves)
		{
			//double value = mv.numVisited != 0 ? mv.value[cp] / mv.numVisited + C * sqrt(log(node->numVisited) / mv.numVisited) : 0.0;
			const double oo_mvVisited = 1.0 / (mv.numVisited + 1);
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
		//traceSelectMove(m_game_rules->ToWString(node->state), moves, m_game_rules->ToWString(it->second->mv), node);
		return it->second;
	}

	StateNode* makeTreeNode(GameState* gs)
	{
		static_assert(sizeof(StateNode) == 2 * sizeof(MoveNode), "sizes of StateNode and MoveNode not aligned");
		const auto current_player = m_game_rules->GetCurrentPlayer(gs);
		MoveList * move_list = m_game_rules->GetPlayerLegalMoves(gs, current_player);
		const int number_of_moves = m_game_rules->GetNumMoves(move_list);
		StateNode *node = reinterpret_cast<StateNode*> (m_nodePool.alloc(number_of_moves + 1));

		TRACE(L"++ node %p\n", node);
		node->state = gs;
		node->numVisited = 0;
		node->currentPlayer = current_player;
		node->numMoves = number_of_moves;
		node->moveList = move_list;
		for (int move_idx=0; move_idx<number_of_moves; ++move_idx) {
			node->moves[move_idx] = {nullptr, 0, short(move_idx), {.0f, .0f, .0f, .0f}};
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
		node->free(m_game_rules);
		m_nodePool.free(reinterpret_cast<uint8_t*>(node), node->numMoves+1);
	}

	void dumpTree(const string & filename)
	{
		//dot.exe -Tsvg mcts_tree_dump.gv -o mcts_tree_dump.svg -Goverlap=prism
		std::wofstream out(filename);
		std::locale loc(std::locale::classic(), new std::codecvt_utf8<wchar_t>);
		out.imbue(loc);
		out << L"digraph g {" << std::endl;
		dumpTreeNode(out, m_root);
		out << L"}" << std::endl;
	}

	void dumpTreeNode(std::wofstream& out, StateNode* sn)
	{
		const wstring name = m_game_rules->ToWString(sn->state);
		const int current_player = m_game_rules->GetCurrentPlayer(sn->state);
		out << L"\"" << sn->state << L"\" [label = \"name=" << name << L"\\nnum_visited = " << sn->numVisited << L"\\ncurrent_player = " << current_player;
		if (m_game_rules->IsTerminal(sn->state)) {
			int score[4];
			m_game_rules->Score(sn->state, score);
			out << L"\\nscore = " << score[0] << L" , " << score[1];
		}
		out << "\"]" << std::endl;

		for (auto & mv : sn->moves)
		{
			if (!mv.next) continue;
			Move* mvmv = m_game_rules->GetMoveFromList(sn->moveList, mv.moveIdx);
			const wstring mv_name = m_game_rules->ToWString(mvmv);
			out << L"\"" << sn->state << L"\" -> \"" << mv.next->state << L"\" [label = \"name=" << mv_name << L"\\nnum_visited = " << mv.numVisited;
			out << L"\\nvalue = ";
			for (int i = 0; i < NumberOfPlayers; ++i) {
				out << mv.value[i] / mv.numVisited << L" , ";
			}
			out << "\"]" << std::endl;
			dumpTreeNode(out, mv.next);
		}
	}

	void traceSelectMove(const wstring& state, const std::multimap<double, MoveNode*>& moves, const wstring& selected, StateNode* sn)
	{
		std::wstringstream ss;
		ss << state << L" moves : ";
		for (auto & it : moves)
		{
			Move* mv = m_game_rules->GetMoveFromList(sn->moveList, it.second->moveIdx);
			ss << m_game_rules->ToWString(mv) << L" : " << it.first << L" | ";
		}
		ss << L"selected " << selected << std::endl;
		TRACE(ss.str().c_str());
	}
};

struct NumSimMoveLimit : IMoveLimit
{
	const int SimLimit;
	int numSim;
	NumSimMoveLimit(int simLimit) : SimLimit(simLimit) {}
	void start() override { numSim=0;}
	bool can_continue() override { return ++numSim < SimLimit; }
	void release() override { delete this; }
};
struct TimeMoveLimit : IMoveLimit
{
	const unsigned TimeLimitmiliSec;
	std::chrono::time_point<CLK> tp_start;
	TimeMoveLimit(double timeLimit) : TimeLimitmiliSec(unsigned(timeLimit*1000)) {}
	void start() override { tp_start = CLK::now(); }
	bool can_continue() override
	{
		const auto mseconds = std::chrono::duration_cast<std::chrono::milliseconds>(CLK::now() - tp_start).count();
		return mseconds < TimeLimitmiliSec;
	}
	void release() override { delete this; }
};

IMoveLimit* createMoveLimit(const PlayerConfig_t& pc)
{
	const auto move_time_limit = pc.get_optional<float>("move_time_limit");
	const auto move_sim_limit = pc.get_optional<int>("move_sim_limit");
	if (move_sim_limit) {
		return new NumSimMoveLimit(move_sim_limit.get());
	}
	return new TimeMoveLimit(move_time_limit.get_value_or(1.0));
}

IGamePlayer* createMCTSPlayer(int player_number, const PlayerConfig_t& pc)
{
	const auto seed = pc.get_optional<unsigned long>("random_seed").get_value_or(unsigned(CLK::now().time_since_epoch().count()));
	const auto number_of_players = pc.get_optional<int>("number_of_players").get_value_or(2);
	const auto max_depth = pc.get_optional<int>("playout_depth").get_value_or(10);
	const auto expand_size = pc.get_optional<int>("expand_size").get_value_or(1);
	const auto e_e_ratio = pc.get_optional<float>("explore_exploit_ratio").get_value_or(1.0);
	auto mctsp = new MCTSPlayer(player_number, number_of_players, max_depth, expand_size, e_e_ratio, 
				createMoveLimit(pc));
	mctsp->seed(seed);
	return mctsp;
}

BOOST_DLL_ALIAS(
	createMCTSPlayer,	// <-- this function is exported with...
	createPlayer				// <-- ...this alias name
)
