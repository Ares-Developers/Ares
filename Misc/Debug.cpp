#include "Debug.h"
#include <Unsorted.h>
#include <MouseClass.h>
#include <WWMouseClass.h>

bool Debug::bLog = true;
FILE *Debug::pLogFile = NULL;
wchar_t Debug::LogFileName[MAX_PATH] = L"\0";
wchar_t Debug::LogFileTempName[MAX_PATH] = L"\0";

void (_cdecl* Debug::Log)(const char* pFormat, ...) =
	(void (__cdecl *)(const char *,...))0x4068E0;

void Debug::LogFileOpen()
{
	LogFileClose();
	MakeLogFile();

	pLogFile = _wfopen(LogFileTempName, L"w");
}

void Debug::LogFileClose()
{
	if(Debug::pLogFile) {
		fclose(Debug::pLogFile);
		CopyFileW(LogFileTempName, LogFileName, FALSE);
	}

	Debug::pLogFile = NULL;
}

void Debug::MakeLogFile() {
	static bool made = 0;
	if(!made) {
		wchar_t path[MAX_PATH];

		SYSTEMTIME time;

		GetLocalTime(&time);
		GetCurrentDirectoryW(MAX_PATH, path);

		swprintf(LogFileName, MAX_PATH, L"%s\\debug", path);
		CreateDirectoryW(LogFileName, NULL);

		swprintf(LogFileTempName, MAX_PATH, L"%s\\debug\\debug.log", 
			path);

		swprintf(LogFileName, MAX_PATH, L"%s\\debug\\debug.%04u%02u%02u-%02u%02u%02u.log", 
			path, time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

		made = 1;
	}
}

void Debug::LogFileRemove()
{
	LogFileClose();
	DeleteFileW(LogFileTempName);
}

void Debug::DumpObj(byte *data, size_t len) {
	Debug::Log("Dumping %u bytes of object at %X\n", len, data);

	Debug::Log(" 00000 |");
	for(DWORD rem = 0; rem < 0x10; ++rem) {
		Debug::Log(" %02X |", rem);
	}
	Debug::Log("\n\n");
	for(DWORD dec = 0; dec < len / 0x10; ++dec) {
		Debug::Log(" %04X0 |", dec);
		for(DWORD rem = 0; rem < 0x10; ++rem) {
			Debug::Log(" %02X |", data[dec * 0x10 + rem]);
		}
		for(DWORD rem = 0; rem < 0x10; ++rem) {
			byte sym = data[dec * 0x10 + rem];
			Debug::Log("%c", isprint(sym) ? sym : '?');
		}
		Debug::Log("\n");
	}

	DWORD dec = len / 0x10 * 0x10;
	DWORD remlen = len - dec;
	Debug::Log(" %05X |", dec);
	for(DWORD rem = 0; rem < remlen; ++rem) {
		Debug::Log(" %02X |", data[dec + rem]);
	}
	for(DWORD rem = remlen; rem < 0x10; ++rem) {
		Debug::Log(" -- |");
	}
	for(DWORD rem = 0; rem < remlen; ++rem) {
		byte sym = data[dec + rem];
		Debug::Log("%c", isprint(sym) ? sym : '?');
	}
	Debug::Log("\nEnd of dump.\n");
}

void Debug::DumpStack(REGISTERS *R, size_t len) {
	Debug::Log("Dumping %X bytes of stack\n", len);
	for(size_t i = 0; i < len; i += 4) {
		Debug::Log("esp+%04X = %08X\n", i, R->get_StackVar32(i));
	}
	Debug::Log("Done.\n");
}

void __cdecl Debug::LogUnflushed(const char *Format, ...) {
	if(Debug::bLog && Debug::pLogFile) {
		va_list ArgList;
		va_start(ArgList, Format);
		vfprintf(Debug::pLogFile, Format, ArgList);
		va_end(ArgList);
	}
}

void Debug::Flush() {
	if(Debug::bLog && Debug::pLogFile) {
		fflush(Debug::pLogFile);
	}
}

/**
 * minidump
 */

LONG WINAPI Debug::ExceptionHandler(int code, LPEXCEPTION_POINTERS pExs)
{
	Debug::FreeMouse();
	Debug::Log("D::EH\n");
	bool g_ExtendedMinidumps = true;
//	if (IsDebuggerAttached()) return EXCEPTION_CONTINUE_SEARCH;
	if (pExs->ExceptionRecord->ExceptionCode == ERROR_MOD_NOT_FOUND ||
		pExs->ExceptionRecord->ExceptionCode == ERROR_PROC_NOT_FOUND)
	{
		Debug::Log("D::EH - _NOT_FOUND\n");
		//tell user
		ExitProcess(pExs->ExceptionRecord->ExceptionCode);
	}

	switch(pExs->ExceptionRecord->ExceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION:
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		case EXCEPTION_BREAKPOINT:
		case EXCEPTION_DATATYPE_MISALIGNMENT:
		case EXCEPTION_FLT_DENORMAL_OPERAND:
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		case EXCEPTION_FLT_INEXACT_RESULT:
		case EXCEPTION_FLT_INVALID_OPERATION:
		case EXCEPTION_FLT_OVERFLOW:
		case EXCEPTION_FLT_STACK_CHECK:
		case EXCEPTION_FLT_UNDERFLOW:
		case EXCEPTION_ILLEGAL_INSTRUCTION:
		case EXCEPTION_IN_PAGE_ERROR:
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
		case EXCEPTION_INT_OVERFLOW:
		case EXCEPTION_INVALID_DISPOSITION:
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		case EXCEPTION_PRIV_INSTRUCTION:
		case EXCEPTION_SINGLE_STEP:
		case EXCEPTION_STACK_OVERFLOW:
		case 0xE06D7363: // exception thrown and not caught
		{
			Debug::Log("D::EH - Dump\n");

			wchar_t filename[MAX_PATH];
			wchar_t path[MAX_PATH];
		
			HANDLE dumpFile;
			SYSTEMTIME time;
			MINIDUMP_EXCEPTION_INFORMATION expParam;
			
			GetLocalTime(&time);
			GetCurrentDirectoryW(MAX_PATH, path);

			swprintf(filename, MAX_PATH, L"%s\\debug", path);
			CreateDirectoryW(filename, NULL);

			swprintf(filename, MAX_PATH, g_ExtendedMinidumps ? L"%s\\debug\\extcrashdump.%04u%02u%02u-%02u%02u%02u.dmp" : L"%s\\debug\\crashdump.%04u%02u%02u-%02u%02u%02u.dmp", 
							path, 
							time.wYear, time.wMonth, time.wDay, 
							time.wHour, time.wMinute, time.wSecond);

			dumpFile = CreateFileW(filename, GENERIC_READ | GENERIC_WRITE, 
							FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);

			expParam.ThreadId = GetCurrentThreadId();
			expParam.ExceptionPointers = pExs;
			expParam.ClientPointers = FALSE;

			MINIDUMP_TYPE type = (MINIDUMP_TYPE) ((g_ExtendedMinidumps ? MiniDumpWithFullMemory : (MiniDumpWithDataSegs | MiniDumpWithIndirectlyReferencedMemory)));

			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dumpFile, type, &expParam, NULL, NULL);
			CloseHandle(dumpFile);

			Debug::FatalError("The cause of this error could not be determined.\r\n"
				"A crash dump should have been created in your game's \\debug subfolder.\r\n"
				"You can submit that to the developers (along with debug.txt and syringe.log).", 0);

			ExitProcess(pExs->ExceptionRecord->ExceptionCode); // Exit.
			break;
		}
		default:
			Debug::Log("D::EH - _CS\n");
			Debug::DumpObj((byte *)pExs->ExceptionRecord, sizeof(*(pExs->ExceptionRecord)));
			ExitProcess(pExs->ExceptionRecord->ExceptionCode); // Exit.
//			return EXCEPTION_CONTINUE_SEARCH;
			break;
	}
};

