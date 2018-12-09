// GraWZombiakiZasady.cpp : Defines the exported functions for the DLL application.
//

#include "pch.h"
#include "memory_mgmt.h"
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS  
#include "GraWZombiakiZasady.h"

namespace GraWZombiaki
{
	void GraWZombiakiZasady::SetRandomGenerator(IRandomGenerator*)
	{
	}

	GameState* GraWZombiakiZasady::CreateRandomInitialState(IRandomGenerator*)
	{
		throw "not implemented";
	}

	GameState* GraWZombiakiZasady::CreateInitialStateFromHash(const uint32_t*)
	{
		throw "not implemented";

	}

	GameState* GraWZombiakiZasady::CreateStateFromString(const wstring&)
	{
		throw "not implemented";

	}

	GameState* GraWZombiakiZasady::CopyGameState(const GameState*)
	{
		throw "not implemented";

	}

	bool GraWZombiakiZasady::AreEqual(const GameState*, const GameState*)
	{
		throw "not implemented";

	}

	void GraWZombiakiZasady::ReleaseGameState(GameState*)
	{
		throw "not implemented";

	}

	bool GraWZombiakiZasady::IsTerminal(const GameState*)
	{
		throw "not implemented";

	}

	void GraWZombiakiZasady::Score(const GameState*, int score[])
	{
		throw "not implemented";
	}

	int GraWZombiakiZasady::GetCurrentPlayer(const GameState*)
	{
		throw "not implemented";
	}

	MoveList* GraWZombiakiZasady::GetPlayerLegalMoves(const GameState*, int playerNum)
	{
		throw "not implemented";
	}

	void GraWZombiakiZasady::ReleaseMoveList(MoveList*)
	{
		throw "not implemented";
	}

	int GraWZombiakiZasady::GetNumMoves(const MoveList*)
	{
		throw "not implemented";
	}

	Move* GraWZombiakiZasady::GetMoveFromList(MoveList*, int idx)
	{
		throw "not implemented";
	}

	MoveList* GraWZombiakiZasady::SelectMoveFromList(const MoveList*, int idx)
	{
		throw "not implemented";
	}

	GameState* GraWZombiakiZasady::ApplyMove(const GameState*, Move*, int player)
	{
		throw "not implemented";
	}

	GameState* GraWZombiakiZasady::Next(const GameState*, const std::vector<MoveList*>& moves)
	{
		throw "not implemented";
	}

	const uint32_t* GraWZombiakiZasady::GetStateHash(const GameState*)
	{
		throw "not implemented";
	}

	size_t GraWZombiakiZasady::GetStateHashSize()
	{
		throw "not implemented";
	}

	string GraWZombiakiZasady::ToString(const GameState*)
	{
		throw "not implemented";
	}

	wstring GraWZombiakiZasady::ToWString(const GameState*)
	{
		throw "not implemented";
	}

	string GraWZombiakiZasady::ToString(const Move*)
	{
		throw "not implemented";
	}

	wstring GraWZombiakiZasady::ToWString(const Move*)
	{
		throw "not implemented";
	}

	void GraWZombiakiZasady::AddRef()
	{
		throw "not implemented";
	}

	void GraWZombiakiZasady::Release()
	{
		throw "not implemented";
	}
}
#ifndef UNIT_TEST
namespace MemoryMgmt
{
	using namespace GraWZombiaki;
	struct MemoryPools
	{
	};

	MemoryPools* makeMemoryPoolsInst() { return nullptr; }
	void		 freeMemoryPoolsInst(MemoryPools* i) { }
}
IGameRules* createGraWZombiakiGameRules()
{
	return new GraWZombiaki::GraWZombiakiZasady();
}
BOOST_DLL_ALIAS(
	createGraWZombiakiGameRules,		// <-- this function is exported with...
	createGameRules			// <-- ...this alias name
)
#endif


