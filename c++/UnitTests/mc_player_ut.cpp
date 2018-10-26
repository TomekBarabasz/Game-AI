#include "stdafx.h"
#include "CppUnitTest.h"
#include "CppUnitTestAssert.h"
#include "CppUnitTestLogger.h"
#include "mem_check.h"

#define UNIT_TEST
#include "test_game_state.h"

#include "..\mcts_player.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

template <typename T>
T* AddRef(T* t) { t->AddRef(); return t; }

namespace Microsoft {
	namespace VisualStudio {
		namespace CppUnitTestFramework {
			template <> wstring ToString(Node* n) {return wstring(n->name.begin(), n->name.end());}
		}
	}
}
TestGameState State;

namespace UnitTests
{
	TEST_CLASS(MCTSPlayer_UT)
	{
		MemoryLeakChecker mlc;
		static void eval(const IGameState*, int value[]) { value[0] = value[1] = 50; }
	public:
		TEST_CLASS_INITIALIZE(classInit)
		{
			State = Load("C:\\MyData\\Projects\\gra_w_pana\\test_state_game.txt");
		}
		TEST_CLASS_CLEANUP(classCleanup)
		{
		}
		TEST_METHOD_INITIALIZE(init)
		{
			mlc.checkpoint();
		}
		TEST_METHOD_CLEANUP(cleanup)
		{
			mlc.check();
			if (mlc.diff){
				std::wstringstream ss;
				ss << L"Memory leak of " << mlc.diff << L" bytes(s) detected";
				Assert::Fail(ss.str().c_str()); 
			}
		}

		TEST_METHOD(createUsingFactory)
		{
			auto p = createMCTSPlayer(0, 1, eval, 0);
			p->release();
		}
		
		TEST_METHOD(selection)
		{
			MCTS_GamePlayer p(0, eval);
			auto *s = &State;
			s->AddRef();
			p.m_root = p.findRootNode(s, nullptr);
			auto path = p.selection(p.m_root);
		}
		TEST_METHOD(selection_playOut)
		{
			MCTS_GamePlayer p(0, eval);
			auto *s = &State;
			s->AddRef();
			p.m_root = p.findRootNode(s, nullptr);
			auto path = p.selection(p.m_root);
			Assert::AreEqual(1ull, path.size());
			Assert::AreEqual( &(State.m_tree[0]), static_cast<const TestGameState*>(path[0].first->state)->m_curState);
			p.playOut(path);
			Assert::AreEqual(4ull, path.size());
		}
		TEST_METHOD(run_1_Simulation)
		{
			MCTS_GamePlayer p(0, eval);
			auto *s = &State;
			s->AddRef();
			p.runSingleSimulation(s);
			Assert::AreEqual(State.m_tree[0].name, p.m_root->state->ToString());
		}
		TEST_METHOD(run_2_Simulations)
		{
			MCTS_GamePlayer p(0, eval);
			auto *s = &State;
			s->AddRef();
			p.runSingleSimulation(s);
			Assert::AreEqual(State.m_tree[0].name, p.m_root->state->ToString());

			p.runSingleSimulation(s);
			Assert::AreEqual(State.m_tree[0].name, p.m_root->state->ToString());
		}
		TEST_METHOD(run_3_Simulations)
		{
			MCTS_GamePlayer p(0, eval);
			auto *s = &State;
			s->AddRef();
			p.runSingleSimulation(s);
			Assert::AreEqual(State.m_tree[0].name, p.m_root->state->ToString());

			p.runSingleSimulation(s);
			Assert::AreEqual(State.m_tree[0].name, p.m_root->state->ToString());
		}
		TEST_METHOD(run_15_Simulations)
		{
			MCTS_GamePlayer p(0, eval, -1, 1234);
			auto *s = &State;
			s->AddRef();
			p.runNSimulations(s, 1);
			p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s1.gv");

			p.runNSimulations(s, 1);
			p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s2.gv");

			p.runNSimulations(s, 1);
			p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s3.gv");

			p.runNSimulations(s, 1);
			p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s4.gv");

			p.runNSimulations(s, 1);
			p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s5.gv");

			p.runNSimulations(s, 5);
			p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s10.gv");

			p.runNSimulations(s, 5);
			p.dumpTree("C:\\MyData\\Projects\\gra_w_pana\\mcts_tree_dump_s15.gv");
		}
	};
}