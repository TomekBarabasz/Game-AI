#pragma once
#include <functional>
#include <boost/property_tree/ptree.hpp>
#include <Metrics.h>

struct MoveList;
struct GameState;
struct IGameRules;
using PlayerConfig_t = boost::property_tree::ptree;

enum class GameResult { Win = 0, Draw, Loose, AbortedByRoundLimit = 1, AbortedByStateLoop = 2 };
struct IGamePlayer
{
	virtual void			startNewGame	(GameState* playerKnownState) = 0;
	virtual void			endGame			(int score, GameResult result) = 0;
	virtual void			setGameRules	(IGameRules*) = 0;
	virtual MoveList*		selectMove		(GameState* playerKnownState) = 0;
	virtual NamedMetrics_t	getGameStats	() = 0;
	virtual std::string		getName			() = 0;
	virtual void			resetStats		() = 0;
	virtual void			release			() = 0;

protected:
	virtual ~IGamePlayer(){}
};
/* config tags:
 * player_type = random | lowcard | minmax | mcts
 * number_of_players = 2 | 3 | 4
 */
using CreatePlayer_t = std::function<IGamePlayer*(int player_number, const PlayerConfig_t&)>;
