#include "stdafx.h"
#include "CppUnitTest.h"
#include "CppUnitTestAssert.h"
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#include "..\gra_w_pana_rules.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_MODULE_INITIALIZE(ModuleInitialize)
	{
		// enable google mock
		::testing::GTEST_FLAG(throw_on_failure) = true;
		int argc = 0;
		char **argv = NULL;
		::testing::InitGoogleMock(&argc, argv);
	}

	TEST_CLASS(GameState_UT)
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

		TEST_METHOD(GameStateCreateMemTest)
		{
			auto gs = GameState::create();
			gs->Release();
		}
		TEST_METHOD(PlayCardMemTest)
		{
			auto mv1 = PlayCard::create({ {9,0},{9,1},{9,2} });
			auto mv2 = PlayCard::create({ {9,0},{9,1} });
			auto mv3 = PlayCard::create({ {9,0} });
			mv1->release();
			mv2->release();
			mv3->release();
		}

	};
}