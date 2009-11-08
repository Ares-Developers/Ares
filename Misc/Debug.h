#ifndef DEBUG_H
#define DEBUG_H

//define DEBUG_FILE "DEBUG.TXT"

#include <stdio.h>
#include <YRPPCore.h>
#include <Helpers\Macro.h>
#include <Windows.h>
#include <Dbghelp.h>

#include "UI\Dialogs.h"

class Debug
{
public:
	static bool bLog;
	static FILE* pLogFile;
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
};

#endif
