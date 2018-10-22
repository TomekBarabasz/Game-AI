#pragma once


template <typename T>
struct IMetric
{
	virtual void update(T value) = 0;
	virtual void merge(const IMetric<T>& other) = 0;
	virtual IMetric<T> apply(std::function<void()>) = 0;
};