#include "SWTypes.h"
#include "SWTypes/SonarPulse.h"
#include "SWTypes/UnitDelivery.h"
#include "SWTypes/GenericWarhead.h"
#include "SWTypes/Firewall.h"
#include "SWTypes/Protect.h"
#include "SWTypes/Reveal.h"
#include "SWTypes/ParaDrop.h"
#include "SWTypes/SpyPlane.h"
#include "SWTypes/ChronoSphere.h"
#include "SWTypes/ChronoWarp.h"
#include "SWTypes/GeneticMutator.h"
#include "SWTypes/Dominator.h"
#include "SWTypes/LightningStorm.h"
#include "SWTypes/Nuke.h"

DynamicVectorClass<NewSWType *> NewSWType::Array;
DynamicVectorClass<SWStateMachine *> SWStateMachine::Array;

void NewSWType::Init()
{
	new SW_SonarPulse();
	new SW_UnitDelivery();
	new SW_GenericWarhead();
	new SW_Firewall();
	new SW_Protect();
	new SW_Reveal();
	new SW_ParaDrop();
	new SW_SpyPlane();
	new SW_ChronoSphere();
	new SW_ChronoWarp();
	new SW_GeneticMutator();
	new SW_PsychicDominator();
	new SW_LightningStorm();
	new SW_NuclearMissile();
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

void SWStateMachine::InvalidatePointer(void *ptr)
{
	for(int i = SWStateMachine::Array.Count - 1; i >= 0; --i) {
		SWStateMachine* Machine = SWStateMachine::Array[i];
		Machine->PointerGotInvalid(ptr);
	}
}
