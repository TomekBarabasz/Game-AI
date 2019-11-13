#include "pch.h"
#include "Karty.h"
#include "GraWZombiakiZasady.h"

namespace GraWZombiaki
{
	static const uint8_t KartyZombi[] =
	{
		KZombiak (TypZombie::zwykly_2 )
		,KZombiak(TypZombie::zwykly_2 )
		,KZombiak(TypZombie::zwykly_3 )
		,KZombiak(TypZombie::zwykly_3 )
		,KZombiak(TypZombie::zwykly_3 )
		,KZombiak(TypZombie::zwykly_3 )
		,KZombiak(TypZombie::zwykly_3 )
		,KZombiak(TypZombie::zwykly_3 )
		,KZombiak(TypZombie::zwykly_4 )
		,KZombiak(TypZombie::zwykly_4 )
		,KZombiak(TypZombie::zwykly_4 )
		,KZombiak(TypZombie::zwykly_5 )
		,KZombiak(TypZombie::kot )
		,KZombiak(TypZombie::pies )
		,KZombiak(TypZombie::krystyna )
		,KZombiak(TypZombie::kon_trojanski )
		,KZombiak(TypZombie::galareta )
		,KZombiak(TypZombie::mlody )
		,KZombiak(TypZombie::kuloodporny )
		,KZombiak(TypZombie::syjamczyk )

		,KWzmocnienie(Wzmocnienie::boss)
		,KWzmocnienie(Wzmocnienie::mis)
		,KWzmocnienie(Wzmocnienie::czlowiek)
		,KWzmocnienie(Wzmocnienie::czlowiek)
		,KWzmocnienie(Wzmocnienie::czlowiek)
		,KWzmocnienie(Wzmocnienie::pazury)

		,KAkcja(Akcja::klik)
		,KAkcja(Akcja::klik)
		,KAkcja(Akcja::masa)
		,KAkcja(Akcja::masa)
		,KAkcja(Akcja::terror)
		,KAkcja(Akcja::terror)
		,KAkcja(Akcja::mieso)
		,KAkcja(Akcja::glod)
		,KAkcja(Akcja::wiadro)
		,KAkcja(Akcja::kilof)
		,KAkcja(Akcja::ugryzienie)
		,KAkcja(Akcja::spadaj)
		,KAkcja(Akcja::papu)
		,KAkcja(Akcja::swit)
	};
	/*static const uint8_t KartyZombi_invalid[] =
	{
		KartaZombie::Zombiak(TypZombie::zwykly_2)// L"Krzysztof")
		,KartaZombie::Zombiak(TypZombie::zwykly_2)// L"Czesiek")
		,KartaZombie::Zombiak(TypZombie::zwykly_3)// L"Wacek")
		,KartaZombie::Zombiak(TypZombie::zwykly_3)// L"Kazimierz")
		,KartaZombie::Zombiak(TypZombie::zwykly_3)// L"Andrzej")
		,KartaZombie::Zombiak(TypZombie::zwykly_3)// L"Arkadiusz")
		,KartaZombie::Zombiak(TypZombie::zwykly_3)//L"Zenek")
		,KartaZombie::Zombiak(TypZombie::zwykly_3)//L"Mietek")
		,KartaZombie::Zombiak(TypZombie::zwykly_4)//,L"Marian")
		,KartaZombie::Zombiak(TypZombie::zwykly_4)//,L"Mariusz")
		,KartaZombie::Zombiak(TypZombie::zwykly_4)//, L"Stefan")
		,KartaZombie::Zombiak(TypZombie::zwykly_5)//, L"Iwan")
		,KartaZombie::Zombiak(TypZombie::kot)//,L"kot")
		,KartaZombie::Zombiak(TypZombie::pies)//,L"pies")
		,KartaZombie::Zombiak(TypZombie::krystyna)// 16,L"krystynka")
		,KartaZombie::Zombiak(TypZombie::kon_trojanski)//ski, 17, L"koń trojański")
		,KartaZombie::Zombiak(TypZombie::galareta)//18, L"galareta")
		,KartaZombie::Zombiak(TypZombie::mlody)// L"młody")
		,KartaZombie::Zombiak(TypZombie::kuloodporny)//y, 20,L"kuloodporny")
		,KartaZombie::Zombiak(TypZombie::syjamczyk)// 21, L"syjamczyk")
	};*/

	static const uint8_t KartyLudzi =
	{
	};
	const uint8_t* GraWZombiakiZasady::getZombieCards() {
		return (uint8_t*) KartyZombi;
	}
	const uint8_t* GraWZombiakiZasady::getHumanCards() {
		return (uint8_t*) KartyLudzi;
	}
}
