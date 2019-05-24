#pragma once
#include <vector>

struct IRandomGenerator
{
	virtual std::vector<int> generateUniform(int lower, int upper, int number_of_samples) = 0;
	virtual void release() = 0;
};

template <typename T>
void shuffle(T& container, std::vector<int> random_numbers)
{
	for (int i = 0; i < random_numbers.size();)
	{
		auto & t0 = container[random_numbers[i++]];
		auto & t1 = container[random_numbers[i++]];
		auto t = t0;
		t0 = t1;
		t1 = t;
	}
}