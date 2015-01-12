#ifndef DEBUG_H
#define DEBUG_H

//define DEBUG_FILE "DEBUG.TXT"

#include "..\Ares.h"
#include <cstdio>
#include <string>
#include <YRPPCore.h>
#ifdef _MSC_VER
#include <Dbghelp.h>
#endif

#include "../UI/Dialogs.h"

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
//	static FILE* pDevLogFile;
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
	static __declspec(noreturn) LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS pExs);
	static LONG CALLBACK ExceptionFilter(PEXCEPTION_POINTERS pExs);

	static void FreeMouse();

	/** TODO: review if all these errors are needed */

	static __declspec(noreturn) void Exit(UINT ExitCode = 1u);

	static void FatalError(bool Dump = false); /* takes formatted message from Ares::readBuffer */
	static void FatalError(const char *Message, ...);
	static __declspec(noreturn) void FatalErrorAndExit(const char *Message, ...);
	static int __stdcall FatalDialog_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static void PrepareSnapshotDirectory(std::wstring &buffer);
	static void FullDump(MINIDUMP_EXCEPTION_INFORMATION *pException, std::wstring * destinationFolder = nullptr, std::wstring * generatedFilename = nullptr);

	static void DevLog(Debug::Severity severity, const char* Format, ...);

	static bool bTrackParserErrors;
	static bool bParserErrorDetected;

	static void INIParseFailed(const char *section, const char *flag, const char *value, const char *Message = nullptr);
};

#endif
