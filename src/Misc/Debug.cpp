#include "Debug.h"
#include <Unsorted.h>
#include <MouseClass.h>
#include <WWMouseClass.h>
#include <CRT.h>
#include "..\Ares.version.h"

bool Debug::bLog = true;
bool Debug::bTrackParserErrors = false;
bool Debug::bParserErrorDetected = false;

FILE *Debug::pLogFile = nullptr;
std::wstring Debug::LogFileName;
std::wstring Debug::LogFileTempName;

void (_cdecl* Debug::Log)(const char* pFormat, ...) =
	(void (__cdecl *)(const char *,...))0x4068E0;

void Debug::DevLog(Debug::Severity severity, const char* Format, ...) {
	if(Debug::bTrackParserErrors) {
		Debug::bParserErrorDetected = true;
	}
	if(!pLogFile) {
		return;
	}
	va_list args;
	fprintf(pLogFile, "[Developer %s]", Debug::SeverityString(severity));
	va_start(args, Format);
	vfprintf(pLogFile, Format, args);
	va_end(args);
//	fprintf(pLogFile, "\n");
}

const char * Debug::SeverityString(Debug::Severity severity) {
	switch(severity) {
		case Notice:
			return "notice";
		case Warning:
			return "warning";
		case Error:
			return "error";
		default:
			return "wtf";
	}
}

void Debug::LogFileOpen()
{
	Debug::MakeLogFile();
	Debug::LogFileClose(999);

	pLogFile = _wfsopen(Debug::LogFileTempName.c_str(), L"w", _SH_DENYWR);
	if(!pLogFile) {
		wchar_t msg[100] = L"\0";
		wsprintfW(msg, L"Log file failed to open. Error code = %X", errno);
		MessageBoxW(Game::hWnd, Debug::LogFileTempName.c_str(), msg, MB_OK | MB_ICONEXCLAMATION);
		ExitProcess(1);
	}
}

void Debug::LogFileClose(int tag)
{
	if(Debug::pLogFile) {
		fprintf(Debug::pLogFile, "Closing log file on request %d", tag);
		fclose(Debug::pLogFile);
		CopyFileW(Debug::LogFileTempName.c_str(), Debug::LogFileName.c_str(), FALSE);
		Debug::pLogFile = nullptr;
	}
}

