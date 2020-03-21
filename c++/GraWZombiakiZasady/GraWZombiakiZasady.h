#pragma once
#include <cassert>
#include "GameRules.h"
#include "object_pool.h"
#include "Karty.h"
#include "Plansza.h"
#include "Moves.h"
#include "Common.h"
#include "object_pool_multisize.h"

using namespace GraWZombiaki;
struct GameState
{
	Plansza		plansza;
	Deck		zombieDeck;
	Deck		humanDeck;
	uint8_t		phase : 6;
	uint8_t		terror : 1;
	uint8_t		current_player : 1;

	Deck& getPlayerDeck(int player) { return player == Player::zombie ? zombieDeck : humanDeck; }
	const Deck& getPlayerDeck(int player) const { return player == Player::zombie ? zombieDeck : humanDeck; }
};

struct MoveList
{
	MoveList() : size(0) {}
	MoveList(std::initializer_list<Move> moves)
	{
		size = static_cast<uint16_t>(moves.size());
		int idx = 0;
		for (auto& mv : moves) { move[idx++] = mv; }
	}
	uint16_t size;
	Move move[1];
};

namespace GraWZombiaki
{
	enum Phase : uint8_t {
		cleanup,
		cat_movement,
		dog_movement,
		movement,
		boss_order,
		discard_card,
		play_1st_card,
		play_2nd_card,
		play_3rd_card
	};

	struct GamePhase
	{
		virtual void getValidMoves(const GameState*gs, std::function<Card_If*(Card)> getCardIf, MoveList_t& moves) {};
		virtual void nextPhase(GameState* gs) = 0;
	};
	
	struct GraWZombiakiZasady : IGameRules
	{
		GraWZombiakiZasady()
		{
			m_noop.size = 1;
			m_noop.move[0].op = Move::noop;
			m_refCnt = 1;
		}
		~GraWZombiakiZasady()
		{
		}

		void SetRandomGenerator(IRandomGenerator*) override;
		GameState* CreateRandomInitialState(IRandomGenerator*) override;
		GameState* CreateInitialStateFromHash(const uint32_t*) override;
		GameState* CreateStateFromString(const wstring&) override;
		GameState* CreateStateFromString(const string&) override;
		GameState* CreatePlayerKnownState(const GameState*, int playerNum) override;
		EvalFunction_t CreateEvalFunction(const string& name) override;
		void		UpdatePlayerKnownState(GameState* playerKnownState, const GameState* completeGameState, const std::vector<MoveList*>& playerMoves) override;
		GameState* CopyGameState(const GameState*) override;
		void copyGameState(const GameState * src, GameState * dst);
		bool AreEqual(const GameState*, const GameState*) override;
		void ReleaseGameState(GameState*) override;
		bool IsTerminal(const GameState*) override;
		void Score(const GameState*, int score[]) override;
		int GetCurrentPlayer(const GameState*) override;
		MoveList* moveListFromVector(vector<uint16_t> moves);
		MoveList* GetPlayerLegalMoves(const GameState*, int playerNum) override;
		void ReleaseMoveList(MoveList*) override;
		int GetNumMoves(const MoveList*) override;
		std::tuple<Move*, float> GetMoveFromList(MoveList*, int idx) override;
		MoveList* SelectMoveFromList(const MoveList*, int idx) override;
		GameState* ApplyMove(const GameState*, Move*, int player) override;
		GameState* Next(const GameState*, const std::vector<MoveList*>& moves) override;
		const uint32_t* GetStateHash(const GameState*) override;
		size_t GetStateHashSize() override;
		string ToString(const GameState*) override;
		wstring ToWString(const GameState*) override;
		string ToString(const Move*) override;
		wstring ToWString(const Move*) override;
		void AddRef() override;
		void Release() override;

		GameState* allocGameState()	{
			return m_GameStatePool.alloc();
		}
		void freeGameState(GameState* s) {
			m_GameStatePool.free(s);
		}

		MoveList* allocMoveList(unsigned numMoves);
		MoveList* allocMoveList(std::initializer_list<Move> moves);

		const Card* getZombieCards();
		const Card* getHumanCards();

		GameState* discardCard(const GameState* gs, uint8_t discardedCardIdx, int player);
		Card_If*   getCardIf(Card card);
		Card_If*   getCardIf(KartaNaPlanszy knp);
		GameState* playCard(const GameState* gs, Mv_PlayCard* mv, int player);
		GameState* useCard(const GameState* gs, unsigned cardIdx, int player);
		GameState* moveCard(const GameState* gs, const Position& from, const Position& to, int player);
		
		static GamePhase* gamePhases[];

		ObjectPoolBlocked<GameState, 512>		m_GameStatePool;
		ObjectPoolMultisize<4 * sizeof(Move), 4096> m_moveListPool;
		MoveList m_noop;
		int		m_refCnt;
	};
}
