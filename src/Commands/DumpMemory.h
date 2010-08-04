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

		HCURSOR loadCursor = LoadCursor(NULL, IDC_WAIT);
		SetClassLong(Game::hWnd, GCL_HCURSOR, (LONG)loadCursor);
		SetCursor(loadCursor);

		MessageListClass::Instance->PrintMessage(L"Dumping process memory...");

		wchar_t filename[MAX_PATH];
		Debug::FullDump(NULL, filename);

		const size_t len = MAX_PATH + 0x20;
		wchar_t Message[len];
		_snwprintf(Message, len, L"Process memory dumped to %s", filename);

		MessageListClass::Instance->PrintMessage(Message);
		Debug::Log("Process memory dumped to %ls\n", filename);

		loadCursor = LoadCursor(NULL, IDC_ARROW);
		SetClassLong(Game::hWnd, GCL_HCURSOR, (LONG)loadCursor);
		SetCursor(loadCursor);

		Dialogs::ReturnMouse();
	}

	//Constructor
	MemoryDumperCommandClass(){}
};

#endif
