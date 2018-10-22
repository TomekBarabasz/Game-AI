#pragma once
#include <unordered_map>
#include <string>

struct IMove;
struct IGameState;
using PlayerStats_t = std::unordered_map<std::string, long>;
struct IGamePlayer
{
	virtual IMove*	selectMove(const IGameState*gs, PlayerStats_t* ps=nullptr) = 0;
	virtual void	getGameStats(PlayerStats_t& ps) = 0;
	virtual void	release() = 0;

protected:
	virtual ~IGamePlayer(){}
};
