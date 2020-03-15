#pragma once
#include <cstdint>
#include <vector>

using std::vector;
namespace GraWZombiaki
{
	struct KartaNaPlanszy
	{
		KartaNaPlanszy()
		{ value = 0; }
		union {
			struct
			{
				uint8_t player : 1;
				uint8_t typ : 5;
				uint8_t bf_ma_czlowieka : 1;
				uint8_t bf_jest_bossem : 1;
				uint8_t	bf_ma_misia : 1;
				uint8_t	bf_ma_betonowe_buty : 1;
				uint8_t	bf_jest_zasieciowany : 1;
				uint8_t	f_sila : 5;	//iwan 5 + dwa od galarety + masa (młody 6) + masa (4) + pazury
			};
			uint16_t value;
		};
		bool operator==(const KartaNaPlanszy& other) const { return value == other.value; }
	};

	using KartaLudziNaPlanszy = KartaNaPlanszy;

	struct KartaZombieNaPlanszy : KartaNaPlanszy
	{
		bool ma_czlowieka() const { return bf_ma_czlowieka; }
		bool jest_bossem()	const { return bf_jest_bossem; }
		bool ma_misia()		const { return bf_ma_misia; }
		bool ma_betonowe_buty() const { return bf_ma_betonowe_buty; }
		bool jest_zasieciowany() const { return bf_jest_zasieciowany; }
		uint8_t sila()		const { return f_sila; }
	};

	struct Position
	{
		unsigned przecznica, tor;
	};
	struct Plansza
	{
		KartaNaPlanszy	cards[4][5];
		uint32_t		occupied_fields;

		Plansza() : occupied_fields(0) {}
		bool isEmpty(unsigned przecznica, unsigned tor) const;
		vector<Position> getEmptyPositions(unsigned przecznica) const;
		std::tuple<Position, bool> find(KartaNaPlanszy kot, Position start={0,0}) const;
	};
}
