#ifndef CMD_AICTRL_H
#define CMD_AICTRL_H

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

#endif
