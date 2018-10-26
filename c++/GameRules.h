#pragma once
#include <vector>
#include <string>

using std::wstring;
using std::string;

struct IRandomGenerator;
struct GameState;
struct Move;
struct MoveList;
struct IGameRules
{
	virtual void SetRandomGenerator(IRandomGenerator*) = 0;
	virtual GameState* CreateRandomInitialState(IRandomGenerator*) = 0;
	virtual GameState* CreateInitialStateFromHash(const uint32_t*) = 0;
	virtual void ReleaseGameState(GameState*) = 0;
	virtual void ReleaseMoveList(const MoveList*) = 0;
	virtual int	 GetNumMoves(const MoveList*) = 0;
	virtual const Move*GetMoveFromList(const MoveList*,int idx) = 0;
	virtual bool IsTerminal(const GameState*) = 0;
	virtual void Score(const GameState*, int score[]) = 0;
	virtual const MoveList* GetPlayerLegalMoves(const GameState*, int playerNum) = 0;
	//apply assumes all other players submitted Noop
	//and will increment player number
	virtual GameState* ApplyMove(const GameState*, const Move*, int player) = 0;
	virtual GameState* Next(const GameState*, const std::vector<Move*>& moves) = 0;
	virtual const uint32_t* GetStateHash(const GameState*) = 0;
	virtual size_t GetStateHashSize() = 0;
	virtual string ToString(const GameState*) = 0;
	virtual wstring ToWString(const GameState*) = 0;
	virtual void Release() = 0;

protected:
	virtual ~IGameRules(){}
};

IGameRules* createGraWPanaGameRules(int NumPlayers);
