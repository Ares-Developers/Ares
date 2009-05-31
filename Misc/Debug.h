#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG_FILE "DEBUG.TXT"

#include <stdio.h>
#include <YRPPCore.h>

class Debug
{
public:
	static bool bLog;
	static FILE* pLogFile;

	static void LogFileOpen();
	static void LogFileClose();
	static void LogFileRemove();
	static void DumpObj(byte *data, size_t len);
	static void DumpStack(REGISTERS *R, size_t len);
	static void (_cdecl* Log)(const char* pFormat, ...);
};

#endif
