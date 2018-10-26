#include "stdafx.h"
#include "test_game_state.h"
#include <fstream>
#include <boost/xpressive/xpressive.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/erase.hpp>

using namespace boost::xpressive;

TestGameState Load(const char *filename)
{	
	map<wstring, size_t> stateNameToIdx;
	std::wifstream input(filename);
	wstring line;

	wsregex nump_regex = L"number_of_players=" >> (s1 = +_d);
	wsregex state_regex = L"statelabel=" >> (s1 = +~_n) >> L"player=" >> (s2 = +_d) >> *(L"score=" >> (s3 = +set[_d | ',']));
	wsregex move_regex = (s1 = +~_n) >> L"->" >> (s2 = +~_n) >> L"label=" >> (s3 = +~_n) >> L"player=" >> (s4 = +_d);

	wsmatch what;

	getline(input, line);
	boost::erase_all(line, " ");
	boost::erase_all(line, "\t");

	vector<wstring> tokens;
	struct Transition { wstring s0, s1, label, player; };
	vector<Transition> transitions;
	map<wstring,wstring> transitionNameToStates;
	const int number_of_players = true == regex_match(line, what, nump_regex) ? stoi(what[1]) : 0;
	if (0==number_of_players) {
		throw "invalid file format";
	}

	TestGameState tgs(number_of_players);

	while (getline(input,line))
	{
		if (line.empty()) continue;
		boost::erase_all(line, " ");
		boost::erase_all(line, "\t");
		if (regex_match(line, what, state_regex))
		{
			const wstring & name = what[1];
			const wstring & score = what[3];
			const bool isTerminal = !score.empty();

			tgs.m_tree.push_back({ name, stoi(what[2]), isTerminal });
			if (isTerminal)
			{
				tokens.clear();
				boost::split(tokens, score, boost::is_any_of(","));
				auto & s = tgs.m_tree.back();
				for (auto i = 0; i < tokens.size(); ++i) s.score[i] = stoi(tokens[i]);
			}
			if (stateNameToIdx.find(name) != stateNameToIdx.end()) {
				std::wstringstream ss;
				ss << L"repeated state label :" << name;
				throw ss.str();
			}
			stateNameToIdx[name] = tgs.m_tree.size()-1;
		}
		else if (regex_match(line, what, move_regex)) {
			const wstring &name = what[3];
			const wstring states = what[1] + what[2];
			if (name != L"noop") {
				if (transitionNameToStates.find(name) != transitionNameToStates.end()) {
					std::wstringstream ss;
					ss << L"repeated transition label :" << name;
					throw ss.str();
				}
				transitionNameToStates[name] = states;
			}
			transitions.push_back({what[1], what[2], what[3], what[4]});
		}else
		{
			std::wstringstream ss;
			ss << "invalid line:" << line;
			throw ss.str();
		}
	}
	for(auto & mv : transitions)
	{
		auto & s1 = tgs.m_tree[stateNameToIdx[mv.s0]];
		auto * s2 = &tgs.m_tree[stateNameToIdx[mv.s1]];
		const wstring & name = mv.label;
		const int player = stoi(mv.player);
		s1.playerMoves[player].push_back({ name, s2 });
	}
	tgs.m_curState = &tgs.m_tree[0];
	return tgs;
}

