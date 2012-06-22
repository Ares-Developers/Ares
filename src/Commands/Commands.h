#ifndef ARES_COMMANDS_H
#define ARES_COMMANDS_H

#include <YRPP.h>
#include <StringTable.h>

class AresCommandClass : public CommandClass
{
protected:
	/**
		Adds a notification to the message list when debug key commands are disabled.

		\return true, if debug commands are deactivated, false otherwise.
		\author AlexB
		\date 2012-06-21
	*/
	bool CheckDebugDeactivated() {
		if(!Ares::GlobalControls::DebugKeysEnabled) {
			if(const wchar_t* text = StringTable::LoadStringA("TXT_COMMAND_DISABLED", NULL, __LINE__, __FILE__)) {
				wchar_t msg[0x100] = L"\0";
				wsprintfW(msg, text, this->GetUIName());
				MessageListClass::Instance->PrintMessage(msg);
			}
			return true;
		}
		return false;
	}
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
		{ return L"Development"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Executes a test function."; }

	virtual void Execute(DWORD dwUnk)
	{
		MessageListClass::Instance->PrintMessage(L"Test Function executed!");
	}

	//Constructor
	TestSomethingCommandClass(){}
};

// will the templates ever stop? :D
template <typename T>
void MakeCommand() {
	T* command;
	GAME_ALLOC(T, command);
	CommandClass::Array->AddItem(command);
};

// include other commands like this
#include "Commands/AIControl.h"
#include "Commands/MapSnapshot.h"
//include "Commands/FrameByFrame.h"
#include "Commands/AIBasePlan.h"
#include "Commands/DumpTypes.h"
#include "Commands/DumpMemory.h"
#include "Commands/Debugging.h"
//include "Commands/Logging.h"

#endif
