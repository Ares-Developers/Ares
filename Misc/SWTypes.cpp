#include "SWTypes.h"
#include "SWTypes/SonarPulse.h"
#include "SWTypes/UnitDelivery.h"
#include "SWTypes/GenericWarhead.h"

DynamicVectorClass<NewSWType *> NewSWType::Array;
DynamicVectorClass<SWStateMachine *> SWStateMachine::Array;

void NewSWType::Init()
{
	new SW_SonarPulse();
	new SW_UnitDelivery();
	new SW_GenericWarhead();
}

DEFINE_HOOK(55AFB3, LogicClass_Update, 6)
{
	SWStateMachine::UpdateAll();
	return 0;
}

void SWStateMachine::UpdateAll()
{
	for(int i = SWStateMachine::Array.Count - 1; i >= 0; --i) {
		SWStateMachine* Machine = SWStateMachine::Array[i];
		Machine->Update();
		if(Machine->Finished()) {
			SWStateMachine::Array.RemoveItem(i);
			delete Machine;
		}
	}
}
