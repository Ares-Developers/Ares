#include "Debug.h"

#include "Exception.h"
#include "..\Ares.h"

#include <MouseClass.h>
#include <Unsorted.h>
#include <WWMouseClass.h>

#include <Dbghelp.h>

bool Debug::bLog = true;
bool Debug::bTrackParserErrors = false;
bool Debug::bParserErrorDetected = false;

FILE* Debug::LogFile = nullptr;
std::wstring Debug::LogFileName;
std::wstring Debug::LogFileTempName;

const char* Debug::SeverityString(Debug::Severity const severity) {
	switch(severity) {
	case Severity::Verbose:
		return "verbose";
	case Severity::Notice:
		return "notice";
	case Severity::Warning:
		return "warning";
	case Severity::Error:
		return "error";
	case Severity::Fatal:
		return "fatal";
	default:
		return "wtf";
	}
}

void Debug::LogFlushed(
	Debug::Severity const severity, const char* const pFormat, ...)
{
	if(Debug::LogFileActive()) {
		if(severity != Severity::None) {
			Debug::LogUnflushed(
				"[Developer %s]", Debug::SeverityString(severity));
		}
		va_list args;
		va_start(args, pFormat);
		Debug::LogWithVArgs(pFormat, args);
		va_end(args);
	}
}

void Debug::LogFlushed(const char* const pFormat, ...)
{
	va_list args;
	va_start(args, pFormat);
	Debug::LogWithVArgs(pFormat, args);
	va_end(args);
}

void Debug::LogWithVArgs(const char* const pFormat, va_list const args) {
	if(Debug::LogFileActive()) {
		Debug::LogWithVArgsUnflushed(pFormat, args);
		Debug::Flush();
	}
}

void __cdecl Debug::LogUnflushed(const char* const pFormat, ...) {
	va_list args;
	va_start(args, pFormat);
	Debug::LogWithVArgsUnflushed(pFormat, args);
	va_end(args);
}

void Debug::LogWithVArgsUnflushed(
	const char* const pFormat, va_list const args)
{
	vfprintf(Debug::LogFile, pFormat, args);
}

void Debug::Flush() {
	fflush(Debug::LogFile);
}

void Debug::LogFileOpen()
{
	Debug::MakeLogFile();
	Debug::LogFileClose(999);

	LogFile = _wfsopen(Debug::LogFileTempName.c_str(), L"w", _SH_DENYWR);
	if(!LogFile) {
		wchar_t msg[100] = L"\0";
		wsprintfW(msg, L"Log file failed to open. Error code = %X", errno);
		MessageBoxW(Game::hWnd, Debug::LogFileTempName.c_str(), msg, MB_OK | MB_ICONEXCLAMATION);
		ExitProcess(1);
	}
}

void Debug::LogFileClose(int tag)
{
	if(Debug::LogFile) {
		fprintf(Debug::LogFile, "Closing log file on request %d", tag);
		fclose(Debug::LogFile);
		CopyFileW(Debug::LogFileTempName.c_str(), Debug::LogFileName.c_str(), FALSE);
		Debug::LogFile = nullptr;
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

		Debug::LogFileTempName = Debug::LogFileName + L"\\debug.log";
		Debug::LogFileName += subpath;

		made = 1;
	}
}

void Debug::LogFileRemove()
{
	Debug::LogFileClose(555);
	DeleteFileW(Debug::LogFileTempName.c_str());
}

void Debug::DumpObj(void const* const data, size_t const len) {
	if(!Debug::LogFileActive()) {
		return;
	}
	Debug::LogUnflushed("Dumping %u bytes of object at %p\n", len, data);
	auto const bytes = static_cast<byte const*>(data);

	Debug::LogUnflushed("       |");
	for(auto i = 0u; i < 0x10u; ++i) {
		Debug::LogUnflushed(" %02X |", i);
	}
	Debug::LogUnflushed("\n");
	Debug::LogUnflushed("-------|");
	for(auto i = 0u; i < 0x10u; ++i) {
		Debug::LogUnflushed("----|", i);
	}
	auto const bytesToPrint = (len + 0x10 - 1) / 0x10 * 0x10;
	for(auto startRow = 0u; startRow < bytesToPrint; startRow += 0x10) {
		Debug::LogUnflushed("\n");
		Debug::LogUnflushed(" %05X |", startRow);
		auto const bytesInRow = std::min(len - startRow, 0x10u);
		for(auto i = 0u; i < bytesInRow; ++i) {
			Debug::LogUnflushed(" %02X |", bytes[startRow + i]);
		}
		for(auto i = bytesInRow; i < 0x10u; ++i) {
			Debug::LogUnflushed(" -- |");
		}
		for(auto i = 0u; i < bytesInRow; ++i) {
			auto const& sym = bytes[startRow + i];
			Debug::LogUnflushed("%c", isprint(sym) ? sym : '?');
		}
	}
	Debug::Log("\nEnd of dump.\n"); // flushes
}

