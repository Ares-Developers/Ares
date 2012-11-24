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
#include "SWTypes/HunterSeeker.h"
#include "SWTypes/DropPod.h"

#include <BuildingClass.h>
#include <HouseClass.h>

#include <algorithm>

std::vector<std::unique_ptr<NewSWType>> NewSWType::Array;
std::vector<std::unique_ptr<SWStateMachine>> SWStateMachine::Array;

void NewSWType::Init()
{
	if(!Array.empty()) {
		return;
	}

	Register(std::make_unique<SW_SonarPulse>());
	Register(std::make_unique<SW_UnitDelivery>());
	Register(std::make_unique<SW_GenericWarhead>());
	Register(std::make_unique<SW_Firewall>());
	Register(std::make_unique<SW_Protect>());
	Register(std::make_unique<SW_Reveal>());
	Register(std::make_unique<SW_ParaDrop>());
	Register(std::make_unique<SW_SpyPlane>());
	Register(std::make_unique<SW_ChronoSphere>());
	Register(std::make_unique<SW_ChronoWarp>());
	Register(std::make_unique<SW_GeneticMutator>());
	Register(std::make_unique<SW_PsychicDominator>());
	Register(std::make_unique<SW_LightningStorm>());
	Register(std::make_unique<SW_NuclearMissile>());
	Register(std::make_unique<SW_HunterSeeker>());
	Register(std::make_unique<SW_DropPod>());
}

bool NewSWType::HasLaunchSite(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct &Coords) const
{
	// the quick way out: indefinite range, no range restriction
	if(pSWType->SW_RangeMaximum == 0.0 && pSWType->SW_RangeMinimum == 0.0) {
		return true;
	}

	return FindLaunchSite(pSWType, pOwner, Coords, false, nullptr) != nullptr;
}

BuildingClass* NewSWType::FindLaunchSite(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct &Coords, bool ignoreRange, int* pMemo) const
{
	// find an appropriate building from memorized start
	int start = (pMemo ? *pMemo : 0);

	for(int i = start; i < pOwner->Buildings.Count; ++i) {
		auto pBld = pOwner->Buildings.GetItem(i);

		// update our poor man's iterator
		if(pMemo) {
			*pMemo = i + 1;
		}

		// if this is an appropriate building, return it
		if(IsLaunchSite(pSWType, pBld)) {
			if(ignoreRange || IsLaunchSiteInRange(pSWType, Coords, pBld)) {
				return pBld;
			}
		}
	}

	return nullptr;
}

bool NewSWType::IsLaunchSite(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const
{
	if(pBuilding->IsAlive && pBuilding->Health && !pBuilding->InLimbo && pBuilding->IsPowerOnline()) {
		return pBuilding->Type->HasSuperWeapon(pSWType->AttachedToObject->ArrayIndex);
	}

	return false;
}

bool NewSWType::IsLaunchSiteInRange(SWTypeExt::ExtData* pSWType, const CellStruct &Coords, BuildingClass* pBuilding) const
{
	// check the default ranges
	return IsLaunchSiteInRange(pSWType, Coords, pBuilding, pSWType->SW_RangeMinimum, pSWType->SW_RangeMaximum);
}

bool NewSWType::IsLaunchSiteInRange(SWTypeExt::ExtData* pSWType, const CellStruct &Coords, BuildingClass* pBuilding, double minRange, double maxRange) const
{
	double distance = Coords.DistanceFrom(pBuilding->GetCell()->MapCoords);

	// no max range, or below
	if(maxRange < 0.0 || distance <= maxRange) {

		// no min range, or above
		if(minRange < 0.0 || distance >= minRange) {
			return true;
		}
	}

	return false;
}

int NewSWType::FindIndex(const char* pType) {
	auto it = std::find_if(Array.begin(), Array.end(), [pType](const std::unique_ptr<NewSWType> &item) {
		const char* pID = item->GetTypeString();
		return pID && !strcmp(pID, pType);
	});

	if(it != Array.end()) {
		return FIRST_SW_TYPE + std::distance(Array.begin(), it);
	}

	return -1;
}

int NewSWType::FindHandler(int Type) {
	auto it = std::find_if(Array.begin(), Array.end(), [Type](const std::unique_ptr<NewSWType> &item) {
		return item->HandlesType(Type);
	});

	if(it != Array.end()) {
		return FIRST_SW_TYPE + std::distance(Array.begin(), it);
	}

	return -1;
}

void SWStateMachine::UpdateAll()
{
	for(auto& Machine : SWStateMachine::Array) {
		Machine->Update();
	}

	Array.erase(std::remove_if(Array.begin(), Array.end(), [](const std::unique_ptr<SWStateMachine> &ptr) {
		return ptr->Finished();
	}), Array.end());
}

void SWStateMachine::PointerGotInvalid(void *ptr, bool remove)
{
	for(auto& Machine : SWStateMachine::Array) {
		Machine->InvalidatePointer(ptr, remove);
	}
}

void SWStateMachine::Clear()
{
	// hard-reset any super weapon related globals
	SW_LightningStorm::CurrentLightningStorm = nullptr;
	SW_NuclearMissile::CurrentNukeType = nullptr;
	SW_PsychicDominator::CurrentPsyDom = nullptr;

	SWStateMachine::Array.clear();
}

DEFINE_HOOK(55AFB3, LogicClass_Update, 6)
{
	SWStateMachine::UpdateAll();
	Ares::UpdateStability();
	return 0;
}
