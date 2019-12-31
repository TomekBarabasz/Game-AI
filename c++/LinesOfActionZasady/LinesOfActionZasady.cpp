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
	mutable uint64_t is_terminal	: 2;

	GameState() : is_terminal(0) {}
	GameState(uint64_t wh, uint64_t bl, int cp)
		: white(wh), black(bl), current_player(cp), is_terminal(0)
	{}
	static bool calcIfTerminal(uint64_t board)
	{
		if (__popcnt64(board) == 1) return true;
		uint8_t boardGroupArr[64];
		const int groups_cnt = calcNumGroups(board, boardGroupArr);
		return groups_cnt == 1;
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
	static void addMoveIfValid(uint64_t other_on_line, int pos, int row, int col, uint64_t pieces, MoveList& ml)
	{
		if (row >=0 && row <=7 && col >=0 && col <=7)
		{
			const int newPos = col * 8 + row;
			const uint64_t newPosMask = 1ull << newPos;
			if (0 == (pieces & newPosMask))
			{
				//there must be no other pieces between starting and ending position on the board
				//other piece on ending position is allowed - it will be captured (and removed from board)
				other_on_line &= ~newPosMask;
				const uint64_t mvMask = newPos > pos ? ((1ull << (newPos - pos)) - 1) << pos : ((1ull << (pos - newPos)) - 1) << newPos;
				if (0 == (other_on_line & mvMask)) {
					auto& mv = ml.move[ml.size++];
					mv.from = pos;
					mv.to = newPos;
				}
			}
		}
	}
	bool isTerminal() const
	{
		/*if (is_terminal != 0) {
			const bool bIsTerminal = is_terminal - 1 > 0;
			const bool ist = calcIfTerminal(white) || calcIfTerminal(black);
			assert(bIsTerminal == ist);
			return is_terminal - 1;
		}*/
		const bool ist = calcIfTerminal(white) || calcIfTerminal(black);
		is_terminal = 1 + (ist ? 1 : 0);
		return ist;
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
			auto line_mask = getRowMask(position);
			auto line = all & line_mask ;
			int cnt = static_cast<int>(__popcnt64(line));
			//left
			addMoveIfValid(other & line_mask, position, col - cnt, row, my, ml);			
			//right
			addMoveIfValid(other & line_mask, position, col + cnt, row, my, ml);

			//column
			line_mask = getColumnMask(position);
			line = all & line_mask;
			cnt = static_cast<int>(__popcnt64(line));
			//up
			addMoveIfValid(other & line_mask, position, col, row + cnt, my, ml);
			//down
			addMoveIfValid(other & line_mask, position, col, row - cnt, my, ml);

			//left diagonal
			line_mask = getLeftDiagonalMask(position);
			line = all & line_mask;
			cnt = static_cast<int>(__popcnt64(line));
			//up-left
			addMoveIfValid(other & line_mask, position, col - cnt, row - cnt, my, ml);
			//down-right
			addMoveIfValid(other & line_mask, position, col + cnt, row + cnt, my, ml);

			//right diagonal
			line_mask = getRightDiagonalMask(position);
			line = all & line_mask;
			cnt = static_cast<int>(__popcnt64(line));
			//up-right
			addMoveIfValid(other & line_mask, position, col + cnt, row - cnt, my, ml);
			//down-left
			addMoveIfValid(other & line_mask, position, col - cnt, row + cnt, my, ml);
			
			tmp &= ~curPosMask;
			assert(ml.size <= 12 * 8);
		}
		for (int i=0;i<ml.size;++i)	{
			assert(ml.move[i].from != ml.move[i].to);
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
	static int calcNumGroups(uint64_t boardMask, uint8_t *boardGroupArr)
	{
		uint64_t tmp = boardMask;

		memset(boardGroupArr, 0, 64 * sizeof(uint8_t));
		uint8_t maxGroup = 0;
		while(tmp)
		{
			unsigned long pos;
			_BitScanForward64(&pos, tmp);
			uint64_t currentPosMask = 1ull << pos;
			
			uint64_t groupMask = currentPosMask;
			uint64_t groupMaskAlreadyChecked = 0;
			uint8_t currGroup = boardGroupArr[pos];
			if (0 == currGroup) {
				currGroup = ++maxGroup;
			}
			uint64_t groupMaskToCheck = groupMask & ~groupMaskAlreadyChecked;
			while (groupMaskToCheck)
			{
				_BitScanForward64(&pos, groupMaskToCheck);
				currentPosMask = 1ull << pos;				
				if (boardGroupArr[pos] != currGroup) {
					assert(boardGroupArr[pos] == 0);
					boardGroupArr[pos] = currGroup;
					groupMask |= getNeighbourMask(pos) & boardMask;
				}
				tmp &= ~currentPosMask;
				groupMaskAlreadyChecked |= currentPosMask;
				groupMaskToCheck = groupMask & ~groupMaskAlreadyChecked;
			}
		}
		return maxGroup;
	}
	/*
	static int calcSumOfGapsBetweenGroups(uint64_t board)
	{
		unsigned long max_gap = 0;
		unsigned long prev_r, prev_c;
		bool first = true;
		while(board)
		{
			unsigned long pos;
			_BitScanForward64(&pos, board);
			if (!first)
			{
				const auto gap_r = pos % 8 - prev_pos % 8;
				const auto gap_c = pos / 8 - pos / 8;
				const auto gap = __min(gap_r, gap_c);
				max_gap = __max(gap, max_gap);
				first = false;
				prev_r = pos
			}
			prev_pos = pos;
		}
		return max_gap;
	}*/
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
			auto proc_connected = [](const GameState* gs, int value[])
			{
				uint8_t boardGroupArr[64];
				auto numWhiteGroups = GameState::calcNumGroups(gs->white, boardGroupArr);
				auto numBlackGroups = GameState::calcNumGroups(gs->black, boardGroupArr);
				value[0] = __max(0, 100 - 20 * (numWhiteGroups - 1));
				value[1] = __max(0, 100 - 20 * (numBlackGroups - 1));
			};
			auto dummy = [](const GameState* gs, int value[])
			{
				value[0] = value[1] = 50;
			};
			return dummy;
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
			return s->isTerminal();
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
			if (moves == &m_noop) return const_cast<MoveList*>(moves);
			auto* ml = allocMoveList(1);
			ml->move[0] = moves->move[idx];
			ml->size = 1;
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
			ns->current_player += 1;	//current_player if one bit field, will round to 0
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