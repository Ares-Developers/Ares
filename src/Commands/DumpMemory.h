#ifndef CMD_DUMPMEM_H
#define CMD_DUMPMEM_H

#include "../Ares.h"
#include "../Misc/Debug.h"

class MemoryDumperCommandClass : public CommandClass
{
public:
	//Destructor
	virtual ~MemoryDumperCommandClass(){}

	//CommandClass
	virtual const char* GetName()
	{ return "Dump Process Memory"; }

	virtual const wchar_t* GetUIName()
	{ return L"Dump Memory"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Development"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Dumps the current process's memory"; }

	virtual void Execute(DWORD dwUnk) {
		Dialogs::TakeMouse();

		HCURSOR loadCursor = LoadCursor(nullptr, IDC_WAIT);
		SetClassLong(Game::hWnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
		SetCursor(loadCursor);

		MessageListClass::Instance->PrintMessage(L"Dumping process memory...");

		std::wstring filename;
		Debug::FullDump(nullptr, nullptr, &filename);

		Debug::Log("Process memory dumped to %ls\n", filename);

		filename = L"Process memory dumped to " + filename;

		MessageListClass::Instance->PrintMessage(filename.c_str());

		loadCursor = LoadCursor(nullptr, IDC_ARROW);
		SetClassLong(Game::hWnd, GCL_HCURSOR, reinterpret_cast<LONG>(loadCursor));
		SetCursor(loadCursor);

		Dialogs::ReturnMouse();
	}

	//Constructor
	MemoryDumperCommandClass(){}
};

#endif
