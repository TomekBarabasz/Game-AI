#pragma once

#include <map>
#include <vector>
#include "GameRules.h"
#include <algorithm>

using std::map;
using std::vector;

struct GameState;
struct Move
{
	Move(const wstring & name, GameState* target) : m_name(name), m_target(target) {}
	wstring m_name;
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
	void UpdatePlayerKnownState(GameState* playerKnownState, const GameState* completeGameState, const std::vector<MoveList*>& playerMoves, int playerNum) override
	{
		throw "not implemented";
	}

	GameState*	CreateStateFromString(const wstring& sstr) override
	{
		auto it = std::find_if(m_tree.begin(), m_tree.end(), [&](const GameState& gs){
			return gs.name == sstr;
		});
		return &*it;
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
		return { &(ml->moves[idx]), 1.0 };
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
		return string(s->name.begin(), s->name.end());
	}
	wstring ToWString(const GameState* s) override
	{
		return s->name;
	}
	string ToString(const Move* mv) override
	{
		return string(mv->m_name.begin(), mv->m_name.end());
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
	state_builder& move(const wchar_t* name, GameState* target, int player=-1)
	{
		if (-1 == player) player = gs->player;
		gs->playerMoves[player].moves.push_back({name, target});
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
