#ifndef CMD_AIBASEPLAN_H
#define CMD_AIBASEPLAN_H

#include "Ares.h"
#include "../Misc/Debug.h"

#include "../Ext/House/Body.h"
#include "../Ext/HouseType/Body.h"

class AIBasePlanCommandClass : public AresCommandClass
{
public:
	//Destructor
	virtual ~AIBasePlanCommandClass(){}

	//CommandClass
	virtual const char* GetName()
		{ return "Dump AI Base Plan"; }

	virtual const wchar_t* GetUIName()
		{ return L"AI Base Plan Logger"; }

	virtual const wchar_t* GetUICategory()
		{ return L"Development"; }

	virtual const wchar_t* GetUIDescription()
		{ return L"Dumps the AI Base Plans to the log"; }

	virtual void Execute(DWORD dwUnk)
	{
		if(this->CheckDebugDeactivated()) {
			return;
		}

		Debug::Log("AI Base Plans:\n");
		for(int i = 0; i < HouseClass::Array->Count; ++i) {
			auto H = HouseClass::Array->GetItem(i);
			if(!H->ControlledByHuman()) {
				Debug::Log("#%02d: country %25s:\n", i, H->Type->ID);
				const auto& b = H->Base.BaseNodes;
				for(int j = 0; j < b.Count; ++j) {
					const auto& n = b[j];
					auto idx = n.BuildingTypeIndex;
					if(idx >= 0) {
						auto lbl = BuildingTypeClass::Array->GetItem(idx)->ID;
						Debug::Log("\tNode #%03d: %s @ (%05d, %05d), Attempts so far: %d, Placed: %d\n"
							, j, lbl, n.MapCoords.X, n.MapCoords.Y, n.Attempts, n.Placed);
					} else {
						Debug::Log("\tNode #%03d: Special %d @ (%05d, %05d), Attempts so far: %d, Placed: %d\n"
							, j, idx, n.MapCoords.X, n.MapCoords.Y, n.Attempts, n.Placed);
					}
				}
				Debug::Log("\n");
			}
		}

		MessageListClass::Instance->PrintMessage(L"Dumped AI Base Plan");
	}

	//Constructor
	AIBasePlanCommandClass(){}
};

#endif
