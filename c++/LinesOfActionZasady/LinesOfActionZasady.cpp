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

struct Move
{
	uint8_t from;
	uint8_t to;

	string toString() const 
	{
		std::ostringstream ss;
		const char  start_col = 'A' + from % 8;
		const auto start_row = from / 8 + 1;
		const char end_col = 'A' + to % 8;
		const auto end_row = to / 8 + 1;
		ss << start_col << start_row << "->" << end_col << end_row;
		return ss.str();
	}
};
struct MoveList
{
	uint16_t size;
	Move move[1];
	//next moves will follow
};
struct GameState
{
	static const int Whites = 0;
	static const int Blacks = 1;
	
	//BIT order: b0..b7=A1..H1 b8..b16=A2..H2 i.e. 1st byte is line 1, 2nd byte line 2, LSB is A MSB is H
	uint64_t white;
	uint64_t black;
	uint64_t current_player : 1;
	uint64_t is_terminal	: 1;

	GameState() {}
	GameState(uint64_t wh, uint64_t bl, int cp)
		: white(wh), black(bl), current_player(cp), is_terminal(calcIfTerminal(white) || calcIfTerminal(black))
	{}
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
	static void addMoveIfValid(int pos, int row, int col, uint64_t pieces, MoveList& ml)
	{
		if (row >=0 && row <=7 && col >=0 && col <=7)
		{
			const int newPos = col * 8 + row;
			const uint64_t newPosMask = 1ull << newPos;
			if (0 == (pieces & newPosMask))
			{
				auto& mv = ml.move[ml.size++];
				mv.from = pos;
				mv.to = newPos;
			}
		}
	}
	int getPlayerLegalMoves(MoveList& ml) const
	{
		uint64_t my,tmp,other;
		if (Whites == current_player) {
			tmp=my = white;
			other = black;
		}
		else {
			tmp=my = black;
			other = white;
		}
		uint64_t all = my | other;
		unsigned long pos;
		while(tmp)
		{
			//go through all bits(pieces) of my color
			_BitScanForward64(&pos, tmp);
			const uint64_t curPosMask = 1ull << pos;
			
			const int position = static_cast<int>(pos);
			const int col = position % 8;
			const int row = position / 8;
			//row
			auto line = all & getRowMask(position) ;
			int cnt = static_cast<int>(__popcnt64(line));
			
			//left
			addMoveIfValid(position, col - cnt, row, my, ml);			
			//right
			addMoveIfValid(position, col + cnt, row, my, ml);

			//column
			line = all & getColumnMask(position);
			cnt = static_cast<int>(__popcnt64(line));
			//up
			addMoveIfValid(position, col, row + cnt, my, ml);
			//down
			addMoveIfValid(position, col, row - cnt, my, ml);
			
			//left diagonal
			line = all & getLeftDiagonalMask(position);
			cnt = static_cast<int>(__popcnt64(line));
			//up-left
			addMoveIfValid(position, col - cnt, row + cnt, my, ml);
			//down-right
			addMoveIfValid(position, col + cnt, row - cnt, my, ml);
			
			//right diagonal
			line = all & getRightDiagonalMask(position);
			cnt = static_cast<int>(__popcnt64(line));
			//up-right
			addMoveIfValid(position, col + cnt, row + cnt, my, ml);
			//down-left
			addMoveIfValid(position, col - cnt, row - cnt, my, ml);
			
			tmp &= ~curPosMask;
		}
		return ml.size;
	}
	void applyMove(Move& mv, int playerNum)
	{
		const uint64_t fromMask = 1ull << mv.from;
		const uint64_t toMask   = 1ull << mv.to;
		uint64_t *my, *other;

		if (Whites == playerNum) {
			my = &white;
			other = &black;
		}
		else {
			my = &black ;
			other = &white;
		}
		assert(*my & fromMask);
		assert(0 == (*my & toMask));
		*my &= ~fromMask;
		*my |= toMask;
		*other &= ~toMask;
	}
	string toString() const
	{
		std::ostringstream ss;
		uint64_t mask = 0x8000000000000000ull;
		int i = 0;
		while(mask)
		{
			char c = white & mask ? 'o' : black & mask ? '*' : '.';
			ss << c;
			if (++i == 8) {
				ss << std::endl;
				i = 0;
			}
			mask >>= 1;
		}
		ss << "cp=" << current_player;
		return ss.str();
	}
};

