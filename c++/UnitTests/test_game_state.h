#pragma once

#include <map>
#include <vector>
#include "GameState.h"

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

using std::map;
using std::vector;

struct Node;
struct Move : IMove
{
	Move(const wstring & name, Node* target) : m_name(name), m_target(target) {}
	wstring toString() const override { return m_name; }
	void release() override {}
	wstring m_name;
	Node* m_target;
};
struct Node
{
	wstring name;
	int player;
	bool isTerminal;
	int score[4];
	map<int, vector<Move>> playerMoves;
};

struct TestGameState : IGameState
{
	TestGameState() : m_RefCnt(1)
	{
		TRACE(L"++ state %p\n", this);
	}
	TestGameState(int np) : TestGameState()
	{
		m_NumPlayers=np;
	}
	TestGameState(const TestGameState& other) : TestGameState(other.m_NumPlayers)
	{
		m_tree = other.m_tree;
	}
	TestGameState(TestGameState&& other) : TestGameState(other.m_NumPlayers)
	{
		m_tree = std::move(other.m_tree);
		m_curState = other.m_curState;
	}
	TestGameState& operator=(TestGameState&& other)
	{
		m_NumPlayers = other.m_NumPlayers;
		m_tree = std::move(other.m_tree);
		m_curState = other.m_curState;
		m_RefCnt = 1;
		return *this;
	}
	void		Initialize(unsigned seed) override {}
	wstring		ToString() const override
	{
		return m_curState->name;
	}
	void		AddRef() const override
	{
		++m_RefCnt;
		TRACE(L"state %p refcnt %d\n",this,m_RefCnt);
	}
	void		Release() const override
	{
		if (--m_RefCnt == 0) {
			TRACE(L"-- state %p\n", this);
			delete this;
		}else {
			TRACE(L"state %p refcnt %d\n", this, m_RefCnt);
		}
	}
	bool		IsTerminal() const override { return m_curState->isTerminal; }
	void		Score(int score[]) const override
	{
		for (int i = 0; i < m_NumPlayers; ++i) { score[i] = m_curState->score[i]; }
	}
	IGameState* Next(const MoveList & moves) const override
	{
		throw "not implemeneted";
	}
	IGameState* Apply(const IMove* imv, int playerNum) const override
	{
		auto & mv = *static_cast<const Move*>(imv);
		const auto & Moves = m_curState->playerMoves[playerNum];
		auto it = find_if(Moves.begin(), Moves.end(), [&mv](const Move &m) { return m.m_name == mv.m_name; });
		_ASSERT(it != Moves.end());
		TestGameState *ns = new TestGameState(*this);
		ns->m_curState = mv.m_target;
		return ns;
	}
	MoveList	GetPlayerLegalMoves(int player) const override
	{
		MoveList ml;
		for (auto & mv : m_curState->playerMoves[player]) {
			ml.push_back(&mv);
		}
		return ml;
	}
	GameStateHash_t	Hash() const override
	{
		GameStateHash_t h(reinterpret_cast<unsigned long long>(m_curState));
		return h;
	}
	string		HashS() const override { return Hash().to_string(); }
	int			NumPlayers() const override { return m_NumPlayers; }
	int			CurrentPlayer() const override { return m_curState->player; }

	int			m_NumPlayers;
	mutable int m_RefCnt;

	vector<Node> m_tree;
	Node	*m_curState;
};
TestGameState Load(const char *filename);
