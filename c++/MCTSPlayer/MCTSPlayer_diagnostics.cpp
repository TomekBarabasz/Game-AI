#include "pch.h"
#include <string>
#include <boost/xpressive/xpressive.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <codecvt>
#include <iostream>
#include <fstream>
#include <GameRules.h>
#include "MCTSPlayer.h"

using namespace boost::xpressive;
using std::wstring;
using std::vector;
using std::map;

namespace MCTS
{
	void Player::dumpTreeWithPath(const string& filename, StateNode* root, const Path_t& path)
	{
		std::set<StateNode*> path_nodes;
		for (auto[state_node, move_node] : path) {
			path_nodes.insert(state_node);
		}
		dumpTree(filename, root, path_nodes);
	}

	void Player::dumpTree(const string & filename, StateNode* root, std::set<StateNode*> nodesToHighlight)
	{
		//dot.exe -Tsvg mcts_tree_dump.gv -o mcts_tree_dump.svg -Goverlap=prism
		std::wofstream out(filename);
		std::locale loc(std::locale::classic(), new std::codecvt_utf8<wchar_t>);
		out.imbue(loc);
		out << L"digraph g {" << std::endl;
		dumpTreeNode(out, root, ++m_curr_visit_id, nodesToHighlight);
		out << L"}" << std::endl;
	}

	void Player::dumpNodeDescription(std::wofstream& out, StateNode* sn, bool highlight)
	{
		wstring name = m_game_rules->ToWString(sn->state);
		boost::replace_all(name, L"|", L"\\n");
		const int current_player = m_game_rules->GetCurrentPlayer(sn->state);

		out << L"\"" << sn << L"\" [label=\"" << sn->state << " \\n" << name << "\" ";
		out << L"num_visited=\"" << sn->numVisited << "\" ";
		out << L"CP=\"" << current_player << "\" ";
		out << L"occured=\"" << (int)sn->occured << "\" ";
		out << L"temp=\"" << (int)sn->temporary << "\" ";
		out << L"term=\"" << (int)sn->terminal << "\"";
		if (highlight) {
			out << L" color = blue ";
		}
		if (sn->terminal) {
			int score[4];
			m_game_rules->Score(sn->state, score);
			out << L"score =\"" << score[0] << "," << score[1] << "\"";
		}
		out << "]" << std::endl;
	}

	bool Player::dumpMoveDescription(std::wofstream& out, StateNode* sn, int dummyNodeId, const MoveNode& mv)
	{
		Move* mvmv = m_game_rules->GetMoveFromList(sn->moveList, mv.moveIdx);
		const wstring mv_name = m_game_rules->ToWString(mvmv);
		bool needIncrement = false;
		if (mv.next)
		{
			if (mv.next->occupied) {
				out << L"\"" << sn << L"\" -> \"" << mv.next << L"\" [label = \"" << mv_name << L" nv = " << mv.numVisited;
				out << L"\\nval = " << mv.value[0] / mv.numVisited;
				for (int i = 1; i < m_cfg.NumberOfPlayers; ++i) {
					out << L"," << mv.value[i] / mv.numVisited;
				}
				out << L"\" mvidx=\"" << mv.moveIdx << "\"]" << std::endl;
			}
			else
			{
				out << L"\"" << sn << L"\" -> \"" << mv.next << L"\" [label = \"" << mv_name << L" corrupted\"";
				out << L" mvidx=\"" << mv.moveIdx << "\" color=red]" << std::endl;
				out << L"\"" << mv.next << L"\" [label=\"corrupted\" color=red]" << std::endl;
				needIncrement = false;
			}
		}
		else
		{
			out << L"\"" << sn << L"\" -> \"" << dummyNodeId << L"\" [label = \"" << mv_name;
			out << L"\" mvidx=\"" << mv.moveIdx << "\"]" << std::endl;
			needIncrement = true;
		}
		return needIncrement;
	}

	void Player::dumpTreeNode(std::wofstream& out, StateNode* sn, unsigned visit_id, std::set<StateNode*>& nodesToHighlight)
	{
		if (visit_id == sn->lastVisitId) return;
		if (sn->occupied != 1)
		{
			out << L"\"" << sn << "\" [label=\"corrupted node\" color=red]" << std::endl;
			return;
		}
		sn->lastVisitId = visit_id;
		auto it = nodesToHighlight.find(sn);
		const bool highlight = it != nodesToHighlight.end();
		dumpNodeDescription(out, sn, highlight);
		if (highlight) nodesToHighlight.erase(it);

		static int dummyNodeId = 1;
		for (int move_idx = 0; move_idx < sn->numMoves; ++move_idx)
		{
			MoveNode& mv = sn->moves[move_idx];
			if (dumpMoveDescription(out, sn, dummyNodeId, mv)) {
				++dummyNodeId;
			}
		}
		for (int move_idx = 0; move_idx < sn->numMoves; ++move_idx)
		{
			MoveNode& mv = sn->moves[move_idx];
			if (mv.next) {
				dumpTreeNode(out, mv.next, visit_id, nodesToHighlight);
			}
		}
	}

	std::tuple<StateNode*, Path_t> Player::loadTreeFromFile(const char* filename)
	{
		std::wifstream input(filename);
		std::locale loc(std::locale::classic(), new std::codecvt_utf8<wchar_t>);
		input.imbue(loc);
		return loadTree(input);
	}

