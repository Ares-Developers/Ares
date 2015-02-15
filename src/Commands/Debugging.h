#pragma once

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
	//CommandClass
	virtual const char* GetName() const override
	{
		return "Debug Dump";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return L"Debugging Dump";
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return L"Development";
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return L"Dumps the current debug data to the log";
	}

	virtual void Execute(DWORD dwUnk) const override
	{
		// empty for now
	}
};
