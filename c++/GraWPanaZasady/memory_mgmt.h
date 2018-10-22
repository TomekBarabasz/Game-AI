#pragma once
struct MemoryPools;
MemoryPools* makeMemoryPoolsInst();
void freeMemoryPoolsInst(MemoryPools* i);
MemoryPools* getMemoryPoolsInst();