	std::tuple<StateNode*, Path_t> Player::loadTree(std::wifstream& input)
	{
		wstring line;
		getline(input, line);
		if (line != L"digraph g {") throw "invalid_file_format";
		//"000000000080CE10" [label="000000000071ACC8 \nS=9♥10♥10♠W♥W♠A♦\nP0=9♠10♣\nP1=9♣9♦10♦W♣W♦D♥D♠D♣D♦K♥K♠K♣K♦A♥A♠A♣\nCP=1" num_visited="8" CP="1" used="1" refCnt="1" ]
		wsregex state_regex = '"' >> (s1 = +_) >> L"\"[label=\"" >> (s2 = -+_) >> L"\\n" >> (s3 = +_) >> L"\"num_visited=\"" >> (s4 = +_d) >> L"\"CP=\"" >> (s5 = +_d) >> "\"occured=\"" >> (s6 = +_d) >> "\"temp=\"" >> (s7 = +_d) >> "\"" >> L"term=\"" >> (s8=+_d) >> "\"" >> (s9=*_) >> ']';

		//"000000000080D5F0" -> "000000000080D350" [label = "play A♥ nv = 1\nval = 0.26,0.28" moveidx="1"]
		wsregex move_regex      = '"' >> (s1 = +_) >> L"\"->\"" >> (s2 = +_) >> L"\"[label=\"" >> (s3=+_) >> L"nv=" >> (s4 = +_d) >> L"\\nval=" >> (s5=+(_d|'.')) >> "," >> (s6=+(_d|'.')) >> "\"mvidx=\"" >> (s7=+_d) >> "\"" >> (s8=*_) >> ']';

		StateNode *root = nullptr;
		map<unsigned long long, StateNode*> sid2state;
		map<unsigned long long ,vector<MoveNode>> sid2move;
		wsmatch what;
		Path_t path;

		//(1) parse states and edges
		while (getline(input, line))
		{
			if (line.empty()) continue;
			boost::erase_all(line, " ");
			boost::erase_all(line, "\t");
			if (line == L"}") break;
			if (regex_match(line, what, state_regex))
			{
				const auto id = std::stoull( what[1], nullptr, 16 );
				wstring  state_str = what[3];
				boost::replace_all(state_str, L"\\n", L"|");
				const int current_player = std::stoi(what[5]);

				GameState *s = m_game_rules->CreateStateFromString(state_str);
				StateNode *sn = makeTreeNode(s);
				assert(sn->currentPlayer == current_player);
				sn->numVisited = std::stoi(what[4]);
				sn->occured = std::stoi(what[6]);
				sn->temporary = std::stoi(what[7]);
				sn->terminal = std::stoi(what[8]);
				sid2state[id] = sn;
				if (!root) root = sn;
				if (what[9] == L"color=blue") { path.push_back( {sn,nullptr} ); }
			}
			else if (regex_match(line, what, move_regex))
			{
				const auto sid = std::stoull(what[1], nullptr, 16);
				const auto target_sid = std::stoull(what[2], nullptr, 16);
				const wstring name = what[3];
				const int num_visited = std::stoi(what[4]);
				const float val1= std::stof(what[5]) * num_visited;
				const float val2= std::stof(what[6]) * num_visited;
				const int mvIdx = std::stoi(what[7]);

				StateNode& sn = *sid2state[sid];
				auto name2 = m_game_rules->ToWString(m_game_rules->GetMoveFromList(sn.moveList, mvIdx));
				boost::erase_all(name2, " ");
				assert(name == name2);
				auto it = sid2move.find(sid);
				if (it == sid2move.end()) {
					it = sid2move.insert( sid2move.begin(), {sid, {}} );
				}
				auto &mv = it->second;
				MoveNode mn { reinterpret_cast<StateNode*>(target_sid), (unsigned char)num_visited, (unsigned char)mvIdx, {val1,val2,0,0} };
				mv.push_back( mn );
			}
		}
		//(2) combine states and edges
		for (auto [sid, sn] : sid2state)
		{
			auto & moves = sid2move[sid];
			for (auto & mv : moves)	{
				mv.next = sid2state[reinterpret_cast<unsigned long long>(mv.next)];
				sn->moves[mv.moveIdx] = mv;
			}
		}

		//(3) initialize path edges
		if (!path.empty()) {
			for (int idx = 0; idx < path.size() - 1; ++idx)
			{
				auto *n = path[idx].first;
				auto *next = path[idx + 1].first;
				for (int mi = 0; mi < n->numMoves; ++mi) {
					if (n->moves[mi].next == next) {
						path[idx].second = n->moves + mi;
						break;
					}
				}
			}
		}

		return { root,path };
	}

	void Player::traceSelectMove(const wstring& state, const std::multimap<double, MoveNode*>& moves, const wstring& selected, StateNode* sn)
	{
		/*
		std::wstringstream ss;
		ss << state << L" moves : ";
		for (auto & it : moves)
		{
			Move* mv = m_game_rules->GetMoveFromList(sn->moveList, it.second->moveIdx);
			ss << m_game_rules->ToWString(mv) << L" : " << it.first << L" | ";
		}
		ss << L"selected " << selected << std::endl;
		TRACE(m_trace,ss.str().c_str());
		*/
	}
	void Player::_dumpMoveTree()
	{
		if (!m_cfg.traceMoveFilename.empty()) {
			dumpTree(makeTreeFilename(m_cfg.traceMoveFilename.c_str()), m_root);
		}
	}

	void Player::_dumpGameTree()
	{
		if (!m_cfg.gameTreeFilename.empty()) {
			dumpTree(makeTreeFilename(m_cfg.gameTreeFilename.c_str()), m_super_root);
		}
	}

	bool Player::checkTree(StateNode* root, unsigned short id, bool dump)
	{
		bool error = false;
		visitTree(root, id, [&](StateNode* sn) {
			if (!sn->occupied) {
				error = true;
				if (!dump) assert(sn->occupied);
				return false;
			}
			return true;
		});

		if (error && dump) {
			dumpTree(makeTreeFilename("mcts_bad_tree"), root);
		}
		assert(!error);
		return error;
	}
}