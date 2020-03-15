#include "pch.h"
#include "GraWZombiakiZasady.h"
#include <cassert>
#include "Karty.h"
#include "Plansza.h"

namespace GraWZombiaki
{
	MoveList* GraWZombiakiZasady::get_moves_in_phase_clenup(const GameState*)
	{
		return &m_noop;
	}

	MoveList* GraWZombiakiZasady::get_moves_in_phase_cat_movement(const GameState* gs)
	{
		vector<uint16_t> moves;
		const auto card = ZombieCard(ObiektZombie::kot);
		getCardIf(card)->getValidApplications(gs, card, moves);
		return moveListFromVector(moves);
	}

	MoveList* GraWZombiakiZasady::get_moves_in_phase_dog_movement(const GameState* gs)
	{
		vector<uint16_t> moves;
		const auto card = ZombieCard(ObiektZombie::pies);
		getCardIf(card)->getValidApplications(gs, card, moves);
		return moveListFromVector(moves);
	}

	MoveList* GraWZombiakiZasady::get_moves_in_phase_general_movement(const GameState*)
	{
		return &m_noop;
	}

	MoveList* GraWZombiakiZasady::getMovesInPhaseTakeCards(const GameState* gs)
	{
		const auto& deck = gs->getPlayerDeck(gs->current_player);
		const unsigned numTotal = gs->current_player == Player::zombie ? NumZombieCards : NumHumanCards;
		
		if (deck.numVisible < 4)
		{
			const unsigned maxToDraw = __min(4u - deck.numVisible, numTotal-deck.numVisible-deck.firstVisible);
			auto* ml = allocMoveList(maxToDraw);
			ml->size = maxToDraw;
			Move* mv = ml->move;
			for (auto cnt = 1u; cnt <= maxToDraw; ++cnt,++mv) {
				new (mv) Mv_DrawCards(cnt);
			}
			return ml;
		}
		return &m_noop;
	}

	MoveList* GraWZombiakiZasady::getMovesInPhaseDiscardCards(const GameState* gs)
	{
		const auto & deck = gs->getPlayerDeck(gs->current_player);
		if (deck.numVisible > 0)
		{
			auto* ml = allocMoveList(deck.numVisible);
			ml->size = deck.numVisible;
			Move* mv = ml->move;
			for (int idx = 0; idx < deck.numVisible; ++idx, ++mv) {
				new (mv) Mv_DiscardCard(idx + deck.firstVisible);
			}
			return ml;
		}
		return &m_noop;
	}

	MoveList* GraWZombiakiZasady::moveListFromVector(vector<uint16_t> moves)
	{
		if (!moves.empty())
		{
			const auto num_moves = (unsigned)moves.size();
			auto* ml = allocMoveList(num_moves);
			ml->size = num_moves;
			Move* mv = ml->move;
			for (auto mve : moves)
			{
				mv->value = mve;
				++mv;
			}
			return ml;
		}
		return &m_noop;
	}

	MoveList* GraWZombiakiZasady::getMovesInPhasePlayCard(const GameState* gs)
	{
		vector<uint16_t> moves;
		const auto& deck = gs->getPlayerDeck(gs->current_player);
		if (deck.numVisible > 0)
		{
			for (int idx = 0; idx < deck.numVisible; ++idx) {
				const auto card = deck.cards[idx];
				auto* cif = getCardIf(card);
				cif->getValidApplications(gs, card, moves);
			}
		}
		return moveListFromVector(moves);
	}

	GameState* GraWZombiakiZasady::drawCards(const GameState* gs, uint8_t numCards, int player)
	{
		auto ngs = allocGameState();
		*ngs = *gs;
		ngs->getPlayerDeck(player).numVisible += numCards;
		return ngs;
	}

	GameState* GraWZombiakiZasady::discardCard(const GameState* gs, uint8_t discardedCardIdx, int player)
	{
		auto ngs = allocGameState();
		*ngs = *gs;
		ngs->getPlayerDeck(player).discard(discardedCardIdx);
		return ngs;
	}

	GameState* GraWZombiakiZasady::playCard(const GameState* gs, Mv_PlayCard* mv, int player)
	{
		auto ngs = allocGameState();
		*ngs = *gs;
		const auto [przecznica, tor, cardIdx] = mv->get();
		auto& deck = ngs->getPlayerDeck(player);
		const Card card = deck.cards[cardIdx];
		Card_If* cif = getCardIf(card);
		deck.discard(cardIdx);
		getCardIf(card)->play(ngs, przecznica, tor, card);
		return ngs;
	}

	GameState* GraWZombiakiZasady::useCard(const GameState* gs, unsigned cardIdx, int player)
	{
		auto ngs = allocGameState();
		*ngs = *gs;
		auto& deck = ngs->getPlayerDeck(player);
		const Card card = deck.cards[cardIdx];
		deck.discard(cardIdx);
		getCardIf(card)->use(ngs, card);
		return ngs;
	}
}
