#include "pch.h"
#include "GameRules.h"
#include "object_pool.h"
#include "object_pool_multisize.h"
#include "random_generator.h"
#include <algorithm>
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS  
#include <intrin.h> 
#include <sstream>
#include <iostream>
#include <string_view>

using std::vector;
using std::string;

struct GameState
{
	//BIT order: b0..b7=A1..H1 b8..b16=A2..H2 i.e. 1st byte is line 1, 2nd byte line 2, LSB is A MSB is H
	uint64_t white;
	uint64_t black;
	uint64_t current_player : 1;
	uint64_t is_terminal	: 1;

	static bool calcIfTerminal(uint64_t board)
	{
		if (__popcnt64(board) == 1) return true;
		unsigned long idx;
		uint64_t tmp = board;
		while(_BitScanForward64(&idx, tmp))
		{
			if (0 == (board & getNeighbourMask(idx))) return false;
			tmp &= ~(1 << idx);
		}
		return true;
	}
	static uint64_t getNeighbourMask(long position)
	{
		uint64_t mask = 0x070507ull;	//this is mask for position [1,1]=9
		int shift = (int)position - 9;
		if (shift > 0)
		{
			mask <<= shift;
			if (shift % 8 == 6) {
				mask &= ~0x0101010101010101ull;	//mask out right edge
			}else if (shift % 8 == 7) {
				mask &= ~0x8080808080808080ull;	//mask out left edge
			}
		}else {
			shift = -shift;
			mask >>= shift;
			if (shift % 8 == 1) {
				mask &= ~0x8080808080808080ull;	//mask out left edge
			}else if (2==shift) {
				mask &= ~0x0101010101010101ull;	//mask out right edge
			}
		}
		return mask;
	}
	static uint64_t getRowMask(int position)
	{
		const auto shift = position / 8 * 8;
		return 0xFFull << shift;
	}
	static uint64_t getColumnMask(int position)
	{
		const auto shift = position % 8;
		return 0x0101010101010101ull << shift;
		//or return 0x8080808080808080ull >> shift;	use if MSB is A
	}
	static uint64_t getLeftDiagonalMask(int position)
	{
		/*..
		 .*.
		 ..*/
		const uint64_t mask = 0x8040201008040201ull;
		uint64_t m;
		auto shift = position / 8 - position % 8;
		if (shift > 0) {
			m = mask >> shift;
			m &= mask << 8 * shift;
		}
		else if (shift < 0) {
			shift = -shift;
			m = mask << shift;
			m &= mask >> 8 * shift;
		}else {
			m = mask;
		}
		return m;
	}
	static uint64_t getRightDiagonalMask(int position)
	{
		// ..*
		// .*.
		// *..
		const uint64_t mask = 0x0102040810204080ull;
		uint64_t m;
		auto shift = position % 8 - 7 + position / 8;
		if (shift > 0){
			m = mask << shift;
			m &= mask << 8 * shift;
		}
		else if (shift < 0){
			shift = -shift;
			m = mask >> shift;
			m &= mask >> 8 * shift;
		}
		else {
			m = mask;
		}
		return m;
	}
};
struct Move
{
};
struct MoveList
{
	uint64_t size;
	Move move[1];
	//next moves will follow
};
namespace LinesOfAction
{
	struct LinesOfActionGameRules : IGameRules
	{
		static const int NumPlayers = 2;
		int m_RefCnt;
		ObjectPoolBlocked<GameState, 512>  m_GameStatePool;
		
		LinesOfActionGameRules()
		{
			
		}
		~LinesOfActionGameRules() override
		{
			
		}
		GameState* allocGameState()
		{
			return m_GameStatePool.alloc();
		}
		void freeGameState(GameState* s)
		{
			m_GameStatePool.free(s);
		}
		void SetRandomGenerator(IRandomGenerator*) override
		{
		}
		GameState* CreateRandomInitialState(IRandomGenerator*) override
		{
			throw "not_implemented";
		}
		GameState* CreateInitialStateFromHash(const uint32_t*) override
		{
			throw "not implemented";
		}
		GameState* CreateStateFromString(const wstring&) override
		{
			throw "not implemented";
		}
		GameState* CreateStateFromString(const string&) override
		{
			throw "not implemented";
		}
		GameState* CreatePlayerKnownState(const GameState* gs, int playerNum) override
		{
			return CopyGameState(gs);
		}
		EvalFunction_t CreateEvalFunction(const string& name) override
		{
			throw "not implemented";
		}
		void UpdatePlayerKnownState(GameState* playerKnownState, const GameState* completeGameState,const std::vector<MoveList*>& playerMoves) override
		{
			*playerKnownState = *completeGameState;
		}
		GameState* CopyGameState(const GameState* s) override
		{
			auto* ns = allocGameState();
			*ns = *s;
			return ns;
		}
		bool AreEqual(const GameState *s1, const GameState *s2) override
		{
			return 0 == memcmp(s1, s2, sizeof(GameState));
		}
		void ReleaseGameState(GameState* s) override
		{
			freeGameState(s);
		}
		bool IsTerminal(const GameState* s) override
		{
			return s->is_terminal;
		}
		void Score(const GameState*, int score[]) override
		{
			throw "not implemented";
		}
		int GetCurrentPlayer(const GameState* s) override
		{
			return s->current_player;
		}
		MoveList* GetPlayerLegalMoves(const GameState*, int playerNum) override
		{
			throw "not implemented";
		}
		void ReleaseMoveList(MoveList*) override
		{
			throw "not implemented";
		}
		int GetNumMoves(const MoveList*) override
		{
			throw "not implemented";
		}
		std::tuple<Move*, float> GetMoveFromList(MoveList*, int idx) override
		{
			throw "not implemented";
		}
		MoveList* SelectMoveFromList(const MoveList*, int idx) override
		{
			throw "not implemented";
		}
		GameState* ApplyMove(const GameState*, Move*, int player) override
		{
			throw "not implemented";
		}
		GameState* Next(const GameState* s, const std::vector<MoveList*>& moves) override
		{
			const int current_player = s->current_player;
			auto* cs = const_cast<GameState*>(s);
			for (int player = 0; player < NumPlayers; ++player)
			{
				auto ml = moves[player];
				auto* ns = ApplyMove(cs, &ml->move[0], player);
				freeGameState(cs);
				cs = ns;
			}
			cs->current_player = current_player + 1;	//current_player if one bit field, will round to 0
			return cs;
		}
		const uint32_t* GetStateHash(const GameState*) override
		{
			throw "not implemented";
		}
		size_t GetStateHashSize() override
		{
			throw "not implemented";
		}
		string ToString(const GameState*) override
		{
			throw "not implemented";
		}
		wstring ToWString(const GameState*) override
		{
			throw "not implemented";
		}
		string ToString(const Move*) override
		{
			throw "not implemented";
		}
		wstring ToWString(const Move*) override
		{
			throw "not implemented";
		}
		void Release() override
		{
			if (--m_RefCnt == 0) {
				delete this;
			}
		}
		void AddRef() override
		{
			++m_RefCnt;
		}
	};
}
IGameRules* createLinesOfActionGameRules()
{
	return new LinesOfAction::LinesOfActionGameRules();
}
BOOST_DLL_ALIAS(
	createLinesOfActionGameRules,	// <-- this function is exported with...
	createGameRules					// <-- ...this alias name
)