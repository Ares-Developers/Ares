#pragma once

#include <cstdio>
#include <string>

class REGISTERS;

class Debug
{
public:
	enum class Severity {
		None = 0,
		Vebose = 1,
		Notice = 2,
		Warning = 3,
		Error = 4,
		Fatal = 5
	};

	// logging

	template <typename... TArgs>
	static void Log(Debug::Severity severity, const char* const pFormat, TArgs&&... args) {
		Debug::LogFlushed(severity, pFormat, std::forward<TArgs>(args)...);
	}

	template <typename... TArgs>
	static void Log(const char* const pFormat, TArgs&&... args) {
		Debug::LogFlushed(pFormat, std::forward<TArgs>(args)...);
	}

	static void RegisterParserError() {
		if(Debug::bTrackParserErrors) {
			Debug::bParserErrorDetected = true;
		}
	}

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

	template <typename T>
	static void DumpObj(const T& object) {
		DumpObj(&object, sizeof(object));
	}

	static void LogWithVArgs(const char* const pFormat, va_list args);

	static void FreeMouse();

	/** TODO: review if all these errors are needed */

	static void FatalError(bool Dump = false); /* takes formatted message from Ares::readBuffer */
	static void FatalError(const char *Message, ...);
	static __declspec(noreturn) void FatalErrorAndExit(const char *Message, ...);

	static std::wstring FullDump();
	static std::wstring FullDump(std::wstring destinationFolder);

	static bool bTrackParserErrors;
	static bool bParserErrorDetected;

	static void INIParseFailed(const char *section, const char *flag, const char *value, const char *Message = nullptr);

private:
	static const char* SeverityString(Debug::Severity severity);

	// the two base cases, with flushing
	static void __cdecl LogFlushed(const char* pFormat, ...);
	static void __cdecl LogFlushed(
		Debug::Severity severity, const char* pFormat, ...);

	// no flushing
	static void __cdecl LogUnflushed(const char* pFormat, ...);
	static void LogWithVArgsUnflushed(const char* pFormat, va_list args);

	static void Flush();
};
