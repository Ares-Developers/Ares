#pragma once

#include "Ares.h"
#include "../Misc/Debug.h"

#include <CommandClass.h>

class FPSCounterCommandClass : public CommandClass
{
public:
	//CommandClass
	virtual const char* GetName() const override
	{
		return "FPS Counter";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return L"FPS Counter";
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return L"Development";
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return L"Shows the current and an average of frames per second.";
	}

	virtual void Execute(DWORD dwUnk) const override
	{
		Ares::bFPSCounter = !Ares::bFPSCounter;
	}
};
