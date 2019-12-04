#pragma once
#include <string>
#include <random>
#include <set>
#include "GamePlayer.h"
#include "object_pool_multisize.h"
#include <boost/bimap.hpp>
#include "Trace.h"

struct Move;
using std::string;
using std::wstring;
using namespace Trace;

namespace MC
{
	struct MCTSConfig
	{
		int			PlayerNumber;
		int			NumberOfPlayers;
		int			NodesToAppendDuringExpansion = 1;
		bool		ExpandFromLastPermanentNode;
		int			MaxPlayoutDepth = 20;
		float		EERatio;
		unsigned	seed;
		int			CyclePenalty = 50;
		string		outDir;
		string		traceMoveFilename;
		string		gameTreeFilename;
		float		BestMoveValueEps = 0.005f;
	};
	struct IMoveLimit
	{
		virtual void start() = 0;
		virtual bool can_continue() = 0;
		virtual void release() = 0;
	protected:
		virtual ~IMoveLimit() {}
	};

	struct StateNode;
	struct ValidMoveList;
#pragma pack (push,1)
	struct MoveNode
	{
		StateNode		*next;		//8
		unsigned char	numVisited;	//1
		unsigned char	moveIdx;	//1
		unsigned short	probability;//2
		float			value[4];	//16
		float getWeight(int player) const {
			//return numVisited != 0 ? value[player] * probability / numVisited : 0;
			return value[player] * get_probability() / (1.0f + numVisited);
		}
		std::tuple<float,float> getWeightEx(int player) const {
			const float ooNumVisited = 1.0f / (1.0f + numVisited);
			return { value[player] * get_probability() * ooNumVisited,ooNumVisited };
		}

		void set_probability(float p) {
			probability = static_cast<unsigned short>(p * 65535);
		}
		float get_probability() const {
			return probability / 65535.0f;
		}
	};

	struct StateNode
	{
		GameState*		state;			//8
		MoveList*		moveList;		//8
		int				numVisited;		//4
		unsigned char	currentPlayer;	//1
		unsigned char	occured : 1;	//1bit 1 means this state occured during real game
		unsigned char	terminal : 1;	//1bit
		unsigned char	temporary : 1;	//1 : 0=this state is part of the game tree, 1=created during playout, not yet included
		unsigned char	occupied;		//1
		unsigned short	lastVisitId;	//2
		unsigned char	numMoves;		//1
		unsigned char	pad[2];			//4
		MoveNode		moves[1];		//next will follow

		void free(IGameRules *gr);
		std::tuple<MoveNode*, Move*, StateNode*> getBestMove(IGameRules *gr);
		ValidMoveList* listValidMoves(unsigned short visit_id, std::function<ValidMoveList*(int)> alloc) const;
		static StateNode* create(GameState*, IGameRules* gameRules, std::function<StateNode*(int)>);
	};

	struct ValidMoveList
	{
		bool empty() const { return 0 == size; }
		void remove(uint8_t value)
		{
			//NOTE: assumes that value is always present in the array!
			--size;
			for (int i = 0; i < size; ++i) {
				if (indices[i] == value) {
					indices[i] = indices[size];
					break;
				}
			}
		}
		uint8_t operator[](size_t i) const { return indices[i]; }
		uint8_t size;
		uint8_t indices[1];
		//next will follow
	};
#pragma pack(pop)

	using Path_t = std::vector< std::pair<MC::StateNode*, MoveNode*> >;
	using CLK = std::chrono::high_resolution_clock;
	struct Player : IGamePlayer
	{
		const MCTSConfig	m_cfg;
		EvalFunction_t	m_eval_function;
		IMoveLimit		*m_mv_limit;
		ITrace			*m_trace;
		IGameRules		*m_game_rules;
		StateNode		*m_root = nullptr;
		StateNode		*m_super_root = nullptr;
		std::default_random_engine	m_generator;
		ObjectPoolMultisize<4 * sizeof(MoveNode), 4096> m_nodePool;					//1 chunk = 1 statenode + 4 moves
		ObjectPoolMultisize<2 * sizeof(ValidMoveList), 16384> m_validMoveListPool;	//1 chunk = 3 moves
		int				m_move_nbr = 1;
		int				m_game_nbr = 1;
		Histogram<long>	m_nodePool_usage;
		Histogram<long> m_num_runs_per_move;
		Histogram<string>	m_find_root_node_result;
		const bool		m_release_nodes_during_find;
		using StatesBimap = boost::bimap<boost::bimaps::set_of<string>, boost::bimaps::set_of<StateNode*>>;
		StatesBimap		m_states_in_game_tree;
		unsigned short	m_curr_visit_id = 0;