void Debug::FreeMouse() {
	MouseClass::Instance->SetPointer(0, 0);
	WWMouseClass::Instance->ReleaseMouse();

	ShowCursor(1);
}

void Debug::FatalError(const char *Message, bool Exit) {
	Debug::FreeMouse();
	strncpy(Dialogs::ExceptDetailedMessage, Message, 0x400);

	Debug::Log("\nFatal Error:\n");
	Debug::Log(Message);

	LPCDLGTEMPLATEA DialogBox = reinterpret_cast<LPCDLGTEMPLATEA>(Game::GetResource(247, 5));

	DialogBoxIndirectParamA(Game::hInstance, DialogBox, Game::hWnd, &Debug::FatalDialog_WndProc, 0);

	if(Exit) {
		Debug::Log("Exiting...\n");
		ExitProcess(1);
	}
}

int __stdcall Debug::FatalDialog_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if(uMsg > WM_COMMAND) {
		if(uMsg == WM_MOVING) {
			Game::sub_776D80((tagRECT *)lParam);
		}
		return 0;
	}
	switch(uMsg) {
		case WM_COMMAND:
			if(wParam == 1153) {
				EndDialog(hWnd, 1153);
			}
			return 0;
		case WM_CLOSE:
			EndDialog(hWnd, 1153);
			Game::sub_53E420(hWnd);
			return 0;
		case WM_INITDIALOG:
			SetDlgItemTextA(hWnd, Dialogs::ExceptControlID, Dialogs::ExceptDetailedMessage);
			SetFocus(hWnd);
			if ( Game::hWnd ) {
				Game::CenterWindowIn(hWnd, Game::hWnd);
			}
			ShowWindow(hWnd, 1);
			Game::sub_53E3C0(hWnd);
		default:
			return 0;
	}
}

DEFINE_HOOK(4C850B, Exception_Dialog, 5)
{
	Debug::FreeMouse();
	return 0;
}

DEFINE_HOOK(4068E0, Debug_Log, 1)
DEFINE_HOOK_AGAIN(4A4AC0, Debug_Log, 1)
{
	if(Debug::bLog && Debug::pLogFile)
	{
		va_list ArgList = (va_list)(R->get_ESP() + 0x8);
		char* Format = (char*)R->get_StackVar32(0x4);

		vfprintf(Debug::pLogFile, Format, ArgList);
		fflush(Debug::pLogFile);
	}
	return 0x4A4AF9; // changed to co-op with YDE
}

//ifdef DUMP_EXTENSIVE
DEFINE_HOOK(4C8FE0, Exception_Handler, 9)
{
	GET(int, code, ECX);
	GET(LPEXCEPTION_POINTERS, pExs, EDX);
	Debug::ExceptionHandler(code, pExs);
}
//endif
