#pragma once

#include <Windows.h>
#include <Dbghelp.h>

#include <string>

class Exception
{
public:
	static const DWORD MS_VC_EXCEPTION = 0x406D1388ul;

	static LONG CALLBACK ExceptionFilter(PEXCEPTION_POINTERS pExs);
	[[noreturn]] static LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS pExs);

	static std::wstring FullDump(PMINIDUMP_EXCEPTION_INFORMATION pException = nullptr);
	static std::wstring FullDump(std::wstring destinationFolder,
		PMINIDUMP_EXCEPTION_INFORMATION pException = nullptr);

	static std::wstring PrepareSnapshotDirectory();

	[[noreturn]] static void Exit(UINT ExitCode = 1u);
};
