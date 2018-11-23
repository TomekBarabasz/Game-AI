#include "pch.h"
#include <Trace.h>
#include <codecvt>
#include <iostream>
#include <fstream>

struct FileTrace : ITrace
{
	FileTrace(const string& filename) : m_file(filename)
	{
		std::locale loc(std::locale::classic(), new std::codecvt_utf8<wchar_t>);
		m_file.imbue(loc);
	}
	std::wofstream m_file;
	void trace(const wchar_t *message) override { m_file << message; m_file.flush(); }
	void release() override { delete this; }
};
struct ConsoleTrace : ITrace
{
	void trace(const wchar_t *message) override { ::OutputDebugString(message); }
	void release() override { delete this; }
};
struct StdOutTrace : ITrace
{
	void trace(const wchar_t *message) override { std::wcout << message; }
	void release() override { delete this; }
};
struct DisabledTrace : ITrace
{
	void trace(const wchar_t *message) override { }
	void release() override { delete this; }
};
ITrace* ITrace::createInstance(const string& filename, const string& outDir)
{
	if (filename.empty()) return new DisabledTrace();
	if ("console" == filename) return new ConsoleTrace();
	if ("stdout" == filename) return new StdOutTrace();
	return new FileTrace(outDir + "\\" + filename);
}
