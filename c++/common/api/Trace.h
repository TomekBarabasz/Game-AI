#pragma once
#include <string>
#include <variant>
#include <functional>
#include <vector>

using std::string;
using std::wstring;

namespace Trace
{
	using Param_t = std::variant<int, unsigned, wstring>;
	using ParamList_t = std::vector<Param_t>;
	using ParamListGen_t = std::function<ParamList_t()>;
	wstring do_format(const wchar_t* fmt, const ParamList_t& prms);

	struct ITrace
	{
		virtual void trace(const wchar_t* message) = 0;
		virtual void trace(const wchar_t* fmt, ParamListGen_t pg) = 0;

		virtual void release() = 0;
	protected:
		virtual ~ITrace() {}
	};
	ITrace* createInstance(const string& filename, const string& outDir = string());
}

using namespace Trace;
#ifndef TRACE
#ifdef ENABLE_TRACE
#define TRACE(tr, fmt, ...) {\
			static wchar_t tmp[512];\
			swprintf_s(tmp, fmt"\n", ##__VA_ARGS__);\
			tr->trace(tmp);}
#define TRACE_L(tr, fmt, ...)  tr->trace(fmt"\n", [&   ](){ return ParamList_t {##__VA_ARGS__};} );
#define TRACE_LT(tr, fmt, ...) tr->trace(fmt"\n", [this](){ return ParamList_t {##__VA_ARGS__};} );
#else
#define TRACE(_,fmt, ...)
#define TRACE_L(tr, fmt, ...)
#define TRACE_LT(tr, fmt, ...)
#endif
#endif
