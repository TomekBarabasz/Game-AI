#pragma once
#include <vector>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include "Metrics.h"

struct IGamePlayer;
struct IGameRules;
struct IRandomGenerator;

using std::string;
using Result_t = boost::property_tree::ptree;
using GameConfig_t = boost::property_tree::ptree;
using RunFromXml_t = std::function < Result_t(const char* filename)>;
using RunFromConfig_t = std::function<Result_t(const GameConfig_t&)>;
using RunSimple_t = std::function<Result_t (const std::vector<IGamePlayer*>, IGameRules*, int numGames)>;
using InternalResults_t = std::unordered_map<string, Metric_t>;

enum class SingleGameResult
{
	Win = 0,
	RoundLimit = 1,
	StateLoop = 2
};

struct IProgressBar
{
	virtual void set(int) = 0;
	virtual void update(int) = 0;
	virtual void release() = 0;
protected:
	virtual ~IProgressBar() {}
};

IProgressBar* createProgressBar(bool enable, int limit, int type);
IRandomGenerator* makeRng();
const char* getPlayerName(int);
void saveXmlResults(const Result_t& results, const GameConfig_t& cfg);
void appendPlayerStats(InternalResults_t& results, const std::vector<IGamePlayer*>& players);
string singleRunResultString(SingleGameResult result);
void mergeResults(InternalResults_t& first, const InternalResults_t& second);
Result_t convertInternalResults(const InternalResults_t& internal_results);
void addPostRunResults(InternalResults_t& results, const std::chrono::steady_clock::time_point& t0, size_t number_of_players);
