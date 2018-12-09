#pragma once
#include "GameRules.h"

struct Card
{
	unsigned char owner : 1;	//zombi,człowiek
	unsigned char moveable : 1;
};

struct Plansza
{
	Card cards[4 * 5 * 4];
	Card* getPosition(int przecznica, int tor)
	{
		return cards + przecznica * 4 + tor;
	}
};

struct GameState
{
	unsigned char current_player : 1;
	unsigned char phase : 3;
	Plansza plansza;
	Card cmentarz[4];
	Card barykada[4];
	Card ludzie[40], zombie[40];
};

struct Move
{
	enum operation : unsigned char { dobierz_karty, odrzuc_karte, zagraj_karte, aktywuj_zdolnosc };
};

struct MoveList
{
};

namespace GraWZombiaki
{
	struct GraWZombiakiZasady : IGameRules
	{
		GraWZombiakiZasady()
		{

		}
		~GraWZombiakiZasady()
		{

		}

		void SetRandomGenerator(IRandomGenerator*) override;
		GameState* CreateRandomInitialState(IRandomGenerator*) override;
		GameState* CreateInitialStateFromHash(const uint32_t*) override;
		GameState* CreateStateFromString(const wstring&) override;
		GameState* CopyGameState(const GameState*) override;
		bool AreEqual(const GameState*, const GameState*) override;
		void ReleaseGameState(GameState*) override;
		bool IsTerminal(const GameState*) override;
		void Score(const GameState*, int score[]) override;
		int GetCurrentPlayer(const GameState*) override;
		MoveList* GetPlayerLegalMoves(const GameState*, int playerNum) override;
		void ReleaseMoveList(MoveList*) override;
		int GetNumMoves(const MoveList*) override;
		Move* GetMoveFromList(MoveList*, int idx) override;
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
	};
}