namespace LinesOfAction
{
	struct LinesOfActionGameRules : IGameRules
	{
		static const int NumPlayers = 2;
		int m_RefCnt;
		MoveList m_noop = {1,{0,0}};
		ObjectPoolBlocked<GameState, 24>			m_GameStatePool;
		ObjectPoolMultisize<8 * sizeof(Move), 4096> m_moveListPool;
		
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
			auto* gs = allocGameState();
			gs->current_player = GameState::Blacks;
			gs->black = 0x7e0000000000007e;
			gs->white = 0x0081818181818100;
			return gs;
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
		void Score(const GameState *gs, int score[]) override
		{
			const bool whiteWins = GameState::calcIfTerminal(gs->white);
			const bool blackWins = GameState::calcIfTerminal(gs->black);
			if (whiteWins && blackWins)
			{
				score[0] = score[1] = 50;
			}
			else {
				score[0] = whiteWins ? 100 : 0;
				score[1] = blackWins ? 100 : 0;
			}
		}
		int GetCurrentPlayer(const GameState* s) override
		{
			return s->current_player;
		}
		MoveList* allocMoveList(int size)
		{
			const size_t num_chunks = size * sizeof(Move) / m_moveListPool.ChunkSize + 1;
			return m_moveListPool.alloc<MoveList>(num_chunks);
		}
		MoveList* copyMoveList(MoveList* moves)
		{
			auto* ml = allocMoveList(moves->size);
			memcpy(ml, moves, sizeof(uint16_t) * (moves->size + 1));
			return ml;
		}
		void ReleaseMoveList(MoveList* moves) override
		{
			if (moves != &m_noop) {
				const size_t num_chunks = moves->size * sizeof(Move) / m_moveListPool.ChunkSize + 1;
				m_moveListPool.free(moves, num_chunks);
			}
		}
		MoveList* SelectMoveFromList(const MoveList* moves, int idx) override
		{
			auto* ml = allocMoveList(1);
			assert(ml->size == 1);
			ml->move[0] = moves->move[idx];
			return ml;
		}
		MoveList* GetPlayerLegalMoves(const GameState* gs, int playerNum) override
		{
			if (playerNum != gs->current_player) return &m_noop;
			uint16_t tmp[12 * 8 + 1];
			auto move_list = reinterpret_cast<MoveList*>(tmp);
			move_list->size = 0;
			gs->getPlayerLegalMoves(*move_list);
			return copyMoveList(move_list);
		}
		int	 GetNumMoves(const MoveList* ml) override
		{
			return 	ml->size;
		}
		std::tuple<Move*, float> GetMoveFromList(MoveList* ml, int idx) override
		{
			auto* move = &ml->move[idx];
			return { move,1.0f };
		}
		GameState* ApplyMove(const GameState *gs, Move* mv, int player) override
		{
			auto* ns = CopyGameState(gs);
			if (mv != m_noop.move) {
				ns->applyMove(*mv, player);
			}
			return ns;
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
		const uint32_t* GetStateHash(const GameState* gs) override
		{
			return reinterpret_cast<const uint32_t*>(gs);
		}
		size_t GetStateHashSize() override
		{
			return sizeof(GameState) / 4;
		}
		string ToString(const GameState* gs) override
		{
			return gs->toString();
		}
		wstring ToWString(const GameState* gs) override
		{
			auto str = ToString(gs);
			wstring wstr(str.begin(), str.end());
			return wstr;
		}
		string ToString(const Move* mv) override
		{
			return mv->toString();
		}
		wstring ToWString(const Move* mv) override
		{
			auto str = ToString(mv);
			wstring wstr(str.begin(), str.end());
			return wstr;
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