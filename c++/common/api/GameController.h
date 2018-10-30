#pragma once
#include <vector>
#include <boost/property_tree/ptree.hpp>

struct IGamePlayer;
struct IGameRules;
using Result_t = boost::property_tree::ptree;
using GameConfig_t = boost::property_tree::ptree;

using RunFromXml_t = std::function < Result_t(const char* filename)>;
using RunFromConfig_t = std::function<Result_t(const GameConfig_t&)>;
using RunSimple_t = std::function<Result_t (const std::vector<IGamePlayer*>, IGameRules*, int numGames)>;
