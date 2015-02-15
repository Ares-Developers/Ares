#pragma once

#include "Commands.h"

#include <MessageListClass.h>

class MapSnapshotCommandClass : public AresCommandClass
{
public:
	//CommandClass
	virtual const char* GetName() const override
	{
		return "MapSnapshot";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return L"Map Snapshot";
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return L"Development";
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return L"Saves the currently played map.";
	}

	virtual void Execute(DWORD dwUnk) const override
	{
		if(this->CheckDebugDeactivated()) {
			return;
		}

		char fName[0x80];

		SYSTEMTIME time;
		GetLocalTime(&time);

		_snprintf_s(fName, 0x7F, "Map.%04u%02u%02u-%02u%02u%02u-%05u.yrm",
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

		char* pBuffer = fName;

		SET_REG8(dl, 0);
		SET_REG32(ecx, pBuffer);
		CALL(0x687CE0);

		wchar_t msg[0xA0] = L"\0";
		wsprintfW(msg, L"Map Snapshot saved as '%hs'.", fName);

		MessageListClass::Instance->PrintMessage(msg);
	}
};
