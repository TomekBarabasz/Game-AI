#include "pch.h"
#define BOOST_TEST_MODULE test_module
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>	//required for BOOST_DATA_TEST_CASE
#include <boost/test/data/monomorphic.hpp>	//required for boost::unit_test::data
#include <boost/mpl/list.hpp>
#include <GraWZombiakiZasady.h>
#include "random_generator.h"

namespace ut = boost::unit_test;
namespace butd = boost::unit_test::data;
using namespace GraWZombiaki;

namespace std {
	inline std::ostream& operator<<(std::ostream& out, const Player& value){
		out << (unsigned char)value;
		return out;
	}
	inline std::ostream& operator<<(std::ostream& out, const Phase& value){
		out << (unsigned char)value;
		return out;
	}
	/*inline std::ostream& operator<<(std::ostream& out, const Akcja& value) {
		out << (unsigned char)value;
		return out;
	}*/
	inline std::ostream& operator<<(std::ostream& out, const TypKarty& value) {
		out << (unsigned char)value;
		return out;
	}
}
struct RngMock : IRandomGenerator
{
	std::vector<int> numbers;
	std::vector<int> generateUniform(int lower, int upper, int number_of_samples) override
	{
		return numbers;
	}
	void release() override {}
};

#define BOOST_TEST_EQ_UINT8(a,b) BOOST_TEST(uint8_t(a) == uint8_t(b))
template <typename T, typename T1, typename T2>
void boost_test_eq(T1 a, T2 b)
{
	BOOST_TEST(static_cast<T>(a) == static_cast<T>(b));
}
BOOST_AUTO_TEST_SUITE(Testy_Plansza);
BOOST_AUTO_TEST_CASE(makeEmpty)
{
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Testy_Karty);
BOOST_AUTO_TEST_CASE(constructor)
{
	ZombieCard k(swit);

	BOOST_TEST_EQ_UINT8(k.getType(), TypKarty::akcja);
	BOOST_TEST_EQ_UINT8(k.typ, TypKarty::akcja);
	BOOST_TEST_EQ_UINT8(k.podtyp, AkcjaZombie::swit);
	boost_test_eq<uint8_t>(k.typ, TypKarty::akcja);
}
BOOST_AUTO_TEST_CASE(na_planszy)
{
	KartaNaPlanszy k;
	KartaLudziNaPlanszy kl;
	KartaZombieNaPlanszy kz;

	BOOST_TEST_EQ_UINT8(sizeof(k), sizeof(uint16_t));
	BOOST_TEST_EQ_UINT8(sizeof(kl), sizeof(uint16_t));
	BOOST_TEST_EQ_UINT8(sizeof(kz), sizeof(uint16_t));
}
BOOST_AUTO_TEST_SUITE_END()

struct CreateGameRules
{
	GraWZombiakiZasady gr;
};

BOOST_FIXTURE_TEST_SUITE(Testy_Zasady, CreateGameRules);
BOOST_AUTO_TEST_CASE(create_new_random)
{
	RngMock rng;
	rng.numbers = { 1,2,3,4,5,6,7,8,9,10,31,38 };
	auto gs = gr.CreateRandomInitialState(&rng);
	
	BOOST_TEST_EQ_UINT8(gs->phase, Phase::take_cards);
	BOOST_TEST_EQ_UINT8(gs->plansza.occupied_fields,0);
	BOOST_TEST_EQ_UINT8(gs->zombieDeck.firstVisible,0);
	BOOST_TEST_EQ_UINT8(gs->zombieDeck.numVisible,0);
	BOOST_TEST_EQ_UINT8(gs->humanDeck.firstVisible,0);
	BOOST_TEST_EQ_UINT8(gs->humanDeck.numVisible,0);
	BOOST_TEST_EQ_UINT8(gs->current_player, Player::zombie);
	auto * last = gs->zombieDeck.cards + 39;
	BOOST_TEST_EQ_UINT8(last->typ, TypKarty::akcja);
	BOOST_TEST_EQ_UINT8(last->podtyp, AkcjaZombie::swit);
}
BOOST_AUTO_TEST_CASE(dummy)
{
	
}

BOOST_AUTO_TEST_SUITE_END()
