// GraWZombiakiZasady.cpp : Defines the exported functions for the DLL application.
//

#include "pch.h"
#include "memory_mgmt.h"
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS  
#include "GraWZombiakiZasady.h"
#include "random_generator.h"

using std::vector;

namespace GraWZombiaki
{
	GraWZombiakiZasady::GetMoveListMethod_t GraWZombiakiZasady::getMovesInState[]{
			&get_moves_in_pase_clenup,
			&get_moves_in_pase_cat_movement,
			&get_moves_in_pase_dog_movement,
			&get_moves_in_pase_general_movement,
			&get_moves_in_pase_take_cards,
			&get_moves_in_pase_discard_cards,
			&get_moves_in_pase_play_1st_card,
			&get_moves_in_pase_play_2nd_card,
			&get_moves_in_pase_play_3rd_card
	};

	void GraWZombiakiZasady::SetRandomGenerator(IRandomGenerator*)
	{
	}

	GameState* GraWZombiakiZasady::CreateRandomInitialState(IRandomGenerator* rng)
	{
		auto gs = allocGameState();
		gs->current_player = static_cast<unsigned char>(Gracz::zombie);
		gs->phase = Phase::take_cards;
		gs->cmentarz_cnt = 0;
		gs->barykada_cnt = 0;
		gs->ludzie_talia_idx = 0;
		gs->ludzie_talia_idx = 0;
		memcpy(gs->zombie, getZombieCards(), sizeof(Karta) * NumZombieCards);
		//shuffle, but leave last card (świt) last
		//shuffle(gs->zombie, rng->generateUniform(0, NumZombieCards-2, NumZombieCards*2));
		return gs;
	}

	GameState* GraWZombiakiZasady::CreateInitialStateFromHash(const uint32_t*)
	{
		throw "not implemented";
	}

	GameState* GraWZombiakiZasady::CreateStateFromString(const wstring&)
	{
		throw "not implemented";
	}

	GameState* GraWZombiakiZasady::CreateStateFromString(const string&)
	{
		throw "not implemented";
	}

	GameState* GraWZombiakiZasady::CreatePlayerKnownState(const GameState*, int playerNum)
	{
		throw "not implemented";
	}

	EvalFunction_t GraWZombiakiZasady::CreateEvalFunction(const string& name)
	{
		throw "not implemented";
	}

	void GraWZombiakiZasady::UpdatePlayerKnownState(GameState* playerKnownState, const GameState* completeGameState, const std::vector<MoveList*>& playerMoves)
	{
		throw "not implemented";
	}

	GameState* GraWZombiakiZasady::CopyGameState(const GameState* gs)
	{
		auto gsn = allocGameState();
		copyGameState(gs, gsn);
		return gsn;
	}

	void GraWZombiakiZasady::copyGameState(const GameState* src, GameState *dst)
	{
		memcpy(dst, src, sizeof(GameState));
	}
	bool GraWZombiakiZasady::AreEqual(const GameState* a, const GameState* b)
	{
		return 0 == memcmp(a, b, sizeof(GameState));
	}

	void GraWZombiakiZasady::ReleaseGameState(GameState* gs)
	{
		freeGameState(gs);
	}

	bool GraWZombiakiZasady::IsTerminal(const GameState*)
	{
		throw "not implemented";

	}

	void GraWZombiakiZasady::Score(const GameState*, int score[])
	{
		throw "not implemented";
	}

	int GraWZombiakiZasady::GetCurrentPlayer(const GameState* gs)
	{
		return gs->current_player;
	}

	MoveList* GraWZombiakiZasady::GetPlayerLegalMoves(const GameState* gs, int playerNum)
	{
		if (playerNum != gs->current_player) {
			return &m_noop;
		}
		return (this->*getMovesInState[static_cast<int>(gs->phase)])(gs);
	}

	void GraWZombiakiZasady::ReleaseMoveList(MoveList*)
	{
		throw "not implemented";
	}

	int GraWZombiakiZasady::GetNumMoves(const MoveList*)
	{
		throw "not implemented";
	}

	std::tuple<Move*, float> GraWZombiakiZasady::GetMoveFromList(MoveList*, int idx)
	{
		throw "not implemented";
	}

	MoveList* GraWZombiakiZasady::SelectMoveFromList(const MoveList*, int idx)
	{
		throw "not implemented";
	}

	GameState* GraWZombiakiZasady::ApplyMove(const GameState*gs, Move*mv, int player)
	{
		GameState* ngs;
		switch (mv->operation) {
		default:
		case Move::noop:			ngs = CopyGameState(gs); break;
		case Move::dobierz_karty:	ngs = dobierzKarty (gs, static_cast<Move_take_cards*>(mv)->num_cards, player);	break;
		case Move::odrzuc_karte:	ngs = odrzucKarte  (gs, static_cast<Move_discard_card*>(mv)->card, player);	break;
		case Move::zagraj_karte:	ngs = zagrajKarte  (gs, static_cast<Move_play_card*>(mv), player);			break;
		case Move::aktywuj_karte:	ngs = aktywujKarte (gs, static_cast<Move_activate_card*>(mv), player);		break;
		}
		return ngs;
	}

	GameState* GraWZombiakiZasady::Next(const GameState* s, const std::vector<MoveList*>& moves)
	{
		const int current_player = s->current_player;
		auto *cs = const_cast<GameState*>(s);
		for (int player = 0; player < 2; ++player)
		{
			auto ml = moves[player];
			auto * ns = ApplyMove(cs, &ml->move[0], player);
			freeGameState(cs);
			ReleaseMoveList(ml);
			cs = ns;
		}
		//cs->current_player = calcNextPlayer(cs, current_player);
		return cs;
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


