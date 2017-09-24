#ifndef CMD_TOGGLEPOWER_H
#define CMD_TOGGLEPOWER_H

#include "../Ext/Rules/Body.h"

#include <MapClass.h>
#include <DisplayClass.h>

class TogglePowerCommandClass : public CommandClass
{
public:
	//Destructor
	virtual ~TogglePowerCommandClass(){}

	//CommandClass
	virtual const char* GetName()
	{ return "TogglePower"; }

	virtual const wchar_t* GetUIName()
	{ return L"Toggle Power Mode"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Interface"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Turn toggle power mode on / off."; }

	virtual void Execute(DWORD dwUnk)
	{
		if(RulesExt::Global()->TogglePowerAllowed) {
			MapClass::Instance->SetTogglePowerMode(-1);
		}
	}

	//Constructor
	TogglePowerCommandClass(){}
};

#endif
