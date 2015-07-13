#pragma once

#include "Commands.h"

#include "../Misc/Debug.h"
#include "../UI/Dialogs.h"

#include <MessageListClass.h>

#include <string>

class MemoryDumperCommandClass : public CommandClass
{
public:
	//CommandClass
	virtual const char* GetName() const override
	{
		return "Dump Process Memory";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return L"Dump Memory";
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return L"Development";
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return L"Dumps the current process's memory";
	}

	virtual void Execute(DWORD dwUnk) const override
	{
		Dialogs::TakeMouse();

		HCURSOR loadCursor = LoadCursor(nullptr, IDC_WAIT);
		SetClassLong(Game::hWnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
		SetCursor(loadCursor);

		MessageListClass::Instance->PrintMessage(L"Dumping process memory...");

		std::wstring filename = Debug::FullDump();

		Debug::Log("Process memory dumped to %ls\n", filename);

		filename = L"Process memory dumped to " + filename;

		MessageListClass::Instance->PrintMessage(filename.c_str());

		loadCursor = LoadCursor(nullptr, IDC_ARROW);
		SetClassLong(Game::hWnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
		SetCursor(loadCursor);

		Dialogs::ReturnMouse();
	}
};
