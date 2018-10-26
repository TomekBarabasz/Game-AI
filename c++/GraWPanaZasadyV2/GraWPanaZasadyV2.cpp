#include "pch.h"
#include "GameRules.h"
#include "object_pool.h"
#include "memory_mgmt.h"
#include "random_generator.h"
#include <algorithm>
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS  
#include <intrin.h> 
#include "boost/tuple/tuple.hpp"

#define TRACE(fmt,...)

using std::vector;

struct GameState
{
	constexpr static int MaxPlayersCount = 4;
	constexpr static int SizeInBytes = (MaxPlayersCount + 1) * sizeof(uint32_t);
	uint32_t stack : 24;
	uint32_t current_player : 3;
	uint32_t hand[MaxPlayersCount];

	void zero()
	{
		memset(this, 0, SizeInBytes);
	}
	GameState& operator=(const GameState& other)
	{
		memcpy(this, &other, SizeInBytes);
		return *this;
	}
};

struct Move
{
	enum { noop = 0, take_cards, play_cards };
	uint32_t cards : 24;
	uint32_t operation : 3;
};

struct MoveList
{
	uint32_t size;
	Move move[1];
	//next moves will follow
};

namespace GraWPanaV2
{
	struct GraWPanaGameRules : IGameRules
	{
		const int NumPlayers;
		const int GameStateHashSize;
		ObjectPoolBlocked<GameState,512> m_GameStatePool;
		MoveList m_noop;

		GraWPanaGameRules(int numPlayers) : NumPlayers(numPlayers), GameStateHashSize(5)
		{
			m_noop.size = 1;
			m_noop.move[0].operation = Move::noop;
		}
		~GraWPanaGameRules()
		{
			
		}
		GameState* allocGameState() {
			return m_GameStatePool.alloc();
		}
		void freeGameState(GameState* s) {
			m_GameStatePool.free(s);
		}
		void SetRandomGenerator(IRandomGenerator*) override {}
		GameState* CreateRandomInitialState(IRandomGenerator *rng) override
		{
			vector<int> cards(24);
			vector<int> shuffle = rng->generateUniform(0, 23, 40);
			std::generate(cards.begin(), cards.end(), [n = 0]() mutable { return n++; });
			for (int i=0;i<shuffle.size();)
			{
				int & t0 = cards[i++];
				int & t1 = cards[i++];
				int t = t0;
				t0 = t1;
				t1 = t;
			}
			GameState *s = allocGameState();
			s->zero();
			int current_player = 0;
			for (auto c : cards) {
				s->hand[current_player] |= 1 << c;
				current_player = (current_player + 1) % NumPlayers;
			}
			for (current_player = 0; current_player < NumPlayers; ++current_player)	{
				if (s->hand[current_player] & 1) break;
			}
			s->current_player = current_player;
			return s;
		}
		GameState* CreateInitialStateFromHash(const uint32_t* hash) override
		{
			GameState *s = allocGameState();
			auto *raw = reinterpret_cast<uint32_t*>(s);
			for (int i = 0; i < GameStateHashSize; ++i) raw[i] = hash[i];
			return s;
		}
		void ReleaseGameState(GameState* s) override
		{
			freeGameState(s);
		}
		bool IsTerminal(const GameState* s) override
		{
			int numPlayersWithCards = 0;
			for (auto i=0;i<NumPlayers;++i) {
				if (s->hand[i]) ++numPlayersWithCards;
			}
			return 1 == numPlayersWithCards;
		}
		void Score(const GameState* s, int score[]) override
		{
			if (IsTerminal(s))
			{
				const int winPts = 100 / (NumPlayers - 1);
				for (auto pn = 0; pn < NumPlayers; ++pn) {
					score[pn] = s->hand[pn] != 0 ? 0 : winPts;
				}
			}
			else
			{
				const int drawPts = 100 / NumPlayers;
				for (auto pn = 0; pn < NumPlayers; ++pn) {
					score[pn] = drawPts;
				}
			}
		}

		static auto makeStackMasks(uint32_t stack)
		{
			unsigned long idx;
			_BitScanReverse(&idx, stack);
			const uint32_t above_stack_mask = ~((1 << (idx + 1)) - 1) &0b111111111111111111111111;

			//const uint32_t first_allowed_quad = 0b1111 << ((idx / 4 + 1) * 4);
			//const uint32_t last_quad_on_stack = idx % 4 != 0 ? first_allowed_quad >> 4 : 0;
			//const uint32_t allowed_cards = above_stack_mask | (~stack & last_quad_on_stack);
			const uint32_t first_allowed_quad = 0b1111 << ((idx / 4) * 4);
			const uint32_t allowed_cards = above_stack_mask | (~stack & first_allowed_quad);

			uint32_t take_cards_mask = 0;
			int taken = 0;
			for(;;) {
				if (stack <= 1) break;
				const uint32_t tos = 1 << idx;
				take_cards_mask |= tos;
				stack &= ~tos;
				if (3 == ++taken) break;
				_BitScanReverse(&idx, stack);
			}
			return boost::tuples::make_tuple(allowed_cards, first_allowed_quad, take_cards_mask);
		}

