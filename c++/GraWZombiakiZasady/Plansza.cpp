#include "pch.h"
#include "Plansza.h"

namespace GraWZombiaki
{
	std::tuple<Position, bool> Plansza::find(KartaNaPlanszy card, Position p) const
	{
		for (;p.przecznica<5;++p.przecznica) {
			for (;p.tor<4;++p.tor) {
				if (cards[p.tor][p.przecznica] == card)	{
					return { p,true };
				}
			}
		}
		return { p,false };
	}
}

