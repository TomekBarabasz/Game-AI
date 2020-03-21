#include "pch.h"
#include "GraWZombiakiZasady.h"
#include <cassert>
#include "Karty.h"
#include "Plansza.h"

namespace GraWZombiaki
{
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
		const auto [pozycja, cardIdx] = mv->get();
		auto& deck = ngs->getPlayerDeck(player);
		const Card card = deck.cards[cardIdx];
		getCardIf(card)->place(ngs, pozycja, cardIdx);
		deck.discard(cardIdx);
		return ngs;
	}

	GameState* GraWZombiakiZasady::useCard(const GameState* gs, unsigned cardIdx, int player)
	{
		auto ngs = allocGameState();
		*ngs = *gs;
		auto& deck = ngs->getPlayerDeck(player);
		const Card card = deck.cards[cardIdx];
		getCardIf(card)->use(ngs, cardIdx);
		deck.discard(cardIdx);
		return ngs;
	}
	GameState* GraWZombiakiZasady::moveCard(const GameState* gs, const Position& from, const Position& to, int player)
	{
		auto ngs = allocGameState();
		*ngs = *gs;
		auto& knp = gs->plansza[from];

		getCardIf(knp)->move(ngs, from, to);
		return ngs;
	}
}