void Debug::MakeLogFile() {
	static bool made = 0;
	if(!made) {
		wchar_t path[MAX_PATH];

		SYSTEMTIME time;

		GetLocalTime(&time);
		GetCurrentDirectoryW(MAX_PATH, path);

		Debug::LogFileName = path;
		Debug::LogFileName += L"\\debug";
		
		CreateDirectoryW(Debug::LogFileName.c_str(), nullptr);

		wchar_t subpath[64];
		swprintf(subpath, 64, L"\\debug.%04u%02u%02u-%02u%02u%02u.log",
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

		Debug::LogFileTempName = Debug::LogFileName;
		Debug::LogFileTempName += L"\\debug.log";
		Debug::LogFileName += subpath;

		made = 1;
	}
}

void Debug::LogFileRemove()
{
	Debug::LogFileClose(555);
	DeleteFileW(Debug::LogFileTempName.c_str());
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

void Debug::DumpStack(REGISTERS *R, size_t len, size_t startAt) {
	Debug::Log("Dumping %X bytes of stack\n", len);
	for(size_t i = startAt; i < len; i += 4) {
		Debug::Log("esp+%04X = %08X\n", i, R->Stack32(i));
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

void Debug::PrepareSnapshotDirectory(std::wstring &buffer) {
	wchar_t path[MAX_PATH];
	SYSTEMTIME time;

	GetLocalTime(&time);
	GetCurrentDirectoryW(MAX_PATH, path);

	buffer = path;
	buffer += L"\\debug";
	CreateDirectoryW(buffer.c_str(), nullptr);

	wchar_t subpath[64];
	swprintf(subpath, 64, L"\\snapshot-%04u%02u%02u-%02u%02u%02u", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	buffer += subpath;
	CreateDirectoryW(buffer.c_str(), nullptr);
}

#pragma warning(push)
#pragma warning(disable: 4646) // this function does not return, though it isn't declared VOID

__declspec(noreturn) LONG CALLBACK Debug::ExceptionHandler(PEXCEPTION_POINTERS pExs)
{
	Debug::FreeMouse();
	Debug::Log("Exception handler fired!\n");
	Debug::Log("Exception %X at %p\n", pExs->ExceptionRecord->ExceptionCode, pExs->ExceptionRecord->ExceptionAddress);
	SetWindowTextW(Game::hWnd, L"Fatal Error - Yuri's Revenge");

//	if (IsDebuggerAttached()) return EXCEPTION_CONTINUE_SEARCH;

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
			std::wstring path;

			Debug::PrepareSnapshotDirectory(path);

			Debug::Flush();
			if(Debug::bLog) {
				std::wstring logCopy = path + L"\\debug.log";
				CopyFileW(Debug::LogFileTempName.c_str(), logCopy.c_str(), FALSE);
			}

			std::wstring except_file = path + L"\\except.txt";

			if(FILE *except = _wfsopen(except_file.c_str(), L"w", _SH_DENYNO)) {
#define DELIM "---------------------\n"
				fprintf(except, "Internal Error encountered!\n");
				fprintf(except, DELIM);
				fprintf(except, VERSION_STRVER);
				fprintf(except, "\n" DELIM);

				fprintf(except, "Exception code: %08X at %08p\n", pExs->ExceptionRecord->ExceptionCode, pExs->ExceptionRecord->ExceptionAddress);

				fprintf(except, "Registers:\n");
				PCONTEXT pCtxt = pExs->ContextRecord;
				fprintf(except, "EIP: %08X\tESP: %08X\tEBP: %08X\n", pCtxt->Eip, pCtxt->Esp, pCtxt->Ebp);
				fprintf(except, "EAX: %08X\tEBX: %08X\tECX: %08X\n", pCtxt->Eax, pCtxt->Ebx, pCtxt->Ecx);
				fprintf(except, "EDX: %08X\tESI: %08X\tEDI: %08X\n", pCtxt->Edx, pCtxt->Esi, pCtxt->Edi);

				fprintf(except, "\nStack dump:\n");
				DWORD *ptr = (DWORD *)(pCtxt->Esp);
				for(int i = 0; i < 0x100; ++i) {
					fprintf(except, "%08p: %08X\n", ptr, *ptr);
					++ptr;
				}

				fclose(except);
				Debug::Log("Exception data has been saved to file:\n%ls\n", except_file.c_str());
			}

			if(MessageBoxW(Game::hWnd, L"Yuri's Revenge has encountered a fatal error!\nWould you like to create a full crash report for the developers?", L"Fatal Error!", MB_YESNO | MB_ICONERROR) == IDYES) {
				HCURSOR loadCursor = LoadCursor(nullptr, IDC_WAIT);
				SetClassLong(Game::hWnd, GCL_HCURSOR, (LONG)loadCursor);
				SetCursor(loadCursor);
				Debug::Log("Making a memory dump\n");

				MINIDUMP_EXCEPTION_INFORMATION expParam;
				expParam.ThreadId = GetCurrentThreadId();
				expParam.ExceptionPointers = pExs;
				expParam.ClientPointers = FALSE;

				Debug::FullDump(&expParam, &path);

				loadCursor = LoadCursor(nullptr, IDC_ARROW);
				SetClassLong(Game::hWnd, GCL_HCURSOR, (LONG)loadCursor);
				SetCursor(loadCursor);
				Debug::FatalError("The cause of this error could not be determined.\r\n"
					"%s"
					"A crash dump should have been created in your game's \\debug subfolder.\r\n"
					"You can submit that to the developers (along with debug.txt and syringe.log)."
						, Debug::bParserErrorDetected ? "(One or more parser errors have been detected that might be responsible. Check the debug logs.)\r\n" : ""
				);
			}
			break;
		}
		case ERROR_MOD_NOT_FOUND:
		case ERROR_PROC_NOT_FOUND:
			Debug::Log("Massive failure: Procedure or module not found!\n");
			break;
		default:
			Debug::Log("Massive failure: reason unknown, have fun figuring it out\n");
			Debug::DumpObj((byte *)pExs->ExceptionRecord, sizeof(*(pExs->ExceptionRecord)));
//			return EXCEPTION_CONTINUE_SEARCH;
			break;
	}

	Debug::Exit(pExs->ExceptionRecord->ExceptionCode);
};

#pragma warning(pop)

LONG CALLBACK Debug::ExceptionFilter(PEXCEPTION_POINTERS pExs)
{
	static const DWORD MS_VC_EXCEPTION = 0x406D1388;

	switch(pExs->ExceptionRecord->ExceptionCode) {
	case EXCEPTION_BREAKPOINT:
	case MS_VC_EXCEPTION:
		return EXCEPTION_CONTINUE_SEARCH;
	}

	Debug::ExceptionHandler(pExs);
}

void Debug::FullDump(MINIDUMP_EXCEPTION_INFORMATION *pException, std::wstring * destinationFolder, std::wstring * generatedFilename) {
	std::wstring filename;
	if(destinationFolder) {
		filename = *destinationFolder;
	} else {
		Debug::PrepareSnapshotDirectory(filename);
	}

	filename += L"\\extcrashdump.dmp";

	if(generatedFilename) {
		generatedFilename->assign(filename);
	}

	HANDLE dumpFile = CreateFileW(filename.c_str(), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, nullptr);

	MINIDUMP_TYPE type = (MINIDUMP_TYPE)MiniDumpWithFullMemory;

	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dumpFile, type, pException, nullptr, nullptr);
	CloseHandle(dumpFile);
}

void Debug::FreeMouse() {
//	static bool freed = false;
//	if(!freed) {
		Game::sub_53E6B0();

		MouseClass::Instance->SetPointer(0, 0);
		WWMouseClass::Instance->ReleaseMouse();

		ShowCursor(1);

#define BLACK_SURFACE(s) \
		if(DSurface::s) { \
			DSurface::s->Fill(0); \
		}

		BLACK_SURFACE(Alternate);
		BLACK_SURFACE(Composite);
		BLACK_SURFACE(Hidden);
		BLACK_SURFACE(Hidden_2);
		BLACK_SURFACE(Primary);
		BLACK_SURFACE(Sidebar);
		BLACK_SURFACE(Tile);

		ShowCursor(1);

//		DirectDrawWrap::DisposeOfStuff();
//		DirectDrawWrap::lpDD->SetCooperativeLevel(Game::hWnd, eDDCoopLevel::DDSCL_NORMAL);
//		DirectDrawWrap::lpDD->SetDisplayMode(800, 600, 16);

//		freed = true;
//	}
}

__declspec(noreturn) void Debug::Exit(UINT ExitCode) {
		Debug::Log("Exiting...\n");
		ExitProcess(ExitCode);
}

void Debug::FatalError(bool Dump) {
	wsprintfW(Dialogs::ExceptDetailedMessage,
		L"Ares has encountered an internal error and is unable to continue normally. "
		L"Please visit our website at http://ares.strategy-x.com for updates and support.\n\n"
		L"%hs",
		Ares::readBuffer);

	Debug::Log("\nFatal Error:\n");
	Debug::Log("%s\n", Ares::readBuffer);

	MessageBoxW(Game::hWnd,
		Dialogs::ExceptDetailedMessage,
		L"Fatal Error - Yuri's Revenge", MB_OK | MB_ICONERROR);

	if(Dump) {
		Debug::FullDump(nullptr);
	}
}

void Debug::FatalError(const char *Message, ...) {
	Debug::FreeMouse();

	va_list args;
	va_start(args, Message);
	vsnprintf_s(Ares::readBuffer, Ares::readLength - 1, Message, args); /* note that the message will be truncated somewhere after 0x300 chars... */
	va_end(args);

	Debug::FatalError(false);
}

__declspec(noreturn) void Debug::FatalErrorAndExit(const char *Message, ...) {
	Debug::FreeMouse();

	va_list args;
	va_start(args, Message);
	vsnprintf_s(Ares::readBuffer, Ares::readLength - 1, Message, args); /* note that the message will be truncated somewhere after 0x300 chars... */
	va_end(args);

	Debug::FatalError(false);
	Debug::Exit();
}

void Debug::INIParseFailed(const char *section, const char *flag, const char *value, const char *Message) {
	if(Debug::bTrackParserErrors) {
		const char * LogMessage = (Message == nullptr)
			? "Failed to parse INI file content: [%s]%s=%s\n"
			: "Failed to parse INI file content: [%s]%s=%s (%s)\n"
		;

		Debug::DevLog(Debug::Warning, LogMessage, section, flag, value, Message);
	}
}

DEFINE_HOOK(4C850B, Exception_Dialog, 5)
{
	Debug::FreeMouse();
	return 0;
}

DEFINE_HOOK_AGAIN(4A4AC0, Debug_Log, 1)
DEFINE_HOOK(4068E0, Debug_Log, 1)
{
	if(Debug::bLog && Debug::pLogFile)
	{
		LEA_STACK(va_list, ArgList, 0x8);
		GET_STACK(char *, Format, 0x4);

		vfprintf(Debug::pLogFile, Format, ArgList);
		fflush(Debug::pLogFile);
	}
	return 0x4A4AF9; // changed to co-op with YDE
}

//ifdef DUMP_EXTENSIVE
DEFINE_HOOK(4C8FE0, Exception_Handler, 9)
{
	//GET(int, code, ECX);
	GET(LPEXCEPTION_POINTERS, pExs, EDX);
	Debug::ExceptionHandler(pExs);
}
//endif

DEFINE_HOOK(534A4D, Theater_Init_ResetLogStatus, 6)
{
	// any errors triggered before this line are irrelevant
	// caused by reading the section while only certain flags from it are needed
	// and before other global lists are initialized
	Debug::bTrackParserErrors = true;

	return 0;
}


DEFINE_HOOK(687C56, INIClass_ReadScenario_ResetLogStatus, 5)
{
	// reset this so next scenario startup log is cleaner
	Debug::bTrackParserErrors = false;
	return 0;
}
