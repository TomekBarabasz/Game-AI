#pragma once

struct IMove;
struct IGameState;
struct IGamePlayer
{
	virtual IMove*	selectMove(const IGameState*) = 0;
	virtual void	release() = 0;

protected:
	virtual ~IGamePlayer(){}
};
