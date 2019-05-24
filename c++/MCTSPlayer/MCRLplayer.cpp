#include "pch.h"
#include "GamePlayer.h"
#include "GameRules.h"
#include "object_pool_multisize.h"
#include <random>
#include <codecvt>
#include <iostream>
#include <fstream>
#define ENABLE_TRACE
#include <Trace.h>

namespace MCRL
{
	struct MCRLConfig
	{
		int			PlayerNumber;
		int			NumberOfPlayers;
		float		EERatio;
		unsigned	seed;
		float		StateLoopPenalty;
		float		Gamma;
		float		MovePenalty;
		string		outDir;
		string		traceMoveFilename;
		string		policyFilename;
	};
#pragma pack (push,1)
	struct StateNode;
	struct MoveNode
	{
		//StateNode* next;
		uint32_t numVisited : 27;//4b
		uint32_t moveIdx	: 5; //-b
		float	 accum;		  	 //4b
		void  update(float value) { accum += value; ++numVisited; }
		float getValue() const { return numVisited != 0 ? accum / numVisited : 0; }
	};

	struct StateNode
	{
		MoveList*		moveList;		//8
		uint32_t		numVisited : 27;//4b
		uint32_t		numMoves   : 5;	//-b
		MoveNode		moves[1];		//8b*numMoves, next will follow

		std::tuple<MoveNode*, Move*, StateNode*> getBestMove(IGameRules *gr);
		static StateNode* create(GameState* gs, IGameRules* gameRules, std::function<StateNode*(int)> alloc)
		{
			const auto current_player = gameRules->GetCurrentPlayer(gs);
			MoveList * move_list = gameRules->GetPlayerLegalMoves(gs, current_player);
			const int number_of_moves = gameRules->GetNumMoves(move_list);
			assert(number_of_moves < (2 << 5));
			StateNode *node = alloc(number_of_moves);
			node->numVisited = 0;
			node->numMoves = number_of_moves;
			node->moveList = move_list;
			for (int move_idx = 0; move_idx < number_of_moves; ++move_idx) {
				node->moves[move_idx] = { 0, unsigned char(move_idx), .0f };
			}
			return node;
		}
	};
#pragma pack(pop)
	using Path_t = std::vector< std::tuple<StateNode*, MoveNode*, float> >;
	using CLK = std::chrono::high_resolution_clock;

	struct Player : IGamePlayer
	{
		//ObjectPoolMultisize<sizeof(StateNode) + 4*sizeof(MoveNode), 4096> m_nodePool;	//1 chunk = state + 5 moves

		Player(MCRLConfig cfg, ITrace *trace) : m_cfg(cfg), m_trace(trace)
		{}
		virtual ~Player()
		{
			releaseStateNodes();
			m_game_rules->Release();
			m_trace->release();
		}
		void startNewGame() override
		{}
		void endGame(int score, GameResult result) override
		{
			backpropagate(m_currentPath, score, result);
			m_currentPath.clear();
		}
		void setGameRules(IGameRules* gr) override
		{
			m_game_rules = gr;
			gr->AddRef();
		}
		void setEvalFunction(EvalFunction_t) override
		{}

		MoveList*	selectMove(GameState* gs) override
		{
			if (m_game_rules->GetCurrentPlayer(gs) != m_cfg.PlayerNumber) {
				return m_game_rules->GetPlayerLegalMoves(gs, m_cfg.PlayerNumber);
			}
			string sth = m_game_rules->ToString(gs);
			auto it = m_visitedStates.find(sth);
			int selectedMoveIdx;
			StateNode *stateNode;
			
			if (it == m_visitedStates.end())
			{
				stateNode = StateNode::create(gs, m_game_rules, [this](int nom) { return _alloc(nom); });
				if (stateNode->numMoves > 1) {
					m_visitedStates[sth] = stateNode;
				}
				selectedMoveIdx = (int)selectOneOf(0, (int)stateNode->numMoves-1);
			}
			else
			{
				stateNode = it->second;
				selectedMoveIdx = selectMove(stateNode, m_cfg.EERatio);
			}
			MoveList *moves = stateNode->moveList;
			auto *selected = m_game_rules->SelectMoveFromList(moves, selectedMoveIdx);
			if (stateNode->numMoves > 1)
			{
				m_currentPath.push_back({ stateNode, stateNode->moves + selectedMoveIdx, -m_cfg.MovePenalty });
			}else {
				_free(stateNode);
			}
			return selected;
		}
		NamedMetrics_t	getGameStats() override
		{
			if (!m_cfg.policyFilename.empty()) {
				saveState(m_cfg.policyFilename);
			}
			return NamedMetrics_t();
		}
		std::string getName() override { return "mcrl_player"; }
		void		resetStats() override {}
		void		release() override { delete this; }

		StateNode*	_alloc(int number_of_moves);
		void		_free(StateNode* node);
		size_t		selectOneOf(size_t first, size_t last);
		void saveState(string filename);
		void loadState(string filename);
		void		seed(unsigned long seed) { m_generator.seed(seed); }
		int			selectMove(StateNode* gs, double C);
		void		releaseStateNodes();
		void		backpropagate(const Path_t& path, int score, GameResult result);

