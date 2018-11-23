#pragma once

#include <map>
#include <vector>
#include "GameRules.h"

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
	void SetRandomGenerator(IRandomGenerator*) override {}
	GameState* CreateRandomInitialState(IRandomGenerator*) override
	{
		return &m_tree[0];
	}
	GameState* CreateInitialStateFromHash(const uint32_t*) override
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
		return const_cast<MoveList*>( &(s->playerMoves.at(player)) );
	}
	void ReleaseMoveList(MoveList*) override
	{
	}
	int	 GetNumMoves(const MoveList* ml) override
	{
		return (int)ml->moves.size();
	}
	Move* GetMoveFromList(MoveList* ml, int idx) override
	{
		return &(ml->moves[idx]);
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
