#include "pch.h"
#include "Karty.h"
#include "GraWZombiakiZasady.h"
#include "Plansza.h"

namespace GraWZombiaki
{
	struct Zombiak_If : Card_If
	{
		unsigned getValidApplications(const GameState* gs, Card card, std::vector<uint16_t>& moves) override
		{
			return 0;
		}
		void play(GameState* gs, unsigned przecznica, unsigned tor, Card card) override
		{
		}
		void use(GameState* gs, Card card) override
		{
		}
	};

	struct Kot_If : Card_If
	{
		unsigned getValidApplications(const GameState* gs, Card card, std::vector<uint16_t>& moves) override
		{
			KartaNaPlanszy kot;
			kot.player = Player::zombie;
			kot.typ = ObiektZombie::kot;
			auto [pos, found] = gs->plansza.find(kot);
			if (found) {
				//TODO: fill valid move positions
			}
			return 0;
		}
		void play(GameState* gs, unsigned przecznica, unsigned tor, Card card) override
		{
		}
		void use(GameState* gs, Card card) override
		{
		}
	};

	struct Pies_If : Card_If
	{
		unsigned getValidApplications(const GameState* gs, Card card, std::vector<uint16_t>& moves) override
		{
			KartaNaPlanszy kot;
			kot.player = Player::zombie;
			kot.typ = ObiektZombie::kot;
			auto [pos, found] = gs->plansza.find(kot);
			if (found) {
				//TODO: fill valid move positions
			}
			return 0;
		}
		void play(GameState* gs, unsigned przecznica, unsigned tor, Card card) override
		{
		}
		void use(GameState* gs, Card card) override
		{
		}
	};
	
	struct Strzal_If : Card_If
	{
		unsigned getValidApplications(const GameState* gs, Card card, std::vector<uint16_t>& moves) override
		{
			return 0;
		}
		void play(GameState* gs, unsigned przecznica, unsigned tor, Card card) override
		{
		}
		void use(GameState* gs, Card card) override
		{
		}
	};
	
	static const ZombieCard zombieCards[] =
	{
		ZombieCard(zombiak_2)
		,ZombieCard(zombiak_2)
		,ZombieCard(zombiak_3)
		,ZombieCard(zombiak_3)
		,ZombieCard(zombiak_3)
		,ZombieCard(zombiak_3)
		,ZombieCard(zombiak_3)
		,ZombieCard(zombiak_3)
		,ZombieCard(zombiak_4)
		,ZombieCard(zombiak_4)
		,ZombieCard(zombiak_4)
		,ZombieCard(zombiak_5)
		,ZombieCard(kot)
		,ZombieCard(pies)
		,ZombieCard(krystyna)
		,ZombieCard(kon_trojanski)
		,ZombieCard(galareta)
		,ZombieCard(mlody)
		,ZombieCard(kuloodporny)
		,ZombieCard(syjamczyk)

		,ZombieCard(boss)
		,ZombieCard(mis)
		,ZombieCard(czlowiek)
		,ZombieCard(czlowiek)
		,ZombieCard(czlowiek)
		,ZombieCard(pazury)

		,ZombieCard(klik)
		,ZombieCard(klik)
		,ZombieCard(masa)
		,ZombieCard(masa)
		,ZombieCard(terror)
		,ZombieCard(terror)
		,ZombieCard(mieso)
		,ZombieCard(glod)
		,ZombieCard(wiadro)
		,ZombieCard(kilof)
		,ZombieCard(ugryzienie)
		,ZombieCard(spadaj)
		,ZombieCard(papu)
		,ZombieCard(swit)
	};
	
	const Card* GraWZombiakiZasady::getZombieCards()
	{
		return (Card*)zombieCards;
	}
	
	static const HumanCard humanCards[] =
	{
		HumanCard(strzal),
		HumanCard(strzal),
		HumanCard(strzal),
		HumanCard(strzal),
		HumanCard(strzal),
		HumanCard(strzal),
		HumanCard(strzal),
		HumanCard(lepszy_strzal),
		HumanCard(lepszy_strzal),
		HumanCard(lepszy_strzal),
		HumanCard(zmiataj),

		HumanCard(krotka_seria),
		HumanCard(dluga_seria),
		HumanCard(sniper),
		HumanCard(jajnik),
		HumanCard(wynocha),
		HumanCard(granat),
		HumanCard(stop),
		HumanCard(reflektor),
		HumanCard(v220),
		HumanCard(miotacz),
		HumanCard(gaz),
		HumanCard(raca),
		HumanCard(ropa),
		HumanCard(krew),
		HumanCard(ulica_w_ogniu),
		HumanCard(chuck),
		
		HumanCard(zapora),
		HumanCard(mur),
		HumanCard(siec),
		HumanCard(beczka),
		HumanCard(mina),
		HumanCard(samochod),
		HumanCard(dziura),

		HumanCard(betonowe_buty)
	};
	const Card* GraWZombiakiZasady::getHumanCards() {
		return (Card*)humanCards;
	}

	static Zombiak_If	zombiakIf;
	static Strzal_If	strzalIf;
	static Kot_If		kotIf;
	static Pies_If		piesIf;

	static std::map<Card, Card_If*> cardIfs =
	{
		{ ZombieCard(zombiak_1),		&zombiakIf },
		{ ZombieCard(zombiak_2),		&zombiakIf },
		{ ZombieCard(zombiak_3),		&zombiakIf },
		{ ZombieCard(zombiak_4),		&zombiakIf },
		{ ZombieCard(zombiak_5),		&zombiakIf },

		{ ZombieCard(kot),			&kotIf },
		{ ZombieCard(pies),			&piesIf },

		{ HumanCard(strzal),			&strzalIf },
		{ HumanCard(lepszy_strzal),	&strzalIf },
	};

	Card_If* GraWZombiakiZasady::getCardIf(Card card)
	{
		return cardIfs[card];
	}
}
