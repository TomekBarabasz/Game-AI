#pragma once
#include <vector>
#include <string>
#include <boost/dynamic_bitset.hpp>
#include <bitset>

using std::wstring;
using std::string;

struct IGameState;
struct IMove
{
	virtual wstring toString() const = 0;
	virtual void release() = 0;
protected:
	virtual ~IMove() {}
};
using MoveList = std::vector<IMove*>;

//using GameStateHash_t = boost::dynamic_bitset<std::uint64_t>;
using GameStateHash_t = std::bitset<24 * 5>;

struct IGameState
{
	virtual void		Initialize(unsigned seed) = 0;
	virtual wstring		ToString() const = 0;
	virtual void		AddRef() const = 0;
	virtual void		Release() const = 0;
	virtual bool		IsTerminal() const = 0;
	virtual void		Score(int[]) const = 0;
	virtual IGameState* Next(const MoveList & moves) const = 0;
	//apply assumes all other players submitted Noop
	//and will increment player number
	virtual IGameState* Apply(const IMove*, int playerNum) const = 0;
	virtual MoveList	GetPlayerLegalMoves(int player) const = 0;
	virtual GameStateHash_t	Hash() const = 0;
	virtual string		HashS() const = 0;
	virtual int			NumPlayers() const = 0;
	virtual int			CurrentPlayer() const = 0;

protected:
	virtual ~IGameState() {}
};
