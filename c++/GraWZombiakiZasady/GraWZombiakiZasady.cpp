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
			&get_moves_in_phase_clenup,
			&get_moves_in_phase_cat_movement,
			&get_moves_in_phase_dog_movement,
			&get_moves_in_phase_general_movement,
			&getMovesInPhaseTakeCards,
			&getMovesInPhaseDiscardCards,
			&getMovesInPhasePlayCard,
			& getMovesInPhasePlayCard,
			& getMovesInPhasePlayCard
	};

	void GraWZombiakiZasady::SetRandomGenerator(IRandomGenerator*)
	{
	}

	GameState* GraWZombiakiZasady::CreateRandomInitialState(IRandomGenerator* rng)
	{
		auto gs = allocGameState();
		gs->current_player = Player::zombie;
		gs->phase = Phase::take_cards;
		gs->zombieDeck.value = 0;
		gs->humanDeck.value = 0;
		memcpy(gs->zombieDeck.cards, getZombieCards(), sizeof(Card) * NumZombieCards);
		//shuffle, but leave last cardIdx (świt) last
		shuffle(gs->zombieDeck.cards, rng->generateUniform(0, NumZombieCards-2, NumZombieCards*2));
		memcpy(gs->humanDeck.cards, getHumanCards(), sizeof(Card) * NumHumanCards);
		shuffle(gs->humanDeck.cards, rng->generateUniform(0, NumZombieCards-1 , NumZombieCards*2));
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
		return (this->*getMovesInState[gs->phase])(gs);
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
		switch (mv->op) {
		default:
		case Move::noop:			ngs = CopyGameState(gs); break;
		case Move::draw_cards:		ngs = drawCards  (gs, static_cast<Mv_DrawCards*>(mv)->getCardsCnt(), player);	break;
		case Move::discard_card:	ngs = discardCard(gs, static_cast<Mv_DiscardCard*>(mv)->getCardIdx(), player);	break;
		case Move::play_card:		ngs = playCard   (gs, static_cast<Mv_PlayCard*>(mv), player);					break;
		case Move::use_card:		ngs = useCard	 (gs, static_cast<Mv_UseCard*>(mv)->getCardIdx(), player);		break;
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

	MoveList* GraWZombiakiZasady::allocMoveList(unsigned numMoves)
	{
		const size_t num_chunks = (numMoves * sizeof(Move)) / m_moveListPool.ChunkSize + 1;
		//const size_t num_chunks = moves.size() + 1;
		return m_moveListPool.alloc<MoveList>(num_chunks);
	}
	
	void GraWZombiakiZasady::freeMoveList(MoveList* ml)
	{
		if (ml != &m_noop && ml) {
			const size_t num_chunks = (ml->size * sizeof(Move)) / m_moveListPool.ChunkSize + 1;
			//const size_t num_chunks = ml->size + 1;
			m_moveListPool.free(ml, num_chunks);
		}
	}

	void Deck::discard(unsigned idx)
	{
		assert(idx >= firstVisible && idx  < firstVisible + numVisible);
		if (idx > firstVisible) {
			Card tmp = cards[firstVisible];
			cards[firstVisible] = cards[idx];
			cards[idx] = tmp;
		}
		firstVisible += 1;
		numVisible -= 1;
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


