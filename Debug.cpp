#include "Debug.h"

bool Debug::bLog = true;
FILE* Debug::pLogFile = NULL;

void (_cdecl* Debug::Log)(const char* pFormat, ...) =
	(void (__cdecl *)(const char *,...))0x4068E0;

void Debug::LogFileOpen()
{
	LogFileClose();
	pLogFile = fopen(DEBUG_FILE, "w");
}

void Debug::LogFileClose()
{
	if(pLogFile)
		fclose(pLogFile);

	pLogFile = NULL;
}

void Debug::LogFileRemove()
{
	LogFileClose();
	remove(DEBUG_FILE);
}

//Hook at 0x4068E0 AND 4A4AC0
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
