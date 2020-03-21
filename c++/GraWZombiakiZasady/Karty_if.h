#pragma once

#include "Karty.h"
#include "Plansza.h"
#include <cassert>

namespace GraWZombiaki
{
	struct ZombieBase_If : Card_If
	{
		void getValidUsage(const GameState* gs, unsigned cardIdx, MoveList_t& moves) override
		{
			for (unsigned tor = 0; tor < Plansza::Columns; ++tor) {
				if (gs->plansza.isEmpty({ 0, tor })) {
					auto mv = Mv_PlayCard({ 0, tor }, cardIdx);
					moves.push_back(mv.value);
				}
			}
		}

		void place(GameState* gs, Position where, unsigned cardIdx) override
		{
			const Card card = gs->zombieDeck.cards[cardIdx];
			const auto type = card.getSubType();
			KartaNaPlanszy znp(Player::zombie, type <= ObiektZombie::mlody ? type : KartaNaPlanszy::zombiak);
			static const unsigned Sila[]{ 0, 1, 2, 1, 3, 2, 3, 3, 2, 2, 2, 3, 4, 5 };	//w kolejności jak ObiektZombie

			znp.bf_sila = Sila[type];
			KartaNaPlanszy& slot = gs->plansza[where];
			assert(slot.bf_typ == KartaNaPlanszy::puste_miejsce);
			slot = znp;
		}

		void	getValidMoves(const GameState* gs, Position p, MoveList_t& moves) override {}
		void	use(GameState* gs, unsigned cardIdx) override {}
	};
	
	struct Zombiak_If : ZombieBase_If
	{
		void	move(GameState* gs, Position from, Position to) {}
		void	hit(GameState* gs, Position where) {}
	};

	struct Kot_If : ZombieBase_If
	{
		//TODO: need to rework to make sure cat does not go throught human objects
		void getValidMoves(const GameState* gs, Position p0, MoveList_t& moves)
		{
			Position p;
			for (p.przecznica = 0; p.przecznica < Plansza::Rows; ++p.przecznica) {
				for (p.tor = 0; p.tor < Plansza::Columns; ++p.tor) {
					const auto distance = Plansza::distance(p0, p);
					if (distance != 0 && distance <= 2) {
						const auto& k = gs->plansza[p];
						if (k.bf_typ == KartaNaPlanszy::puste_miejsce || k.bf_player == Player::zombie) {
							moves.push_back(makeMove<Mv_MoveCard>(p0, p));
						}
					}
				}
			}
			moves.push_back(makeMove<Mv_Noop>());
		}
		void place(GameState* gs, Position where, unsigned cardIdx) override
		{
			ZombieBase_If::place(gs, where, cardIdx);
			gs->plansza.karta_pod_kotem.bf_typ = KartaNaPlanszy::puste_miejsce;
		}
		void move(GameState* gs, Position from, Position to)
		{
			auto& k_from = gs->plansza[from];
			auto& k_to = gs->plansza[to];
			
			uint16_t kot = k_from.value;
			k_from = gs->plansza.karta_pod_kotem;
			gs->plansza.karta_pod_kotem = k_to;
			k_to.value = kot;
		}
		void	hit(GameState* gs, Position where) {}
	};
	
	struct Pies_If : ZombieBase_If
	{
		void getValidMoves(const GameState* gs, Position p0, MoveList_t& moves)
		{
			Position p;
			for (p.przecznica = 0; p.przecznica < Plansza::Rows; ++p.przecznica) {
				for (p.tor = 0; p.tor < Plansza::Columns; ++p.tor) {
					const auto distance = Plansza::distance(p0, p);
					if (distance != 0 && distance <= 3) {
						const auto& k = gs->plansza[p];
						if (k.bf_typ == KartaNaPlanszy::puste_miejsce) {
							moves.push_back(makeMove<Mv_MoveCard>(p0, p));
						}
					}
				}
			}
			moves.push_back(makeMove<Mv_Noop>());
		}
		void move(GameState* gs, Position from, Position to)
		{
			auto& k_from = gs->plansza[from];
			gs->plansza[to] = k_from;
			k_from.bf_typ = KartaNaPlanszy::puste_miejsce;
		}
		void	hit(GameState* gs, Position where) {}
	};

	struct Boss_If : Card_If
	{
		void getValidUsage(const GameState* gs, unsigned cardIdx, MoveList_t& moves) override {}
		void place(GameState* gs, Position where, unsigned cardIdx) override {}
		void use(GameState* gs, unsigned cardIdx) override {}
		void getValidMoves(const GameState* gs, Position p, MoveList_t& moves) override {}
		void move(GameState* gs, Position from, Position to) override {}
		void hit(GameState* gs, Position where) override {}
	};
	
	struct Strzal_If : Card_If
	{
		void getValidUsage(const GameState* gs, unsigned cardIdx, MoveList_t& moves) override {}
		void getValidMoves(const GameState* gs, Position p, MoveList_t& moves) override {}
		void place(GameState* gs, Position where, unsigned cardIdx) override {}
		void use(GameState* gs, unsigned cardIdx) override {}
		void move(GameState* gs, Position from, Position to) override {}
		void hit(GameState* gs, Position where) override {}
	};
}
