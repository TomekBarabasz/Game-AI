#pragma once
#include <boost/variant.hpp>
#include <chrono>
#include <map>
#include <unordered_map>

template <typename T>
struct Histogram
{
	char m_prefix;
	T m_rounding;
	std::map<T, unsigned> values;
	using ValueType_t = typename std::map<T, unsigned>::value_type;

	Histogram() : m_prefix(0), m_rounding(1) {}
	Histogram<T>& Prefix(const char p) { m_prefix = p; return *this; }
	Histogram<T>& Rounding(int digits);
	T round(T val) const { return val; }
	void insert(T val)
	{
		const T rval = round(val);
		auto it = values.find(rval);
		if (it != values.end()) it->second += 1;
		else values[rval] = 1;
	}
	std::string val_to_string(T val) const;
	std::string to_string() const
	{
#if 0
		auto kv_to_string = [](const ValueType_t& kv) {
			return std::to_string(kv.first) + ":" + std::to_string(kv.second);
		};

		return std::accumulate(values.begin(), values.end(),
			kv_to_string(values.begin()),
			//[=](string prev, const ValueType_t& kv) {
			[=](string prev, auto kv) {
			return prev + "," + kv_to_string(kv);
		});
#else
		std::ostringstream ss;
		for (auto kv : values) {
			ss << val_to_string(kv.first) << ":" << std::to_string(kv.second) << ",";
		}
		//ss.str().erase(ss.str().size()-1, 1);
		//return ss.str().erase(ss.str().size()-1, 1);
		//ss.seekp(-1, ss.cur); ss << " ";
		std::string result(ss.str());
		if (!result.empty()) result.pop_back();
		return result;
#endif
	}
	Histogram<T>& operator+=(const Histogram<T>& other)
	{
		for (auto & kv : other.values)
		{
			auto it = values.find(kv.first);
			if (it != values.end()) it->second += kv.second;
			else values[kv.first] = kv.second;
		}
		return *this;
	}
};
template<>
inline Histogram<long>& Histogram<long>::Rounding(int digits)
{
	m_rounding = (int)pow(10, digits);
	return *this;
}
template <>
inline long Histogram<long>::round(long val) const
{
	return (val / m_rounding) * m_rounding;
}
template <>
inline std::string Histogram<long>::val_to_string(long val) const
{
	switch(m_prefix)
	{
	case 'K':	return std::to_string(val / 1000) + 'K';
	case 'M':	return std::to_string(val / 1000000) + 'M';
	default:	return std::to_string(val);
	}
}
template<>
inline Histogram<float>& Histogram<float>::Rounding(int digits)
{
	m_rounding = (float)pow(10, -digits);
	return *this;
}
template <>
float Histogram<float>::round(float val) const
{
	return float(int(val / m_rounding) * m_rounding);
}
template <>
inline std::string Histogram<float>::val_to_string(float val) const
{
	switch (m_prefix)
	{
	case 'm':	return std::to_string(val * 1000) + 'm';
	case 'u':	return std::to_string(val * 1000000) + 'u';
	default:	return std::to_string(val);
	}
}
template<>
inline std::string  Histogram<std::string>::to_string() const
{
	std::ostringstream ss;
	for (auto kv : values) {
		ss << kv.first << ":" << std::to_string(kv.second) << ",";
	}

	std::string result(ss.str());
	if (!result.empty()) result.pop_back();
	return result;
}
template<>
Histogram<std::string>::Histogram(){}
template <>
inline std::string Histogram<std::string>::val_to_string(std::string val) const { return val; }

template <typename T>
struct Average
{
	T	 m_value = T(0);
	long m_count = 0;
	void insert(T val) { m_value == val; ++m_count; }
	Average<T>& operator+=(const Average<T>& other)
	{
		m_value += other.m_value;
		m_count += other.m_count;
		return *this;
	}
	std::string to_string() const
	{
		const double average = m_count > 0 ? m_value / double(m_count) : NAN;
		return std::to_string(average);
	}
};
namespace std {
	template <typename T>
	string to_string(const Histogram<T>& h) { return h.to_string(); }
	template <typename T>
	string to_string(const Average<T>& a) { return a.to_string(); }
	inline string to_string(const string s) { return s; }
	inline string to_string(const chrono::duration<long long, nano>& duration)
	{
		const long long miliseconds = chrono::duration_cast<chrono::milliseconds>(duration).count();
		std::ostringstream ss;
		const auto total_sec = miliseconds / 1000;
		const auto hr = total_sec / 3600;
		const auto min = (total_sec - hr * 3600) / 60;
		const auto sec = total_sec - hr * 3600 - min * 60;
		const auto msec = miliseconds % 1000;

		if (hr) { ss << hr << " hrs "; }
		if (min) { ss << min << " min "; }
		if (sec) { ss << sec << " s "; }
		if (msec) { ss << msec << " ms "; }

		return ss.str();
	}
}

using Duration_t = std::chrono::duration<long long, std::nano>;
using Metric_t = boost::variant<int, float, std::string, Duration_t, Average<long>, Average<float>, Histogram<long>, Histogram<float>, Histogram<std::string>>;
using NamedMetrics_t = std::unordered_map<std::string, Metric_t>;