#pragma once
#include "GameRules.h"
#include "object_pool.h"
#include "Karty.h"
#include "Plansza.h"
#include "object_pool_multisize.h"

namespace GraWZombiaki
{
	static const int NumZombieCards = 40;
	static const int NumHumanCards = 40;
	static const int NumCards = 40;
	
	enum Phase : uint8_t {
		clenup,
		cat_movement,
		dog_movement,
		movement,
		take_cards,
		discard_cards,
		play_1st_card,
		play_2nd_card,
		play_3rd_card
	};

	struct Deck
	{
		union {
			struct {
				uint16_t firstVisible : 4;
				uint16_t numVisible : 4;
			};
			uint16_t value;
		};
		Card		cards[NumCards];
		void discard(unsigned idx);
	};
}

using namespace GraWZombiaki;
struct GameState
{
	Plansza		plansza;
	Deck		zombieDeck;
	Deck		humanDeck;	
	uint8_t		phase : 7;
	uint8_t		current_player : 1;

	Deck& getPlayerDeck(int player) { return player == Player::zombie ? zombieDeck : humanDeck;	}
	const Deck& getPlayerDeck(int player) const { return player == Player::zombie ? zombieDeck : humanDeck;	}
};

struct Move
{
	enum Operation : uint8_t { noop, draw_cards, discard_card, play_card, use_card, move_cat, move_dog };
	union {
		struct {
			uint16_t op : 3;
			uint16_t params : 13;
		};
		uint16_t value;
	};
};

struct Mv_DrawCards : Move
{
	Mv_DrawCards(unsigned cnt)
	{
		op = Operation::draw_cards;
		params = cnt;
	}
	unsigned getCardsCnt() const { return params; }
};

struct Mv_DiscardCard : Move
{
	Mv_DiscardCard(unsigned idx)
	{
		op = Operation::discard_card;
		params = idx;
	}
	unsigned getCardIdx() const { return params; }
};

struct Mv_PlayCard : Move
{
	union Value {
		struct {
			uint16_t op			: 3;
			uint16_t przecznica : 3;
			uint16_t tor		: 2;
			uint16_t cardIdx		: 8;
		};
		uint16_t value;
	};
	Mv_PlayCard(uint16_t przecznica, uint16_t tor, uint16_t cardIdx)
	{
		Value v;
		v.przecznica = przecznica;
		v.tor = tor;
		v.cardIdx = cardIdx;
		value = v.value;
	}
	std::tuple<uint16_t, uint16_t, uint16_t> get() const
	{
		auto & val = *(Value*)&value;
		return { val.przecznica, val.tor, val.cardIdx };
	}
};

struct Mv_UseCard : Move
{
	Mv_UseCard(unsigned idx)
	{
		op = Operation::use_card;
		params = idx;
	}
	unsigned getCardIdx() const { return params; }
};

struct Mv_noop : Move
{
	Mv_noop()
	{
		op = Operation::noop;
	}
};

struct MoveList
{
	uint16_t size;
	Move move[1];
};

namespace GraWZombiaki
{
	struct GraWZombiakiZasady : IGameRules
	{
		GraWZombiakiZasady()
		{
			m_noop.size = 1;
			m_noop.move[0].op = Move::noop;
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
		void freeMoveList(MoveList*);
		
		static const Card* getZombieCards();
		static const Card* getHumanCards();
		MoveList* get_moves_in_phase_clenup(const GameState*);
		MoveList* get_moves_in_phase_cat_movement(const GameState*);
		MoveList* get_moves_in_phase_dog_movement(const GameState*);
		MoveList* get_moves_in_phase_general_movement(const GameState*);
		MoveList* getMovesInPhaseTakeCards(const GameState*);
		MoveList* getMovesInPhaseDiscardCards(const GameState*);
		MoveList* moveListFromVector(vector<uint16_t> moves);
		MoveList* getMovesInPhasePlayCard(const GameState*);

		GameState* drawCards(const GameState* gs, uint8_t numCards, int player);
		GameState* discardCard(const GameState* gs, uint8_t discardedCardIdx, int player);
		Card_If*   getCardIf(Card card);
		GameState* playCard(const GameState* gs, Mv_PlayCard* mv, int player);
		GameState* useCard(const GameState* gs, unsigned cardIdx, int player);

		using GetMoveListMethod_t = MoveList * (GraWZombiakiZasady::*)(const GameState*);
		static GetMoveListMethod_t getMovesInState[];

		ObjectPoolBlocked<GameState, 512>		m_GameStatePool;
		ObjectPoolMultisize<4 * sizeof(Move), 4096> m_moveListPool;
		MoveList m_noop;
	};
}