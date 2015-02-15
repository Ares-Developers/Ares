#pragma once

#include "../Ext/Rules/Body.h"

#include <CommandClass.h>
#include <MapClass.h>

class TogglePowerCommandClass : public CommandClass
{
public:
	//CommandClass
	virtual const char* GetName() const override
	{
		return "TogglePower";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return L"Toggle Power Mode";
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return L"Interface";
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return L"Turn toggle power mode on / off.";
	}

	virtual void Execute(DWORD dwUnk) const override
	{
		if(RulesExt::Global()->TogglePowerAllowed) {
			MapClass::Instance->SetTogglePowerMode(-1);
		}
	}
};
