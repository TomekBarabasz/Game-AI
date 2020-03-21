#pragma once
#include <vector>
using std::vector;

namespace GraWZombiaki
{
	struct Position
	{
		unsigned przecznica, tor;
		bool operator==(const Position& other) const {
			return przecznica == other.przecznica && tor == other.tor;
		}
	};
	using MoveList_t = vector<uint16_t>;
	enum Player : uint8_t { zombie = 0, human = 1 };
}