		MoveList* GetPlayerLegalMoves(const GameState* s, int player) override
		{
			if (player != s->current_player) return &m_noop;
			vector<Move> moves;
			const uint32_t player_hand = s->hand[player];
			if (0 == s->stack)
			{
				if (player_hand & 1) moves.push_back( {1, Move::play_cards} );
				const uint32_t all_suites = 0b1111;
				if ((player_hand & all_suites) == all_suites) moves.push_back( {all_suites, Move::play_cards} );
			}
			else 
			{
				if (1 == s->stack)
				{ 
					const uint32_t thee_nines= 0b1110;
					if ((player_hand & thee_nines) == thee_nines) moves.push_back( {thee_nines, Move::play_cards} );
				}

				const auto masks = makeStackMasks(s->stack);	// top_of_stack, stack_mask, allowed_cards
				const uint32_t allowed_cards = masks.get<0>();
				const uint32_t first_allowed_quad = masks.get<1>();
				const uint32_t take_cards_mask = masks.get<2>();

				//stack is	   : 00000000000000000010101010110011
				//stack_mask   : 00000000000000000011111111111111
				//allowed_cards: 11111111111111111100000000000000
				//first_quad   : 00000000000011110000000000000000

				/* use this if all cards of the same color shall be enumerated
				uint32_t cards_to_play = player_hand & allowed_cards;
				while (cards_to_play)
				{
					const uint32_t lowest = cards_to_play & -cards_to_play;
					moves.push_back( {lowest, Move::play_cards} );
					cards_to_play &= ~lowest;
				}*/
				//use this if only the lowest card of the same color shall be enumerated (reduce branching factor by 3)
				//uint32_t quad = first_allowed_quad >> 4;
				//if (0 == (quad & allowed_cards)) quad = first_allowed_quad;
				uint32_t quad = first_allowed_quad;
				uint32_t player_hand_masked = player_hand & allowed_cards;
				while (player_hand_masked != 0)
				{
					const uint32_t allowed_cards_from_quad = quad & player_hand_masked;
					if (allowed_cards_from_quad) {
						const uint32_t lower = allowed_cards_from_quad & -(signed)allowed_cards_from_quad;
						moves.push_back({ lower, Move::play_cards });
					}
					if ((player_hand_masked & quad) == quad) {
						moves.push_back({ quad, Move::play_cards });
					}
					player_hand_masked &= ~quad;
					quad <<= 4;
				}
				if (0 != take_cards_mask) {
					moves.push_back({ take_cards_mask, Move::take_cards });
				}
			}
			return allocMoveList(moves);
		}
		MoveList* allocMoveList(const vector<Move>& moves)
		{
			const auto SizeOfMovesInBytes = moves.size() * sizeof(Move);
			MoveList *ml = reinterpret_cast<MoveList*>(new uint8_t[SizeOfMovesInBytes + sizeof(uint32_t)]);
			ml->size = (uint32_t) moves.size();
			memcpy(ml->move, moves.data(), SizeOfMovesInBytes);
			return ml;
		}
		void ReleaseMoveList(const MoveList* ml) override
		{
			if (ml != &m_noop) {
				delete[] ml;
			}
		}
		int	 GetNumMoves(const MoveList* ml) override
		{
			return 	ml->size;
		}
		const Move* GetMoveFromList(const MoveList* ml, int idx)
		{
			return &ml->move[idx];
		}
		//apply assumes all other players submitted Noop
		//and will increment player number
		GameState* ApplyMove(const GameState* s, const Move* m, int player) override
		{
			auto * ns = allocGameState();
			*ns = *s;
			switch (m->operation)
			{
			case Move::noop:
				break;
			case Move::take_cards:
				ns->stack &= ~m->cards;
				ns->hand[player] |= m->cards;
				break;
			case Move::play_cards:
				ns->stack |= m->cards;
				ns->hand[player] &= ~m->cards;
				break;
			}
			ns->current_player = s->current_player + 1;	//bit field will automatically do % NumPlayers
			return ns;
		}
		GameState* Next(const GameState* s, const std::vector<Move*>& moves) override
		{
			const int next_player = s->current_player + 1;
			auto *cs = const_cast<GameState*>(s);
			for (int player=0; player<NumPlayers;++player)
			{
				auto * ns = ApplyMove(s, moves[player], player);
				free(cs);
				cs = ns;
			}
			cs->current_player = next_player;
			return cs;
		}
		const uint32_t* GetStateHash(const GameState* s) override
		{
			return reinterpret_cast<const uint32_t*>(s);
		}
		size_t GetStateHashSize() override
		{
			return GameStateHashSize;
		}
		string ToString(const GameState*) override { return string(); }
		wstring ToWString(const GameState*) override { return wstring(); }
		void Release() override
		{
			delete this;
		}
	};
}
#ifndef UNIT_TEST
namespace MemoryMgmt
{
	using namespace GraWPanaV2;
	struct MemoryPools
	{
	};

	MemoryPools* makeMemoryPoolsInst()				 { return nullptr; }
	void		 freeMemoryPoolsInst(MemoryPools* i) { }

}
IGameRules* createGraWPanaGameRules(int NumPlayers)
{
	return new GraWPanaV2::GraWPanaGameRules(NumPlayers);
}
BOOST_DLL_ALIAS(
	createGraWPanaGameRules,		// <-- this function is exported with...
	createGameRules			// <-- ...this alias name
)
#endif