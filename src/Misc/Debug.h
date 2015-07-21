#pragma once

#include <cstdio>
#include <string>

class REGISTERS;

class Debug
{
public:
	enum class Severity {
		None = 0,
		Verbose = 1,
		Notice = 2,
		Warning = 3,
		Error = 4,
		Fatal = 5
	};

	// logging

	template <typename... TArgs>
	static void Log(bool enabled, Debug::Severity severity, const char* const pFormat, TArgs&&... args) {
		if(enabled) {
			Debug::Log(severity, pFormat, std::forward<TArgs>(args)...);
		}
	}

	template <typename... TArgs>
	static void Log(bool enabled, const char* const pFormat, TArgs&&... args) {
		if(enabled) {
			Debug::Log(pFormat, std::forward<TArgs>(args)...);
		}
	}

	template <typename... TArgs>
	static void Log(Debug::Severity severity, const char* const pFormat, TArgs&&... args) {
		Debug::LogFlushed(severity, pFormat, std::forward<TArgs>(args)...);
	}

	template <typename... TArgs>
	static void Log(const char* const pFormat, TArgs&&... args) {
		Debug::LogFlushed(pFormat, std::forward<TArgs>(args)...);
	}

	static void LogWithVArgs(const char* const pFormat, va_list args);

	// parser errors

	static bool bTrackParserErrors;
	static bool bParserErrorDetected;

	static void INIParseFailed(const char *section, const char *flag, const char *value, const char *Message = nullptr);

	static void RegisterParserError() {
		if(Debug::bTrackParserErrors) {
			Debug::bParserErrorDetected = true;
		}
	}

	// dumping

	static void DumpStack(REGISTERS* R, size_t len, int startAt = 0);

	static void DumpObj(void const* data, size_t len);

	template <typename T>
	static void DumpObj(const T& object) {
		DumpObj(&object, sizeof(object));
	}

	static std::wstring FullDump();
	static std::wstring FullDump(std::wstring destinationFolder);

	// unsorted

	static bool bLog;
	static std::wstring LogFileName;
	static std::wstring LogFileTempName;

	static void MakeLogFile();
	static void LogFileOpen();
	static void LogFileClose(int tag);
	static void LogFileRemove();

	static void FreeMouse();

	/** TODO: review if all these errors are needed */

	static void FatalError(bool Dump = false); /* takes formatted message from Ares::readBuffer */
	static void FatalError(const char *Message, ...);
	[[noreturn]] static void FatalErrorAndExit(const char *Message, ...);

private:
	static FILE* LogFile;

	static bool LogFileActive() {
		return Debug::bLog && Debug::LogFile;
	}

	static const char* SeverityString(Debug::Severity severity);

	// the two base cases, with flushing
	static void __cdecl LogFlushed(const char* pFormat, ...);
	static void __cdecl LogFlushed(
		Debug::Severity severity, const char* pFormat, ...);

	// no flushing, and unchecked
	static void __cdecl LogUnflushed(const char* pFormat, ...);
	static void LogWithVArgsUnflushed(const char* pFormat, va_list args);

	// flush unchecked
	static void Flush();
};
