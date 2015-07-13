#pragma once

#include <Windows.h>
#include <Dbghelp.h>

#include <string>

class Exception
{
public:
	static const DWORD MS_VC_EXCEPTION = 0x406D1388ul;

	static LONG CALLBACK ExceptionFilter(PEXCEPTION_POINTERS pExs);
	static __declspec(noreturn) LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS pExs);

	static void FullDump(
		PMINIDUMP_EXCEPTION_INFORMATION pException = nullptr,
		std::wstring const* destinationFolder = nullptr,
		std::wstring* generatedFilename = nullptr);

	static void PrepareSnapshotDirectory(std::wstring &buffer);
};
