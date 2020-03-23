#pragma once
#include "Common.h"
#include <ostream>
#include "Karty.h"
#include "GraWZombiakiZasady.h"

namespace std {
	inline std::ostream& operator<<(std::ostream& out, const Player& value) {
		out << (unsigned char)value;
		return out;
	}
	inline std::ostream& operator<<(std::ostream& out, const Phase& value) {
		out << (unsigned char)value;
		return out;
	}
	/*inline std::ostream& operator<<(std::ostream& out, const Akcja& value) {
		out << (unsigned char)value;
		return out;
	}*/
	inline std::ostream& operator<<(std::ostream& out, const GraWZombiaki::TypKarty& value) {
		out << (unsigned char)value;
		return out;
	}
	inline std::ostream& operator<<(std::ostream& out, const GraWZombiaki::Position& value) {
		out << "(" << value.przecznica << "," << value.tor << ")";
		return out;
	}
}

#define BOOST_TEST_EQ_UINT8(a,b) BOOST_TEST(uint8_t(a) == uint8_t(b))
template <typename T, typename T1, typename T2>
void boost_test_eq(T1 a, T2 b)
{
	BOOST_TEST(static_cast<T>(a) == static_cast<T>(b));
}

inline bool findMove(MoveList_t& mvs, uint16_t mv)
{
	auto it = std::find(mvs.begin(), mvs.end(), mv);
	if (it != mvs.end()) {
		mvs.erase(it);
		return true;
	}
	return false;
}
