#pragma once
#include <cassert>
#include "GameRules.h"
#include "Common.h"

struct Move
{
	enum Operation : uint8_t { noop, discard_card, play_card, use_card, move_card, boss_order };
	union {
		struct {
			uint16_t op : 3;
			uint16_t params : 13;
		};
		uint16_t value;
	};
};

namespace GraWZombiaki
{
	template <typename MOVE, typename...Args>
	uint16_t makeMove(Args...args)
	{
		MOVE m(std::forward<Args>(args)...);
		return m.value;
	}

	struct Mv_BossOrder : Move
	{
		Mv_BossOrder(unsigned idx)
		{
			op = Operation::boss_order;
			params = idx;
		}
		unsigned getCardIdx() const { return params; }
	};

	struct Mv_DiscardCard : Move
	{
		Mv_DiscardCard(unsigned idx)
		{
			op = Operation::discard_card;
			params = idx;
		}
		unsigned getCardIdx() const { return params; }
	};

	struct Mv_PlayCard : Move
	{
		union Value {
			struct {
				uint16_t op : 3;
				uint16_t przecznica : 3;
				uint16_t tor : 2;
				uint16_t cardIdx : 8;
			};
			uint16_t value;
		};
		Mv_PlayCard(Position p, uint16_t cardIdx)
		{
			Value v;
			v.op = Operation::play_card;
			v.przecznica = p.przecznica;
			v.tor = p.tor;
			v.cardIdx = cardIdx;
			value = v.value;
		}
		std::tuple<Position, uint16_t> get() const
		{
			auto& val = *(Value*)&value;
			return { { val.przecznica, val.tor }, val.cardIdx };
		}
	};

	struct Mv_MoveCard : Move
	{
		union Value {
			struct {
				uint16_t op : 3;
				uint16_t from_przecznica : 3;
				uint16_t from_tor : 2;
				uint16_t to_przecznica : 3;
				uint16_t to_tor : 2;
			};
			uint16_t value;
		};
		Mv_MoveCard(Position from, Position to)
		{
			Value v;
			v.op = Operation::move_card;
			v.from_przecznica = from.przecznica;
			v.from_tor = from.tor;
			v.to_przecznica = to.przecznica;
			v.to_tor = to.tor;
			value = v.value;
		}
		std::tuple<Position, Position> get() const
		{
			auto& v = *(Value*)&value;
			return { {v.from_przecznica,v.from_tor},{v.to_przecznica, v.to_tor} };
		}
	};

	struct Mv_UseCard : Move
	{
		Mv_UseCard(unsigned idx)
		{
			op = Operation::use_card;
			params = idx;
		}
		unsigned getCardIdx() const { return params; }
	};

	struct Mv_Noop : Move
	{
		Mv_Noop()
		{
			op = Operation::noop;
		}
	};
}
