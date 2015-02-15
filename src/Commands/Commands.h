#pragma once

#include "../Ares.h"

#include <YRPP.h>
#include <CommandClass.h>
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
	bool CheckDebugDeactivated() const {
		if(!Ares::GlobalControls::DebugKeysEnabled) {
			if(const wchar_t* text = StringTable::LoadStringA("TXT_COMMAND_DISABLED")) {
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
	//CommandClass
	virtual const char* GetName() const override
		{ return "TestSomething"; }

	virtual const wchar_t* GetUIName() const override
		{ return L"Test Function"; }

	virtual const wchar_t* GetUICategory() const override
		{ return L"Development"; }

	virtual const wchar_t* GetUIDescription() const override
		{ return L"Executes a test function."; }

	virtual void Execute(DWORD dwUnk) const override
	{
		MessageListClass::Instance->PrintMessage(L"Test Function executed!");
	}
};

// will the templates ever stop? :D
template <typename T>
void MakeCommand() {
	T* command = GameCreate<T>();
	CommandClass::Array->AddItem(command);
};
