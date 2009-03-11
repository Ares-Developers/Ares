#ifndef CMD_DUMPFACT_H
#define CMD_DUMPFACT_H

#include "Ares.h"
#include "Debug.h"

class DumperFactoryCommandClass : public CommandClass
{
public:
	//Destructor
	virtual ~DumperFactoryCommandClass(){}

	//CommandClass
	virtual const char* GetName()
	{ return "Dump Factories"; }

	virtual const wchar_t* GetUIName()
	{ return L"Dump Factories"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Development"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Dumps the current factory debug data to the log"; }

	virtual void Execute(DWORD dwUnk)
	{
		Debug::Log("Dumping all Factories\n\n");
		for(int i = 0; i < FactoryClass::Array->get_Count(); ++i)
		{
			FactoryClass *F = FactoryClass::Array->GetItem(i);
			if(F)
			{
				Debug::Log("Logging Factory, owned by %s\n", F->get_Owner()->get_Type()->get_ID());

				TechnoClass *T = F->get_InProduction();
				Debug::Log("Current Production: %s\n", T ? T->GetType()->get_ID() : "NULL");

				DynamicVectorClass<TechnoTypeClass*> * Q = F->get_QueuedObjects();
				for(int j = 0; j < Q->get_Count(); ++j)
				{
					Debug::Log("\tQueue #%d is %s\n", j, Q->GetItem(j)->get_ID());
				}
			}
		}
		Debug::Log("All Factories dumped\n\n");

		MessageListClass::PrintMessage(L"Factory data dumped");
	}

	//Constructor
	DumperFactoryCommandClass(){}
};

#endif