void Debug::DumpStack(REGISTERS* R, size_t len, int startAt) {
	if(!Debug::LogFileActive()) {
		return;
	}
	Debug::LogUnflushed("Dumping %X bytes of stack\n", len);
	auto const end = len / 4;
	auto const* const mem = R->lea_Stack<DWORD*>(startAt);
	for(auto i = 0u; i < end; ++i) {
		Debug::LogUnflushed("esp+%04X = %08X\n", i * 4, mem[i]);
	}
	Debug::Log("Done.\n"); // flushes
}

/**
 * minidump
 */

std::wstring Debug::FullDump() {
	return Exception::FullDump();
}

std::wstring Debug::FullDump(std::wstring destinationFolder) {
	return Exception::FullDump(std::move(destinationFolder));
}

void Debug::FreeMouse() {
//	static bool freed = false;
//	if(!freed) {
		Game::sub_53E6B0();

		MouseClass::Instance->UpdateCursor(MouseCursorType::Default, false);
		WWMouseClass::Instance->ReleaseMouse();

		ShowCursor(TRUE);

		auto const BlackSurface = [](DSurface* const pSurface) {
			if(pSurface) {
				pSurface->Fill(0);
			}
		};

		BlackSurface(DSurface::Alternate);
		BlackSurface(DSurface::Composite);
		BlackSurface(DSurface::Hidden);
		BlackSurface(DSurface::Hidden_2);
		BlackSurface(DSurface::Primary);
		BlackSurface(DSurface::Sidebar);
		BlackSurface(DSurface::Tile);

		ShowCursor(TRUE);

//		DirectDrawWrap::DisposeOfStuff();
//		DirectDrawWrap::lpDD->SetCooperativeLevel(Game::hWnd, eDDCoopLevel::DDSCL_NORMAL);
//		DirectDrawWrap::lpDD->SetDisplayMode(800, 600, 16);

//		freed = true;
//	}
}

void Debug::FatalError(bool Dump) {
	wchar_t Message[0x400];
	wsprintfW(Message,
		L"An internal error has been encountered and the game is unable to continue normally. "
		L"Please notify the mod's creators about this issue, or visit our website at "
		L"http://ares.strategy-x.com for updates and support.\n\n"
		L"%hs",
		Ares::readBuffer);

	Debug::Log("\nFatal Error:\n");
	Debug::Log("%s\n", Ares::readBuffer);

	MessageBoxW(Game::hWnd, Message, L"Fatal Error - Yuri's Revenge", MB_OK | MB_ICONERROR);

	if(Dump) {
		Debug::FullDump();
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

[[noreturn]] void Debug::FatalErrorAndExit(const char *Message, ...) {
	Debug::FreeMouse();

	va_list args;
	va_start(args, Message);
	vsnprintf_s(Ares::readBuffer, Ares::readLength - 1, Message, args); /* note that the message will be truncated somewhere after 0x300 chars... */
	va_end(args);

	Debug::FatalError(false);
	Exception::Exit();
}

void Debug::INIParseFailed(const char *section, const char *flag, const char *value, const char *Message) {
	if(Debug::bTrackParserErrors) {
		const char * LogMessage = (Message == nullptr)
			? "Failed to parse INI file content: [%s]%s=%s\n"
			: "Failed to parse INI file content: [%s]%s=%s (%s)\n"
		;

		Debug::Log(Debug::Severity::Warning, LogMessage, section, flag, value, Message);
		Debug::RegisterParserError();
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
	LEA_STACK(va_list const, args, 0x8);
	GET_STACK(const char* const, pFormat, 0x4);

	Debug::LogWithVArgs(pFormat, args);

	return 0x4A4AF9; // changed to co-op with YDE
}

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
