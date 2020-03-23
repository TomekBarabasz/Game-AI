#include "pch.h"
#include "Karty.h"
#include "GraWZombiakiZasady.h"
#include "Karty_if.h"

namespace GraWZombiaki
{
	Zombiak_If	zombiakIf;
	Strzal_If	strzalIf;
	Kot_If		kotIf;
	Pies_If		piesIf;

	std::map<Card, Card_If*> cardIfs =
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

	Card_If* GraWZombiakiZasady::getCardIf(KartaNaPlanszy knp)
	{
		Card c;
		c.player = knp.bf_player;
		c.typ = TypKarty::obiekt;

		static const uint8_t typ2podtyp[] =
		{
			/*puste_miejsce*/0,
			/*kot*/ObiektZombie::kot,
			/*pies*/ObiektZombie::pies,
			/*zombiak*/ObiektZombie::zombiak_1,
			/*krystyna*/ObiektZombie::krystyna,
			/*kon_trojanski*/ObiektZombie::kon_trojanski,
			/*kuloodporny*/ObiektZombie::kuloodporny,
			/*galareta*/ObiektZombie::galareta,
			/*syjamczyk*/ObiektZombie::syjamczyk,
			/*mlody*/ObiektZombie::mlody,
			/*mur*/ObiektLudzi::mur,
			/*beczka*/ObiektLudzi::beczka,
			/*mina*/ObiektLudzi::mina,
			/*samochod*/ObiektLudzi::samochod,
			/*dziura*/ObiektLudzi::dziura
		};
		c.podtyp = typ2podtyp[knp.bf_typ];
		return getCardIf(c);
	}
}
