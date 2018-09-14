#pragma once
#include <vector>
#include <string>

using std::wstring;
struct IMove
{
	virtual wstring toString() const = 0;
	virtual void release() = 0;
protected:
	virtual ~IMove() {}
};
using MoveList = std::vector<IMove*>;

struct IGameState
{
	virtual void initialize(int numPlayers) = 0;
	virtual wstring toString() const = 0;
	virtual void release() const = 0;
	virtual bool isTerminal() const = 0;
	virtual void score(int []) const = 0;
	virtual IGameState* next(const MoveList & moves) const = 0;
	virtual MoveList getPlayerLegalMoves(int player) const = 0;

protected:
	virtual ~IGameState(){}
};