		const MCRLConfig m_cfg;
		IGameRules*	m_game_rules;
		Path_t		m_currentPath;
		std::map<string, StateNode*> m_visitedStates;
		std::default_random_engine	m_generator;
		ITrace			*m_trace;
	};

	void Player::releaseStateNodes()
	{
		for (auto kv : m_visitedStates)
		{
			StateNode *sn = kv.second;
			m_game_rules->ReleaseMoveList(sn->moveList);
			_free(sn);
		}
	}

	int Player::selectMove(StateNode* sn, double C)
	{
		if (1 == sn->numMoves) return 0;
		std::multimap<double, MoveNode*> moves;

		for (unsigned mi = 0; mi < sn->numMoves; ++mi)
		{
			auto *mv = sn->moves + mi;
			const double oo_mvVisited = 1.0 / (mv->numVisited + 1);
			const double value = mv->getValue() * oo_mvVisited + C * sqrt(log(sn->numVisited + 1) * oo_mvVisited);
			moves.insert({ value, mv });
		}
		auto it = moves.rbegin();
		auto it2 = it;
		int cnt = 1;
		for (++it2; it2 != moves.rend(); ++it2, ++cnt) {
			if (it2->first != it->first) break;
		}
		return (int)selectOneOf(0, cnt - 1);
	}

	void Player::backpropagate(const Path_t& path, int score, GameResult result)
	{
		std::get<2>(m_currentPath.back()) = result != GameResult::AbortedByStateLoop ? (score-50)/50.0f : -m_cfg.StateLoopPenalty;
		//std::get<2>(m_currentPath.back()) = result != GameResult::AbortedByStateLoop ? score/100.0f : -m_cfg.StateLoopPenalty;
		
		float discountedReturn = 0.0f;
		for (int idx = (int)path.size()-1; idx >= 0; --idx)
		{
			discountedReturn = std::get<2>(path[idx]) + m_cfg.Gamma * discountedReturn;
			std::get<1>(path[idx])->update(discountedReturn);
			auto *sn = std::get<0>(path[idx]);
			sn->numVisited++;
			/*auto Vnext = sn->moves[0].getValue();
			for (int aidx=1;aidx < sn->numMoves;++aidx) {
				const auto av = sn->moves[aidx].getValue();
				Vnext = __max(Vnext, av);
			}*/
		}
	}

	StateNode* Player::_alloc(int number_of_moves)
	{
		//const size_t num_chunks = (number_of_moves * sizeof(MoveNode)) / m_nodePool.ChunkSize + 1;
		//return reinterpret_cast<StateNode*>(m_nodePool.alloc(num_chunks));
		uint8_t *raw = new uint8_t[sizeof(StateNode) + (number_of_moves - 1) * sizeof(MoveNode)];
		return reinterpret_cast<StateNode*>(raw);
	}

	void Player::_free(StateNode* node)
	{
		//const size_t num_chunks = (node->numMoves * sizeof(MoveNode)) / m_nodePool.ChunkSize + 1;
		//m_nodePool.free(reinterpret_cast<uint8_t*>(node), num_chunks);
		uint8_t *raw = reinterpret_cast<uint8_t*>(node);
		delete[] raw;
	}

	size_t Player::selectOneOf(size_t first, size_t last)
	{
		std::uniform_int_distribution<size_t> distribution(first, last);
		return distribution(m_generator);
	}

	void Player::saveState(string filename)
	{
		//std::wofstream out(filename);
		//std::locale loc(std::locale::classic(), new std::codecvt_utf8<wchar_t>);
		//out.imbue(loc);
		std::ofstream out(filename);

		for (auto kv : m_visitedStates)
		{
			auto *sn = kv.second;
			out << "state " << kv.first << " visited " << sn->numVisited << " moves " << sn->numMoves << std::endl;
			for (unsigned mi=0;mi<sn->numMoves;++mi)
			{
				out << "move idx " << sn->moves[mi].moveIdx << " visited " << sn->moves[mi].numVisited << " value " << sn->moves[mi].accum << std::endl;
			}
		}
	}

	void Player::loadState(string filename)
	{
		std::ifstream out(filename);
	}

	IGamePlayer* createMCRLPlayer(int player_number, const PlayerConfig_t& pc)
	{
		MCRLConfig cfg;

		cfg.PlayerNumber = player_number;
		cfg.seed = pc.get_optional<unsigned long>("random_seed").get_value_or(unsigned(CLK::now().time_since_epoch().count()));
		cfg.NumberOfPlayers = pc.get_optional<int>("number_of_players").get_value_or(2);
		cfg.EERatio = pc.get_optional<float>("explore_exploit_ratio").get_value_or(1.0f);
		cfg.Gamma = pc.get_optional<float>("discount_factor").get_value_or(1.0f);
		cfg.MovePenalty = pc.get_optional<float>("move_penalty").get_value_or(0.0f);
		cfg.StateLoopPenalty = pc.get_optional<float>("cycle_penalty").get_value_or(1.0f);

		cfg.traceMoveFilename = pc.get_optional<string>("trace_move_filename").get_value_or("");
		cfg.outDir = pc.get_optional<string>("out_dir").get_value_or("");
		cfg.policyFilename = pc.get_optional<string>("policy_filename").get_value_or("");
		auto logger = ITrace::createInstance(pc.get_optional<string>("trace").get_value_or(""), cfg.outDir);
		
		return new Player(cfg, logger);
	}
}
