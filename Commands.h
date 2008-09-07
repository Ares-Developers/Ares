#ifndef ARES_COMMANDS_H
#define ARES_COMMANDS_H

#include <YRPP.h>
#include "Commands.FrameByFrame.h"

class MapSnapshotCommandClass : public CommandClass
{
public:
	//Destructor
	virtual ~MapSnapshotCommandClass(){}

	//CommandClass
	virtual const char* GetName()
	{ return "MapSnapshot"; }

	virtual const wchar_t* GetUIName()
	{ return L"Map Snapshot"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Ares"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Saves the currently played map."; }

	virtual void Execute(DWORD dwUnk)
	{
		int i = 0;
		
		FILE* F = NULL;
		char buffer[0x10] = "\0";

		do
		{
			if(F)fclose(F);

			sprintf(buffer, "Map%04d.yrm", i++);
			F = fopen(buffer, "rb");
		}while(F != NULL);

		Ares::Log("\t\t%s", buffer);

		char* pBuffer = buffer;

		SET_REG8(dl, 0);
		SET_REG32(ecx, pBuffer);
		CALL(0x687CE0);

		wchar_t msg[0x40] = L"\0";
		wsprintf(msg, L"Map Snapshot saved as '%hs'.", buffer);
		MessageListClass::PrintMessage(msg);
	}

	//Constructor
	MapSnapshotCommandClass(){}
};

class AIControlCommandClass : public CommandClass
{
public:
	//Destructor
	virtual ~AIControlCommandClass(){}

	//CommandClass
	virtual const char* GetName()
	{ return "AIControl"; }

	virtual const wchar_t* GetUIName()
	{ return L"AI Control"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Ares"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Let the AI assume control."; }

	virtual void Execute(DWORD dwUnk)
	{
		HouseClass* P = HouseClass::Player();

		if(P->get_CurrentPlayer() && P->get_PlayerControl())
		{
			//let AI assume control
			P->set_CurrentPlayer(false);
			P->set_PlayerControl(false);
			P->set_Production(true);
			P->set_AutocreateAllowed(true);

			//give full capabilities
			P->set_IQLevel(RulesClass::Global()->get_MaxIQLevels());
			P->set_IQLevel2(RulesClass::Global()->get_MaxIQLevels());
			P->set_AIDifficulty(0);	//brutal!

			//notify
			MessageListClass::PrintMessage(L"AI assumed control!");
		}
		else
		{
			//re-assume control
			P->set_CurrentPlayer(true);
			P->set_PlayerControl(true);

			//notify
			MessageListClass::PrintMessage(L"Player assumed control!");
		}
	}

	//Constructor
	AIControlCommandClass(){}
};

class TestSomethingCommandClass : public CommandClass
{
public:
	//Destructor
	virtual ~TestSomethingCommandClass(){}

	//CommandClass
	virtual const char* GetName()
		{ return "TestSomething"; }

	virtual const wchar_t* GetUIName()
		{ return L"Test Function"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Ares"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Executes a test function."; }

	virtual void Execute(DWORD dwUnk)
	{
		MessageListClass::PrintMessage(L"Test Function executed!");
	}

	//Constructor
	TestSomethingCommandClass(){}
};

#endif
