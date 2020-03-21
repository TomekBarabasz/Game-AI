#include "pch.h"
#include "Karty.h"
#include "GraWZombiakiZasady.h"
#include "Plansza.h"

namespace GraWZombiaki
{
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
}
