#pragma once

#include "Commands.h"

#include <HouseClass.h>
#include <MessageListClass.h>
#include <RulesClass.h>

class AIControlCommandClass : public AresCommandClass
{
public:
	//CommandClass
	virtual const char* GetName() const override
	{
		return "AIControl";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return L"AI Control";
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return L"Ares";
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return L"Let the AI assume control.";
	}

	virtual void Execute(DWORD dwUnk) const override
	{
		if(this->CheckDebugDeactivated()) {
			return;
		}

		HouseClass* pPlayer = HouseClass::Player;

		if(pPlayer->CurrentPlayer && pPlayer->PlayerControl) {
			//let AI assume control
			pPlayer->CurrentPlayer = pPlayer->PlayerControl = false;
			pPlayer->Production = pPlayer->AutocreateAllowed = true;

			//give full capabilities
			pPlayer->IQLevel = RulesClass::Global()->MaxIQLevels;
			pPlayer->IQLevel2 = RulesClass::Global()->MaxIQLevels;
			pPlayer->AIDifficulty = AIDifficulty::Hard;	//brutal!

			//notify
			MessageListClass::Instance->PrintMessage(L"AI assumed control!");

		} else {
			//re-assume control
			pPlayer->CurrentPlayer = pPlayer->PlayerControl = true;
			pPlayer->Production = pPlayer->AutocreateAllowed = false;

			//make it a vegetable
			pPlayer->IQLevel = 0;
			pPlayer->IQLevel2 = 0;
			pPlayer->AIDifficulty = AIDifficulty::Normal;

			//notify
			MessageListClass::Instance->PrintMessage(L"Player assumed control!");
		}
	}
};
