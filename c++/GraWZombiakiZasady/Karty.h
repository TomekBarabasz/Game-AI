#pragma once
#include <cstdint>
#include <functional>
#include <vector>

struct GameState;
struct MoveList;

namespace GraWZombiaki
{
	enum Player : uint8_t { zombie = 0, human = 1 };
	enum TypKarty : uint8_t { obiekt = 0, wzmocnienie, akcja };
	
	enum ObiektLudzi : uint8_t
	{
		zapora, mur, beczka, mina, samochod, dziura
	};
	enum ObiektZombie : uint8_t
	{
		zombiak_1, zombiak_2, zombiak_3, zombiak_4, zombiak_5,
		kot, pies,
		krystyna, kon_trojanski, kuloodporny, galareta, syjamczyk, mlody
	};
	enum AkcjaLudzi : uint8_t
	{
		strzal, lepszy_strzal, zmiataj, krotka_seria, dluga_seria, sniper, jajnik, wynocha, granat, stop,
		siec, reflektor, v220, miotacz, gaz, raca, ropa, krew, ulica_w_ogniu, chuck
	};
	enum AkcjaZombie : uint8_t
	{
		klik, masa, terror, mieso, kilof, glod, wiadro, ugryzienie, spadaj, papu, swit
	};
	enum WzmocnienieLudzi : uint8_t
	{
		betonowe_buty
	};

	enum WzmocnienieZombie : uint8_t
	{
		pazury, czlowiek, boss, mis
	};

	struct Card
	{
		union {
			struct {
				uint8_t player : 1;
				uint8_t typ : 2;
				uint8_t podtyp : 5;
			};
			uint8_t value;
		};
		Card(){}
		Card(uint8_t p, uint8_t t, uint8_t pt) : player(p), typ(t), podtyp(pt){}
		Player getPlayer() const { return static_cast<Player>(player); };
		TypKarty getType() const { return static_cast<TypKarty>(typ); }
		uint8_t getSubType() const { return podtyp; }
	};
	inline bool operator< (const Card& lhs, const Card& rhs) { return lhs.value < rhs.value; }
	struct ZombieCard : Card
	{
		ZombieCard(ObiektZombie z) : Card(zombie, TypKarty::obiekt, z){}
		ZombieCard(AkcjaZombie a)	: Card(zombie, akcja, a) {}
		ZombieCard(WzmocnienieZombie w): Card(zombie, wzmocnienie, w) {}
	};
	struct HumanCard : Card
	{
		HumanCard(ObiektLudzi z) : Card(human, TypKarty::obiekt, z) {}
		HumanCard(AkcjaLudzi a) : Card(human, akcja, a) {}
		HumanCard(WzmocnienieLudzi w) : Card(human, wzmocnienie, w) {}
	};

	struct Card_If
	{
		virtual unsigned  getValidApplications	(const GameState* gs, Card card, std::vector<uint16_t>& moves) = 0;
		virtual void	  play					(GameState* gs, unsigned przecznica, unsigned tor, Card card) = 0;
		virtual void	  use					(GameState* gs, Card card) = 0;
	};
}
