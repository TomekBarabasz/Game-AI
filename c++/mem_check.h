#pragma once
#include <crtdbg.h>

struct MemoryLeakChecker
{
	_CrtMemState memState_;
	size_t diff = 0;
	void checkpoint()
	{
		_CrtMemCheckpoint(&memState_);
		diff = 0;
	}
	size_t check()
	{
		_CrtMemState stateNow, stateDiff;
		_CrtMemCheckpoint(&stateNow);
		int diffResult = _CrtMemDifference(&stateDiff, &memState_, &stateNow);
		if (diffResult)
		{
			_CrtDumpMemoryLeaks();
		}
		diff =  stateDiff.lSizes[1];
		return diff;
	}
};