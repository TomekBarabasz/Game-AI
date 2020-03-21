#pragma once
#include <cstdint>
#include <functional>
#include <vector>
#include "Common.h"
#include <cassert>

struct GameState;
struct MoveList;

namespace GraWZombiaki
{
	static const int NumZombieCards = 40;
	static const int NumHumanCards = 40;
	static const int NumCards = 40;

	enum TypKarty : uint8_t { obiekt = 0, wzmocnienie, akcja };
	
	enum ObiektLudzi : uint8_t
	{
		zapora, mur, beczka, mina, samochod, dziura
	};
	enum ObiektZombie : uint8_t
	{
		zombiak_1=1,
		kot, pies, krystyna, kon_trojanski, kuloodporny, galareta, syjamczyk, mlody,
		zombiak_2, zombiak_3, zombiak_4, zombiak_5,
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
		//get valid usage is called when card will be played from deck
		//and will produce valid moves that will result in a call to place or use
		virtual void	getValidUsage	(const GameState* gs, unsigned cardIdx, MoveList_t& moves) = 0;
		virtual void	place			(GameState* gs, Position where, unsigned cardIdx) = 0;
		virtual void	use				(GameState* gs, unsigned cardIdx) = 0;

		//get valid moves is called when card in on the board and has options to move (like a cat or dog)
		//this call will produce valid moves that wil result in a call to move
		virtual void	getValidMoves	(const GameState* gs, Position p, MoveList_t& moves) = 0;
		virtual	void	move			(GameState* gs, Position from, Position to) = 0;
		
		virtual void	hit				(GameState* gs, Position where) = 0;
	};

	struct Deck
	{
		union {
			struct {
				uint16_t firstVisible : 4;
				uint16_t numVisible : 4;
			};
			uint16_t value;
		};
		Card cards[NumCards];
		void discard(unsigned idx)
		{
			assert(idx >= unsigned(firstVisible) && idx < unsigned(firstVisible + numVisible));
			if (idx > firstVisible) {
				Card tmp = cards[firstVisible];
				cards[firstVisible] = cards[idx];
				cards[idx] = tmp;
			}
			firstVisible += 1;
			numVisible -= 1;
		}
	};
}
