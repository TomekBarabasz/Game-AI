#pragma once

#include <map>
#include <vector>
#include "GameRules.h"
#include <algorithm>
#include <boost/test/utils/string_cast.hpp>
#include "MCTSPlayer.h"

using std::map;
using std::vector;

struct GameState;
struct Move
{
	Move(const wstring & name, GameState* target,float prob=1.0f) : m_name(name), m_target(target), m_prob(prob) {}
	wstring m_name;
	float m_prob;
	GameState* m_target;
};

struct MoveList
{
	vector<Move> moves;
	int selected  = -1;
};

struct GameState
{
	wstring name;
	int player;
	bool isTerminal;
	int score[4];
	map<int, MoveList> playerMoves;
};

struct TestGameRules: IGameRules
{
	/*TestGameRules(){}
	TestGameRules(const TestGameRules& other)
	{
		m_tree = other.m_tree;
	}
	TestGameRules& operator=(TestGameRules&& other)
	{
		m_tree = std::move(other.m_tree);
		return *this;
	}*/
	void SetRandomGenerator(IRandomGenerator*) override {}
	GameState* CreateRandomInitialState(IRandomGenerator*) override
	{
		return &m_tree[0];
	}
	GameState* CreateInitialStateFromHash(const uint32_t*) override
	{
		throw "not implemented";
	}
	GameState* CreatePlayerKnownState(const GameState*, int playerNum) override
	{
		throw "not implemented";
	}
	EvalFunction_t CreateEvalFunction(const string& name)
	{
		throw "not implemented";
	}
	void UpdatePlayerKnownState(GameState* playerKnownState, const GameState* completeGameState, const std::vector<MoveList*>& playerMoves) override
	{
		throw "not implemented";
	}

	GameState*	CreateStateFromString(const wstring& sstr) override
	{
		auto it = std::find_if(m_tree.begin(), m_tree.end(), [&](const GameState& gs) {
			return gs.name == sstr;
			});
		return &*it;
	}
	GameState* CreateStateFromString(const string& sstr) override
	{
		throw "not implemented";
	}
	GameState* CopyGameState(const GameState*) override
	{
		throw "not implemented";
	}
	bool AreEqual(const GameState* a, const GameState* b) override
	{
		return a->name == b->name;
	}
	void ReleaseGameState(GameState*) override
	{
	}
	bool IsTerminal(const GameState* s) override
	{
		return s->isTerminal;
	}
	void Score(const GameState* s, int score[]) override
	{
		for (int i = 0; i < 4; ++i) score[i] = s->score[i];
	}
	int GetCurrentPlayer(const GameState *s) override
	{
		return s->player;
	}
	MoveList* GetPlayerLegalMoves(const GameState* s, int player) override
	{
		static MoveList empty;
		if (s->isTerminal) return &empty;
		auto it = s->playerMoves.find(player);
		/*auto & m = s->playerMoves;
		for (auto & [player, ml] : m){}*/
		return it != s->playerMoves.end() ? const_cast<MoveList*>(&(it->second)) : &empty;
	}
	void ReleaseMoveList(MoveList*) override
	{
	}
	int	 GetNumMoves(const MoveList* ml) override
	{
		return (int)ml->moves.size();
	}
	std::tuple<Move*, float> GetMoveFromList(MoveList* ml, int idx) override
	{
		auto & move = ml->moves[idx];
		return { &move, move.m_prob };
	}
	MoveList* SelectMoveFromList(const MoveList* ml, int idx) override
	{
		MoveList *ret = const_cast<MoveList*>(ml);
		ret->selected = idx;
		return ret;
	}
	GameState* ApplyMove(const GameState* s, Move* mv, int player) override
	{
		return mv->m_target;
	}
	GameState* Next(const GameState* s, const std::vector<MoveList*>& moves) override
	{
		GameState *ns = nullptr;
		for (auto *ml : moves)
		{
			ns = ml->moves[ml->selected].m_target;
		}	
		return ns;
	}
	const uint32_t* GetStateHash(const GameState* s) override
	{
		return reinterpret_cast<const uint32_t*>(s);
	}
	size_t GetStateHashSize() override
	{
		return 2;
	}
	string ToString(const GameState* s) override
	{
		std::ostringstream oss;
		oss << s -> name.c_str();
		return oss.str();
	}
	wstring ToWString(const GameState* s) override
	{
		return s->name;
	}
	string ToString(const Move* mv) override
	{
		std::ostringstream oss;
		oss << mv->m_name.c_str();
		return oss.str();
	}
	wstring ToWString(const Move* mv) override
	{
		return mv->m_name;
	}
	void AddRef() override
	{
		
	}
	void Release() override
	{
		
	}
	vector<GameState> m_tree;
};
TestGameRules Load(const char *filename);

struct state_builder
{
	GameState *gs;
	state_builder(GameState* gs_, const wchar_t* name, int current_player) :gs(gs_)
	{
		//gs = new GameState { name, current_player, false };
		*gs = { name, current_player, false };
	}
	state_builder& move(const wchar_t* name, GameState* target, float prob=1.0f, int player=-1)
	{
		if (-1 == player) player = gs->player;
		gs->playerMoves[player].moves.push_back({name, target, prob});
		return *this;
	}
	state_builder& score(std::initializer_list<int> score)
	{
		gs->isTerminal = true;
		int idx = 0;
		for (int sc : score) {
			gs->score[idx++] = sc;
		}
		return *this;
	}
	state_builder& noop(int player)
	{
		gs->playerMoves[player].moves.push_back({ L"noop",gs });
		return *this;
	}
	GameState* make()
	{
		return gs;
	}
};
inline state_builder new_state(GameState *gs, const wchar_t* name, int current_player)
{
	return state_builder(gs, name, current_player);
}

struct TestSimLimit : MC::IMoveLimit
{
	TestSimLimit(int limit) : sim_limit(limit) {}
	int sim_done;
	const int sim_limit;
	void start() { sim_done = 0; }
	bool can_continue() { return ++sim_done < sim_limit; }
	void release() { delete this; }
};
struct CreateMCTSPlayer
{
	MC::MCTSConfig		cfg{ 0,2,1,false, 10,2.0,1234,-50,"c:\\MyData\\Projects\\gra_w_pana\\logs","","" };
	MC::Player		player;
	CreateMCTSPlayer() : player(cfg, new TestSimLimit(30), createInstance(""))
	{
	}
	~CreateMCTSPlayer()
	{
	}
};
