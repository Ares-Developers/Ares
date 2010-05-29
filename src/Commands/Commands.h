#ifndef ARES_COMMANDS_H
#define ARES_COMMANDS_H

#include <YRPP.h>

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
		MessageListClass::PrintMessage(L"Test Function executed!");
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
#include "Commands/DumpTypes.h"
#include "Commands/Debugging.h"
//include "Commands/Logging.h"

#endif
