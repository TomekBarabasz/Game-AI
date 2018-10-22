#include "stdafx.h"
#include "CppUnitTest.h"
#include "CppUnitTestAssert.h"
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#include "..\mcts_player.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(MCTSPlayer_UT)
	{
		_CrtMemState memState_;
	public:
		TEST_METHOD_INITIALIZE(init)
		{
			_CrtMemCheckpoint(&memState_);
		}
		TEST_METHOD_CLEANUP(cleanup)
		{
			_CrtMemState stateNow, stateDiff;
			_CrtMemCheckpoint(&stateNow);
			int diffResult = _CrtMemDifference(&stateDiff, &memState_, &stateNow);
			if (diffResult)
			{
				_CrtDumpMemoryLeaks();
				std::wstringstream ss;
				ss << L"Memory leak of " << stateDiff.lSizes[1] << L" bytes(s) detected";
				Assert::Fail(ss.str().c_str());
			}
		}

		TEST_METHOD(Create)
		{
			auto eval = [](const IGameState*, int value[]) { value[0] = value[1] = 50; };
			auto p = createMCTSPlayer(0, 1, eval, 0);
			p->release();
		}
	};
}