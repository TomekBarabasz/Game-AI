#pragma once
#include <vector>

struct IRandomGenerator
{
	virtual std::vector<int> generateUniform(int lower, int upper, int number_of_samples) = 0;
};