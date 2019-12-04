#include "pch.h"
#include <Trace.h>
#include <codecvt>
#include <iostream>
#include <fstream>
#include <sstream>

namespace Trace
{
	wstring do_format(const wchar_t* fmt, const ParamList_t& prms)
	{
		std::wstringstream oss;

		auto vit = prms.begin();
		oss.clear();
		oss.str(L"");
		for (; *fmt != '\0'; fmt++) {
			if (*fmt == '%') {
				std::visit([&oss](auto&& arg) {oss << arg; }, *vit);
				++fmt;
				++vit;
			}
			else {
				oss << *fmt;
			}
		}
		return oss.str();
	}
	struct FileTrace : ITrace
	{
		FileTrace(const string& filename) : m_file(filename)
		{
			std::locale loc(std::locale::classic(), new std::codecvt_utf8<wchar_t>);
			m_file.imbue(loc);
		}
		std::wofstream m_file;
		void trace(const wchar_t* message) override { m_file << message; m_file.flush(); }
		void trace(const wchar_t* fmt, ParamListGen_t pg) override
		{
			const auto prms = pg();
			m_file << do_format(fmt, prms);
			m_file.flush();
		}
		void release() override { delete this; }
	};
	
	struct ConsoleTrace : ITrace
	{
		void trace(const wchar_t* message) override { ::OutputDebugString(message); }
		void trace(const wchar_t* fmt, ParamListGen_t pg) override
		{
			const auto prms = pg();
			::OutputDebugString(do_format(fmt, prms).c_str());
		}
		void release() override { delete this; }
	};
	struct StdOutTrace : ITrace
	{
		void trace(const wchar_t* message) override { std::wcout << message; }
		void trace(const wchar_t* fmt, ParamListGen_t pg) override
		{
			const auto prms = pg();
			std::wcout << do_format(fmt, prms);
		}
		void release() override { delete this; }
	};
	struct DisabledTrace : ITrace
	{
		void trace(const wchar_t* message) override { }
		void trace(const wchar_t* fmt, ParamListGen_t pg) override {}
		void release() override { delete this; }
	};
	ITrace* createInstance(const string& filename, const string& outDir)
	{
		if (filename.empty()) return new DisabledTrace();
		if ("console" == filename) return new ConsoleTrace();
		if ("stdout" == filename) return new StdOutTrace();
		return new FileTrace(outDir + "\\" + filename);
	}
}

