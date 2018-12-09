#pragma once
#include <string>
using std::string;

struct ITrace
{
	static ITrace* createInstance(const string& filename, const string& outDir = string());
	virtual void trace(const wchar_t *message) = 0;
	virtual void release() = 0;
protected:
	virtual ~ITrace() {}
};

#ifndef TRACE
#ifdef ENABLE_TRACE
#define TRACE(tr, fmt, ...) {\
			static wchar_t tmp[256];\
			swprintf_s(tmp, fmt"\n", ##__VA_ARGS__);\
			tr->trace(tmp);}
#else
#define TRACE(_,fmt, ...)
#endif
#endif
