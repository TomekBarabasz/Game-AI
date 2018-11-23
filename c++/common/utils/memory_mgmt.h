#pragma once
namespace MemoryMgmt
{
	struct MemoryPools;
	MemoryPools* makeMemoryPoolsInst();
	void freeMemoryPoolsInst(MemoryPools* i);
	MemoryPools* getMemoryPoolsInst();
}
