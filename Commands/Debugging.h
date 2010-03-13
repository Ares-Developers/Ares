#ifndef CMD_DEBUGGING_H
#define CMD_DEBUGGING_H

#include "Ares.h"
#include "../Misc/Debug.h"

#include "../Ext/Techno/Body.h"
#include "../Ext/TechnoType/Body.h"
#include "../Ext/House/Body.h"
#include "../Ext/HouseType/Body.h"
#include "../Ext/WeaponType/Body.h"
#include "../Ext/WarheadType/Body.h"

class DebuggingCommandClass : public CommandClass
{
public:
	//Destructor
	virtual ~DebuggingCommandClass(){}

	//CommandClass
	virtual const char* GetName()
	{ return "Debug Dump"; }

	virtual const wchar_t* GetUIName()
	{ return L"Debugging Dump"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Development"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Dumps the current debug data to the log"; }

	virtual void Execute(DWORD dwUnk)
	{
/*
		Debug::Log("Total memory consumed: 0x%X bytes\n", MemMap::Total);

		wchar_t msg[0x40] = L"/0";
		wsprintfW(msg, L"Total memory consumed: 0x%X bytes\n", MemMap::Total);
		MessageListClass::PrintMessage(msg);
*/

/*
		Ares::TrackIndex = (Ares::TrackIndex + 1) % 72;
		wchar_t msg[0x40] = L"\0";
		wsprintfW(msg, L"TrackIndex: %d\n", Ares::TrackIndex);

		MessageListClass::PrintMessage(msg);
*/
	}

	//Constructor
	DebuggingCommandClass(){}
};

#endif
