#include "pch.h"
#include "GraWZombiakiZasady.h"

namespace GraWZombiaki
{
	struct GamePhaseCleanup : GamePhase
	{
		void nextPhase(GameState* gs) override
		{
			if (gs->current_player == Player::human) {
				gs->plansza.removeZapora();
				gs->plansza.removeNet();
			}
			else {
				gs->terror = 0;
			}
			gs->phase = cat_movement;
		}
	};
	struct GamePhaseCatMovement : GamePhase
	{
		void getValidMoves(const GameState* gs, std::function<Card_If * (Card)> getCardIf, MoveList_t& moves) override
		{
			if (gs->current_player == Player::zombie) {
				auto [p, found] = gs->plansza.findByType(KartaNaPlanszy::kot);
				if (found) {
					getCardIf(ZombieCard{ ObiektZombie::kot })->getValidMoves(gs, p, moves);
				}
			}
		}

		void nextPhase(GameState* gs) override
		{
			gs->phase = dog_movement;
		}
	};
	
	struct GamePhaseDogMovement : GamePhase
	{
		const ZombieCard pies = ZombieCard(ObiektZombie::pies);
		void getValidMoves(const GameState* gs, std::function<Card_If * (Card)> getCardIf, MoveList_t& moves) override
		{
			if (gs->current_player == Player::zombie) {
				auto [p, found] = gs->plansza.findByType(KartaNaPlanszy::pies);
				if (found) {
					getCardIf(ZombieCard{ ObiektZombie::pies })->getValidMoves(gs, p, moves);
				}
			}
		}
		void nextPhase(GameState* gs) override
		{
			gs->phase = movement;
		}
	};
	
	struct GamePhaseMovement : GamePhase
	{
		void nextPhase(GameState* gs) override
		{
			gs->plansza.moveCards();

			//draw new cards
			auto& deck = gs->getPlayerDeck(gs->current_player);
			const unsigned numTotal = gs->current_player == Player::zombie ? NumZombieCards : NumHumanCards;
			if (deck.numVisible < 4) {
				deck.numVisible = min(4, numTotal - deck.numVisible - deck.firstVisible);
			}
			gs->phase = boss_order;
		}
	};
	
	struct GamePhaseBossOrder : GamePhase
	{
		void getValidMoves(const GameState* gs, std::function<Card_If * (Card)> getCardIf, MoveList_t& moves) override
		{
			if (gs->current_player == Player::zombie) {
				KartaNaPlanszy m;
				m.bf_jest_bossem = 1;
				auto [p, found] = gs->plansza.findByMask(m.value);
				if (found) {
					getCardIf(ZombieCard{ WzmocnienieZombie::boss })->getValidMoves(gs, p, moves);
				}
			}
		}
		void nextPhase(GameState* gs) override
		{
			gs->phase = discard_card;
		}
	};

	struct GamePhaseDiscardCard : GamePhase
	{
		void getValidMoves(const GameState* gs, std::function<Card_If * (Card)> getCardIf, MoveList_t& moves) override
		{
			const auto& deck = gs->getPlayerDeck(gs->current_player);
			if (deck.numVisible == 4)
			{
				for (int idx = 0; idx < deck.numVisible; ++idx) {
					moves.push_back( makeMove<Mv_DiscardCard>(idx + deck.firstVisible) );
				}
			}
		}
		void nextPhase(GameState* gs) override
		{
			gs->phase = Phase::play_1st_card;
			
			/*const auto& deck = gs->getPlayerDeck(gs->current_player);
			if (deck.numVisible > 0) {
				gs->phase = play_1st_card;
			}
			else {
				gs->phase = Phase::cleanup;
				gs->current_player = gs->current_player + 1;
			}*/
		}
		virtual ~GamePhaseDiscardCard() {}
	};
	struct GamePhasePlayCard : GamePhase
	{
		void getValidMoves(const GameState* gs, std::function<Card_If * (Card)> getCardIf, MoveList_t& moves) override
		{
			const auto& deck = gs->getPlayerDeck(gs->current_player);
			if (deck.numVisible > 0)
			{
				for (int idx = 0; idx < deck.numVisible; ++idx) {
					const int cidx = deck.firstVisible + idx;
					const auto card = deck.cards[cidx];
					getCardIf(card)->getValidUsage(gs, cidx, moves);
				}
				moves.push_back( makeMove<Mv_Noop>() );
			}
		}
		void nextPhase(GameState* gs) override
		{
			const auto& deck = gs->getPlayerDeck(gs->current_player);
			if (deck.numVisible > 0 && gs->phase < play_3rd_card && !gs->terror) {
				gs->phase += 1;
			}
			else {
				gs->phase = Phase::cleanup;
				gs->current_player = gs->current_player + 1;
			}
		}
		virtual ~GamePhasePlayCard() {}
	};
	GamePhaseCleanup	 phCleanup;
	GamePhaseCatMovement phCatMovement;
	GamePhaseDogMovement phDogMovement;
	GamePhaseMovement	 phMovement;
	GamePhaseDiscardCard phDiscard;
	GamePhasePlayCard	phPlay;

	GamePhase* GraWZombiakiZasady::gamePhases[] =
	{
		&phCleanup,
		&phCatMovement,
		&phDogMovement,
		&phMovement,
		&phDiscard,
		&phPlay,
		&phPlay,
		&phPlay
	};
}