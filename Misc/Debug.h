#ifndef DEBUG_H
#define DEBUG_H

//define DEBUG_FILE "DEBUG.TXT"

#include <cstdio>
#include <YRPPCore.h>
#include <Windows.h>
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
	static wchar_t LogFileName[MAX_PATH];
	static wchar_t LogFileTempName[MAX_PATH];

	static void MakeLogFile();
	static void LogFileOpen();
	static void LogFileClose();
	static void LogFileRemove();
	static void DumpObj(byte *data, size_t len);
	static void DumpStack(REGISTERS *R, size_t len);
	static void (_cdecl* Log)(const char* pFormat, ...);

	static void __cdecl LogUnflushed(const char *Format, ...);
	static void Flush();
	static LONG WINAPI ExceptionHandler(int code, LPEXCEPTION_POINTERS pExs);

	static void FreeMouse();
	static void FatalError(const char *Message, bool Exit = 1);
	static int __stdcall FatalDialog_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static void DevLog(Debug::Severity severity, const char* Format, ...);
};

#endif
