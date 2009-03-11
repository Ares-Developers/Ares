#ifndef CMD_TOGGLE_FS_H
#define CMD_TOGGLE_FS_H

#include "..\Ares.h"
#include "..\Ext\House\Body.h"
#include "..\Misc\Debug.h"

class FirestormToggleCommandClass : public CommandClass
{
public:
	//Destructor
	virtual ~FirestormToggleCommandClass(){}

	//CommandClass
	virtual const char* GetName()
		{ return "Toggle Firestorm state"; }

	virtual const wchar_t* GetUIName()
		{ return L"Firestorm Toggle"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Development"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Toggles your house's Firestorm state"; }

	virtual void Execute(DWORD dwUnk)
	{
		HouseClass *H = HouseClass::Player();
		bool FS = H->FirestormActive;
		H->FirestormActive = !FS;
		wchar_t message[0x40];
		wsprintfW(message, L"Firestorm state inverted, now %d\n", !FS);
		MessageListClass::PrintMessage(message);
	}

	//Constructor
	FirestormToggleCommandClass(){}
};

#endif
