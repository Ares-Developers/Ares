#pragma once

#include "Commands.h"

#include <CommandClass.h>
#include <MessageListClass.h>

// toggle
class FrameByFrameCommandClass : public CommandClass
{
public:
	//CommandClass
	virtual const char* GetName() const override
	{
		return "SingleStep";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return L"Toggle Single-stepping";
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return L"Development";
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return L"Toggles the single-stepping debug mode on or off.";
	}

	virtual void Execute(DWORD dwUnk) const override
	{
		Unsorted::ArmageddonMode = !Unsorted::ArmageddonMode;

		wchar_t msg[0x40] = L"\0";
		wsprintfW(msg, L"Single stepping mode %s.", Unsorted::ArmageddonMode ? L"enabled" : L"disabled");
		MessageListClass::Instance->PrintMessage(msg);
	}
};

// step
class FrameStepCommandClass : public CommandClass
{
public:
	//CommandClass
	virtual const char* GetName() const override
	{
		return "SingleStepForward";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return L"Single Step";
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return L"Ares";
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return L"Proceeds a single step forward.";
	}

	static int ArmageddonState;

	virtual void Execute(DWORD dwUnk) const override
	{
		Unsorted::ArmageddonMode = 0;
		ArmageddonState = 1;

		MessageListClass::Instance->PrintMessage(L"Stepping.");
	}
};
