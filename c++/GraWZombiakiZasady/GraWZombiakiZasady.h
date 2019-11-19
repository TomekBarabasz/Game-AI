#pragma once
#include "GameRules.h"
#include "object_pool.h"
#include "Karty.h"

namespace GraWZombiaki
{
	struct Pole
	{
		uint8_t idx;
		uint8_t size;
	};

	struct Plansza
	{
		Pole cards[4][5];
	};

	using Karta = uint8_t;

	struct Karta_na_planszy
	{
		Gracz gracz : 1;
		uint8_t dummy1: 7;
		uint8_t dummy2;
		uint8_t dummy3;
	};
	struct KartaZombie_na_planszy
	{
		Gracz gracz : 1;
		enum TypZombie : uint8_t
		{
			zwykly,
			kot,
			pies,
			krystyna,
			kon_trojanski,
			kuloodporny,
			galareta,
			zeton_po_galarecie,
			syjamczyk,
			mlody
		} typ : 4;
		
		uint8_t	ma_czlowieka : 1;
		uint8_t	jest_bossem : 1;
		uint8_t	ma_misia : 1;
		uint8_t sila : 5;	//iwan 5 + dwa od galarety + masa (młody 6) + masa (4) + pazury
		uint8_t	ma_betonowe_buty : 1;
		uint8_t	jest_zasieciowany : 1;
		uint8_t rany : 5;
	};
	struct KartaLudzi_na_planszy
	{
		Gracz gracz : 1;
		enum TypKarty : uint8_t
		{
			dziura,
			beczka,
			samochod,
			mur_slaby,
			mur_mocny,
			zapora,
		} typ : 4;
	};

	enum class Phase : uint8_t {
		clenup,
		movement,	//cat, dog, zombie, human ?
		take_cards,
		discard_cards,
		play_1st_card,
		play_2nd_card,
		play_3rd_card
	};
	static const int NumZombieCards = 40;
	static const int NumHumanCards = 40;
	static const int MaxNumCardsonBoard = 24;	//20 miejsc + kot + pies + dzura + zeton_po_galarecie
}

using namespace GraWZombiaki;
struct GameState
{
	Plansza plansza;
	Phase	phase;

	uint8_t current_player : 1;
	uint8_t terror : 1;				//ludzie zagrywają tylko jedną kartę
	uint8_t zombie_stop : 1;		//zombie nie wykonują ruchu
	uint8_t aktywacja_misia : 1;	//ludzie odrzucają dwie karty

	uint8_t cmentarz_idx : 6;
	uint8_t cmentarz_cnt : 2;

	uint8_t barykada_idx : 6;
	uint8_t barykada_cnt : 2;

	uint8_t ludzie_talia_idx;
	uint8_t zombie_talia_idx;
	Karta_na_planszy karty_na_planszy[MaxNumCardsonBoard];
	Karta ludzie[NumHumanCards];
	Karta zombie[NumZombieCards];
};

struct Move
{
	enum Operation : uint8_t { noop, dobierz_karty, odrzuc_karte, zagraj_karte, aktywuj_karte } operation;
};
struct Move_take_cards : Move
{
	uint8_t num_cards;
	uint8_t pad;
};
struct Move_discard_card : Move
{
	uint8_t card;
	uint8_t pad;
};
struct Move_play_card : Move
{
	uint8_t przecznica : 4;
	uint8_t tor : 4;
	uint8_t card;
};
struct Move_activate_card : Move
{
	uint8_t card;
	uint8_t pad;
};
struct Move_noop : Move
{
	uint8_t pad1;
	uint8_t pad2;
};
struct MoveList
{
	union {
		unsigned short	size;
		Move_noop		dummy;
	};
	Move_noop move[1];
};

namespace GraWZombiaki
{
	struct GraWZombiakiZasady : IGameRules
	{
		GraWZombiakiZasady()
		{
			m_noop.size = 1;
			m_noop.move[0].operation = Move::noop;
		}
		~GraWZombiakiZasady()
		{

		}

		void SetRandomGenerator(IRandomGenerator*) override;
		GameState* CreateRandomInitialState(IRandomGenerator*) override;
		GameState* CreateInitialStateFromHash(const uint32_t*) override;
		GameState* CreateStateFromString(const wstring&) override;
		GameState* CreatePlayerKnownState(const GameState*, int playerNum) override;
		void		UpdatePlayerKnownState(GameState* playerKnownState, const GameState* completeGameState, const std::vector<MoveList*>& playerMoves, int playerNum) override;
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
		static const unsigned char* getZombieCards();
		static const unsigned char* getHumanCards();
		MoveList* get_moves_in_pase_clenup(const GameState*);
		MoveList* get_moves_in_pase_cat_movement(const GameState*);
		MoveList* get_moves_in_pase_dog_movement(const GameState*);
		MoveList* get_moves_in_pase_general_movement(const GameState*);
		MoveList* get_moves_in_pase_take_cards(const GameState*);
		MoveList* get_moves_in_pase_discard_cards(const GameState*);
		MoveList* get_moves_in_pase_play_1st_card(const GameState*);
		MoveList* get_moves_in_pase_play_2nd_card(const GameState*);
		MoveList* get_moves_in_pase_play_3rd_card(const GameState*);

		GameState* dobierzKarty(const GameState* gs, uint8_t num_cards, int player);
		GameState* odrzucKarte(const GameState* gs, uint8_t card, int player);
		GameState* zagrajKarte(const GameState* gs, Move_play_card* move_play_card, int player);
		GameState* aktywujKarte(const GameState* gs, Move_activate_card* mv, int player);

		using GetMoveListMethod_t = MoveList * (GraWZombiakiZasady::*)(const GameState*);
		static GetMoveListMethod_t getMovesInState[];
		ObjectPoolBlocked<GameState, 512>		m_GameStatePool;
		MoveList m_noop;
	};
}