#ifndef CMD_AICTRL_H
#define CMD_AICTRL_H

class AIControlCommandClass : public AresCommandClass
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

	//Constructor
	AIControlCommandClass(){}
};

#endif
