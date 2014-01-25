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

std::vector<NewSWType *> NewSWType::Array;
DynamicVectorClass<SWStateMachine *> SWStateMachine::Array;

void NewSWType::Init()
{
	if(!Array.empty()) {
		return;
	}

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
	Ares::UpdateStability();
	return 0;
}


DEFINE_HOOK(539760, Scenario_ResetAllSuperWeapons_Custom, 5)
{
	// hard-reset any super weapon related globals
	SW_LightningStorm::CurrentLightningStorm = nullptr;
	SW_NuclearMissile::CurrentNukeType = nullptr;
	SW_PsychicDominator::CurrentPsyDom = nullptr;

	for(int i = SWStateMachine::Array.Count - 1; i >= 0; --i) {
		if(SWStateMachine* pMachine = SWStateMachine::Array[i]){
			SWStateMachine::Array.RemoveItem(i);
			delete pMachine;
		}
	}

	return 0;
}

void SWStateMachine::UpdateAll()
{
	for(int i = SWStateMachine::Array.Count - 1; i >= 0; --i) {
		SWStateMachine* Machine = SWStateMachine::Array[i];

		if(!Machine) {
			Debug::FatalErrorAndExit("SWStateMachine was NULL at index %d.\n", i);
		}

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
