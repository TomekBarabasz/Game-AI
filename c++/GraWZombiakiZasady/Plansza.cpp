#include "pch.h"
#include "Plansza.h"

namespace GraWZombiaki
{
	std::tuple<Position, bool> Plansza::findByType(KartaNaPlanszy::Typ card, Position start) const
	{
		Position p;
		for (p.przecznica=start.przecznica;p.przecznica<Rows;++p.przecznica) {
			for (p.tor=start.tor;p.tor<Columns;++p.tor) {
				if (cards[p.przecznica][p.tor].bf_typ == card)	{
					return { p,true };
				}
			}
		}
		return { p,false };
	}
	std::tuple<Position, bool> Plansza::findByMask(uint16_t mask, Position start) const
	{
		Position p;
		for (p.przecznica = start.przecznica; p.przecznica < Rows; ++p.przecznica) {
			for (p.tor = start.tor; p.tor < Columns; ++p.tor) {
				if (cards[p.przecznica][p.tor].value & mask) {
					return { p,true };
				}
			}
		}
		return { p,false };
	}

	void Plansza::removeZapora()
	{
		zapora = 0;
	}

	void Plansza::removeNet()
	{
		for (int p= 0; p < Rows; ++p)
			for (int t=0; t < Columns; ++t)
				cards[p][t].bf_jest_zasieciowany = 0;
	}

	bool Plansza::isTerminal() const
	{
		return false;
	}

	void Plansza::moveCards()
	{
	}

	unsigned Plansza::distance(const Position& p1, const Position& p2)
	{
		unsigned dist = 0;
		if (p1.przecznica > p2.przecznica) {
			dist += p1.przecznica - p2.przecznica;
		}
		else {
			dist += p2.przecznica - p1.przecznica;
		}
		if (p1.tor > p2.tor) {
			dist += p1.tor - p2.tor;
		}
		else {
			dist += p2.tor - p1.tor;
		}
		return dist;
	}

	vector<Position> Plansza::listPositionsByType(const Position& pos, unsigned maxDistance, KartaNaPlanszy::Typ typ) const
	{
		vector<Position> result;
		Position p;
		for (p.przecznica = 0; p.przecznica < Rows; ++p.przecznica) {
			for (p.tor = 0; p.tor < Columns; ++p.tor) {
				const unsigned dist = distance(pos, p);
				if (dist != 0 && dist <= maxDistance && (*this)[p].bf_typ == typ) {
					result.push_back(p);
				}
			}
		}
		return result;
	}
}

