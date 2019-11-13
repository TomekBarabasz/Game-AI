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
	inline std::ostream& operator<<(std::ostream& out, const Gracz& value){
		out << (unsigned char)value;
		return out;
	}
	inline std::ostream& operator<<(std::ostream& out, const Phase& value){
		out << (unsigned char)value;
		return out;
	}
	inline std::ostream& operator<<(std::ostream& out, const Akcja& value) {
		out << (unsigned char)value;
		return out;
	}
	inline std::ostream& operator<<(std::ostream& out, const TypKartyZombie& value) {
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

BOOST_AUTO_TEST_SUITE(Testy_Plansza);
BOOST_AUTO_TEST_CASE(makeEmpty)
{
	Plansza p;
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Testy_Karty);
BOOST_AUTO_TEST_CASE(first)
{
	/*KartaZombi_Zombiak kzz;
	kzz.value = 0;
	kzz.typ = static_cast<unsigned short>(TypKartyZombie::akcja);
	kzz.gracz = 1;

	BOOST_TEST(0b0000000000000101 == kzz.value);*/
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
	BOOST_TEST(gs->phase == Phase::take_cards);
	BOOST_TEST(static_cast<unsigned char>(gs->cmentarz_cnt) == 0);
	BOOST_TEST(static_cast<unsigned char>(gs->barykada_cnt) == 0);
	BOOST_TEST(static_cast<unsigned char>(gs->ludzie_talia_idx) == 0);
	BOOST_TEST(static_cast<unsigned char>(gs->zombie_talia_idx) == 0);
	BOOST_TEST(static_cast<Gracz>(gs->current_player) == Gracz::zombie);
	KartaZombie* last = reinterpret_cast<KartaZombie*>(gs->zombie + 39);
	BOOST_TEST(static_cast<TypKartyZombie >(last->typ) == TypKartyZombie::akcja);
	BOOST_TEST(static_cast<Akcja>(last->podtyp) == Akcja::swit);
}
BOOST_AUTO_TEST_CASE(dummy)
{
	KartaZombie k(Akcja::swit);
	//TODO: fix me - powinno działać bez linijki pod spodem
	k.value = KAkcja(Akcja::swit);

	BOOST_TEST((uint8_t)k.typ == (uint8_t)TypKartyZombie::akcja);
	BOOST_TEST((uint8_t)k.podtyp == (uint8_t)Akcja::swit);
}

BOOST_AUTO_TEST_SUITE_END()
