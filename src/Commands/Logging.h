#ifndef CMD_LOGGING_H
#define CMD_LOGGING_H

#include "Ares.h"
#include "../Misc/Debug.h"

class LoggingCommandClass : public CommandClass
{
public:
	//Constructor
	LoggingCommandClass(){}

	//Destructor
	virtual ~LoggingCommandClass(){}

	//CommandClass
	virtual const char* GetName()
	{ return "Toggle DEBUG.TXT logging"; }

	virtual const wchar_t* GetUIName()
	{ return L"Toggle DEBUG.TXT logging"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Development"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Toggles the logging of extra data to the DEBUG.TXT log file"; }

	virtual void Execute(DWORD dwUnk)
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
