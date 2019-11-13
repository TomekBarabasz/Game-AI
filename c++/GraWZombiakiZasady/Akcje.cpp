#include "pch.h"
#include "GraWZombiakiZasady.h"

namespace GraWZombiaki
{
	MoveList* GraWZombiakiZasady::get_moves_in_pase_clenup(const GameState*)
	{
		return &m_noop;
	}

	MoveList* GraWZombiakiZasady::get_moves_in_pase_cat_movement(const GameState*)
	{
		return &m_noop;
	}

	MoveList* GraWZombiakiZasady::get_moves_in_pase_dog_movement(const GameState*)
	{
		return &m_noop;
	}

	MoveList* GraWZombiakiZasady::get_moves_in_pase_general_movement(const GameState*)
	{
		return &m_noop;
	}

	MoveList* GraWZombiakiZasady::get_moves_in_pase_take_cards(const GameState*)
	{
		return &m_noop;
	}

	MoveList* GraWZombiakiZasady::get_moves_in_pase_discard_cards(const GameState*)
	{
		return &m_noop;
	}

	MoveList* GraWZombiakiZasady::get_moves_in_pase_play_1st_card(const GameState*)
	{
		return &m_noop;
	}

	MoveList* GraWZombiakiZasady::get_moves_in_pase_play_2nd_card(const GameState*)
	{
		return &m_noop;
	}

	MoveList* GraWZombiakiZasady::get_moves_in_pase_play_3rd_card(const GameState*)
	{
		return &m_noop;
	}

	GameState* GraWZombiakiZasady::dobierzKarty(const GameState* gs, uint8_t num_cards, int player)
	{
		auto ngs = allocGameState();
		return ngs;
	}

	GameState* GraWZombiakiZasady::odrzucKarte(const GameState* gs, uint8_t card, int player)
	{
		auto ngs = allocGameState();
		return ngs;
	}

	GameState* GraWZombiakiZasady::zagrajKarte(const GameState* gs, Move_play_card* move_play_card, int player)
	{
		auto ngs = allocGameState();
		return ngs;
	}

	GameState* GraWZombiakiZasady::aktywujKarte(const GameState* gs, Move_activate_card* mv, int player)
	{
		auto ngs = allocGameState();
		return ngs;
	}
}
