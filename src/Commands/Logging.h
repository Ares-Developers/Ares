#ifndef CMD_LOGGING_H
#define CMD_LOGGING_H

#include "Ares.h"
#include "../Misc/Debug.h"

class LoggingCommandClass : public CommandClass
{
public:
	//CommandClass
	virtual const char* GetName() const override
	{ return "Toggle DEBUG.TXT logging"; }

	virtual const wchar_t* GetUIName() const override
	{ return L"Toggle DEBUG.TXT logging"; }

	virtual const wchar_t* GetUICategory() const override
		{ return L"Development"; }

	virtual const wchar_t* GetUIDescription() const override
		{ return L"Toggles the logging of extra data to the DEBUG.TXT log file"; }

	virtual void Execute(DWORD dwUnk) const override
	{
		if(Debug::bLog) {
			Debug::LogFileClose(666);
			Debug::bLog = false;
			MessageListClass::PrintMessage(L"Debug logging OFF");
		} else {
			Debug::LogFileOpen();
			Debug::bLog = true;
			MessageListClass::PrintMessage(L"Debug logging ON");
		}
	}
};

#endif