		Player(const MCTSConfig cfg, IMoveLimit *mv_limit, ITrace* trace);
		~Player();
		void	seed(unsigned long seed) { m_generator.seed(seed); }
		void	release() override { delete this; }
		void	startNewGame(GameState*) override { m_move_nbr = 1; }
		void	endGame(int score, GameResult result) override;
		void	setGameRules(IGameRules* gr)       override { m_game_rules = gr; }
		void	setEvalFunction(EvalFunction_t ef) override { m_eval_function = ef; }
		NamedMetrics_t	getGameStats() override;
		void	resetStats() override {}
		std::string getName() override { return "mcts"; }
		void _dumpMoveTree();
		void _dumpGameTree();
		MoveList* selectMove(GameState* gs) override;
		void runSingleSimulation();
		std::tuple<Move*, float> runNSimulations(GameState* bs, int totNumSimulations);
		string makeTreeFilename(const char* basename);
		StateNode* findRootNode(GameState* s);
		enum class VisitTreeOpResult { Cont=0, Abort, Skip  };
		static void visitTree(StateNode* node, unsigned short id, std::function<bool(StateNode*)> visit);
		static bool visitTreeDepthFirstInt(StateNode* node, unsigned short visit_id, std::function<Player::VisitTreeOpResult(StateNode*, int)> visit_op, int depth);
		static void visitTreeDepthFirst(StateNode* node, unsigned short visit_id, std::function<VisitTreeOpResult(StateNode*, int)> visit_op);
		bool checkTree(StateNode* root, unsigned short id, bool dump = true);
		StateNode* find_breadth_first(StateNode*root, const GameState*s);
		void freeSubTree(StateNode* root, unsigned short stay_alive_id, unsigned short visit_id);
		Path_t selection_playOut(StateNode* root, unsigned short visit_id);
		size_t selectOneOf(size_t first, size_t last);
		void backpropagation(Path_t& path, bool cycle);
		void freeTemporaryNodes(StateNode * root, unsigned short visit_id);
		void expansion(Path_t& path);
		MoveNode* selectMove(StateNode* node, const ValidMoveList& moves, double C);
		MoveNode* selectBestMove(StateNode* node);
		StateNode* makeTreeNode(GameState* pks);
		void makeNodePermanent(StateNode * sn);
		void freeTree(StateNode* node);
		void freeTree(StateNode* root, std::vector<StateNode*>& toBeFreed, unsigned short visit_id);
		void freeStateNode(StateNode* node);
		StateNode* _alloc(int number_of_moves);
		void _free(StateNode* node);
		void dumpTreeWithPath(const string& filename, StateNode* root, const Path_t& path);
		void dumpTree(const string & filename, StateNode* root, std::set<StateNode*> nodesToHighlight = {});
		void dumpNodeDescription(std::wofstream& out, StateNode* sn, bool highlight);
		bool dumpMoveDescription(std::wofstream& out, StateNode* sn, int dummyNodeId, const MoveNode& mv);
		void dumpTreeNode(std::wofstream& out, StateNode* sn, unsigned visit_id, std::set<StateNode*>& nodesToHighlight);
		std::tuple<StateNode*, Path_t> loadTreeFromFile(const char* filename);
		std::tuple<StateNode*, Path_t> loadTree(std::wifstream& input);
		void traceSelectMove(const wstring& state, const std::multimap<double, MoveNode*>& moves, const wstring& selected, StateNode* sn);
	};
	IGamePlayer* createMCTSPlayer(int player_number, const PlayerConfig_t& pc);
}
