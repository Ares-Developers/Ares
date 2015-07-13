#pragma once

#include <cstdio>
#include <string>

class REGISTERS;

class Debug
{
public:
	enum Severity {
		Notice = 0x1,
		Warning = 0x2,
		Error = 0x3
	};
	static const char * SeverityString(Debug::Severity severity);

	static bool bLog;
	static FILE* pLogFile;
	static std::wstring LogFileName;
	static std::wstring LogFileTempName;

	static void MakeLogFile();
	static void LogFileOpen();
	static void LogFileClose(int tag);
	static void LogFileRemove();
	static void DumpObj(void const* data, size_t len);
	static void DumpStack(REGISTERS* R, size_t len, int startAt = 0);
	static void (_cdecl* Log)(const char* pFormat, ...);

	template <typename T>
	static void DumpObj(const T& object) {
		DumpObj(&object, sizeof(object));
	}

	static void __cdecl LogUnflushed(const char *Format, ...);
	static void LogUnflushed(const char* Format, va_list ArgList);
	static void Flush();

	static void FreeMouse();

	/** TODO: review if all these errors are needed */

	static void FatalError(bool Dump = false); /* takes formatted message from Ares::readBuffer */
	static void FatalError(const char *Message, ...);
	static __declspec(noreturn) void FatalErrorAndExit(const char *Message, ...);

	static std::wstring FullDump();
	static std::wstring FullDump(std::wstring destinationFolder);

	static void DevLog(Debug::Severity severity, const char* Format, ...);

	static bool bTrackParserErrors;
	static bool bParserErrorDetected;

	static void INIParseFailed(const char *section, const char *flag, const char *value, const char *Message = nullptr);
};
