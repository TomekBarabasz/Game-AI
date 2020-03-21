#pragma once
#include <cstdint>
#include <vector>
#include "Common.h"

using std::vector;
namespace GraWZombiaki
{
	struct KartaNaPlanszy
	{
		KartaNaPlanszy()
		{ value = 0; }
		KartaNaPlanszy(int p, int t)
		{
			value = 0;
			bf_player = p;
			bf_typ = t;
		}
		enum Typ {	puste_miejsce,
					zombiak, kot, pies, krystyna, kon_trojanski, kuloodporny, galareta, syjamczyk, mlody,
					mur, beczka, mina, samochod, dziura };
		
		union {
			struct
			{
				uint16_t bf_player			  : 1;
				uint16_t bf_typ				  : 4;
				uint16_t bf_empty			  : 1;
				uint16_t bf_jest_bossem		  : 1;
				uint16_t bf_ma_czlowieka	  : 1;
				uint16_t bf_ma_misia		  : 1;
				uint16_t bf_ma_betonowe_buty  : 1;
				uint16_t bf_jest_zasieciowany : 1;
				uint16_t bf_sila			  : 5;	//iwan 5 + dwa od galarety + masa (młody 6) + masa (4) + pazury
			};
			uint16_t value;
		};
		bool operator==(const KartaNaPlanszy& other) const { return value == other.value; }
	};
;
	struct Plansza
	{
		static constexpr unsigned Columns = 3;
		static constexpr unsigned Rows = 5;
		KartaNaPlanszy cards[Rows][Columns];
		union {
			struct {
				uint16_t	zapora : 4;
				uint16_t	rozkazy_bossa : 2;
				uint16_t	zycie_misia : 3;
			};
			uint16_t value;
			KartaNaPlanszy karta_pod_kotem;
		};
		
		Plansza() : value(0)
		{
			memset(cards, 0, Columns * Rows * sizeof(KartaNaPlanszy));
		}
		bool isEmpty(Position p) const
		{
			return KartaNaPlanszy::puste_miejsce == (*this)[p].bf_typ;
		}
		std::tuple<Position, bool> findByType(KartaNaPlanszy::Typ card, Position start={0,0}) const;
		std::tuple<Position, bool> findByMask(uint16_t mask, Position start={0,0}) const;
		KartaNaPlanszy& operator[](Position p) { return cards[p.przecznica][p.tor]; }
		const KartaNaPlanszy& operator[](Position p) const { return cards[p.przecznica][p.tor]; }
		void removeZapora();
		void removeNet();
		//isTerminal return true if there's at least one free to move zombie in 5th przecznica
		bool isTerminal() const;
		void moveCards();
		static unsigned distance(const Position& p1, const Position& p2);
		vector<Position> listPositionsByType(const Position& pos, unsigned maxDistance, KartaNaPlanszy::Typ typ) const;
	};
}
