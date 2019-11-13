#pragma once
#include <cstdint>

namespace GraWZombiaki
{
	enum class Gracz : uint8_t { zombie = 0, human = 1 };
	enum class TypKartyZombie : uint8_t { zombiak = 0, wzmocnienie, akcja };
	enum class TypZombie
	{
		zwykly_2 = 0,
		zwykly_3,
		zwykly_4,
		zwykly_5,
		kot,
		pies,
		krystyna,
		kon_trojanski,
		kuloodporny,
		galareta,
		syjamczyk,
		mlody
	};
	enum class Akcja : uint8_t { klik, masa, terror, mieso, kilof, glod, wiadro, ugryzienie, spadaj, papu, swit };
	enum class Wzmocnienie : uint8_t { pazury = 0, czlowiek, boss, mis };
	struct KartaZombie
	{
		union {
			struct {
				uint8_t gracz  : 1;
				uint8_t typ    : 2;
				uint8_t podtyp : 5;
			};
			uint8_t value;
		};
		//KartaZombie(){}
		KartaZombie(TypZombie z)  : gracz(uint8_t(Gracz::zombie)), typ(uint8_t(TypKartyZombie::zombiak)),	  podtyp(uint8_t(z)) {}
		KartaZombie(Akcja a)	  : gracz(uint8_t(Gracz::zombie)), typ(uint8_t(TypKartyZombie::akcja)),		  podtyp(uint8_t(a)) {}
		KartaZombie(Wzmocnienie w): gracz(uint8_t(Gracz::zombie)), typ(uint8_t(TypKartyZombie::wzmocnienie)), podtyp(uint8_t(w)) {}
		
		/*static KartaZombie makeKAkcja(Akcja a)
		{
			KartaZombie k{ {uint8_t(Gracz::zombie), uint8_t(TypKartyZombie::akcja), uint8_t(a)} };
			return k;
		}*/
		/*static uint8_t Akcja(Akcja a)
		{
			KartaZombie kz;
			kz.gracz = Gracz::zombie;
			kz.typ = TypKartyZombie::akcja;
			kz.podtyp = (uint8_t)a;
			return kz.value;
		}
		static uint8_t Wzmocnienie(Wzmocnienie w)
		{
			KartaZombie kz;
			kz.gracz = Gracz::zombie;
			kz.typ = TypKartyZombie::wzmocnienie;
			kz.podtyp = (uint8_t)w;
			return kz.value;
		}*/
	};
	inline uint8_t KZombiak(TypZombie z)	   { KartaZombie k(z); return k.value; }
	inline uint8_t KAkcja(Akcja a)			   { KartaZombie k(a); return k.value; }
	inline uint8_t KWzmocnienie(Wzmocnienie w) { KartaZombie k(w); return k.value; }
	template <typename PODTYP>
	uint8_t makeKartaZombie(PODTYP p)
	{
		KartaZombie k(p);
		return k.value;
	}
}
