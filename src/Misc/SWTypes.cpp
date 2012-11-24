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

#include "../Ext/TechnoType/Body.h"

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

bool NewSWType::IsLaunchSite(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const
{
	if(pBuilding->IsAlive && pBuilding->Health && !pBuilding->InLimbo && pBuilding->IsPowerOnline()) {
		return pBuilding->HasSuperWeapon(pSWType->AttachedToObject->ArrayIndex);
	}

	return false;
}

std::pair<double, double> NewSWType::GetLaunchSiteRange(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const
{
	return std::make_pair(pSWType->SW_RangeMinimum, pSWType->SW_RangeMaximum);
}

bool NewSWType::HasLaunchSite(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct &Coords) const
{
	// the quick way out: no range restriction at all
	auto range = GetLaunchSiteRange(pSWType);

	if(range.first < 0.0 && range.second < 0.0) {
		return true;
	}

	return std::any_of(pOwner->Buildings.begin(), pOwner->Buildings.end(), [&](BuildingClass* pBld) {
		return IsLaunchSiteEligible(pSWType, Coords, pBld, false);
	});
}

bool NewSWType::IsLaunchSiteEligible(SWTypeExt::ExtData* pSWType, const CellStruct &Coords, BuildingClass* pBuilding, bool ignoreRange) const
{
	if(!IsLaunchSite(pSWType, pBuilding)) {
		return false;
	}

	if(ignoreRange) {
		return true;
	}

	// get the range for this building
	auto range = GetLaunchSiteRange(pSWType, pBuilding);
	const auto& minRange = range.first;
	const auto& maxRange = range.second;

	const auto distance = Coords.DistanceFrom(pBuilding->GetCell()->MapCoords);

	// negative range values just pass the test
	return (minRange < 0.0 || distance >= minRange)
		&& (maxRange < 0.0 || distance <= maxRange);
}

bool NewSWType::IsDesignator(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, TechnoClass* pTechno) const
{
	if(pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated) {
		if(pTechno->GetOwningHouse() == pOwner) {
			auto pType = pTechno->GetTechnoType();
			return pSWType->SW_AnyDesignator || pSWType->SW_Designators.Contains(pType);
		}
	}

	return false;
}

bool NewSWType::HasDesignator(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct &Coords) const
{
	// does not require designators
	if(pSWType->SW_Designators.empty() && !pSWType->SW_AnyDesignator) {
		return true;
	}

	// a single designator in range suffices
	return FindDesignator(pSWType, pOwner, Coords, nullptr) != nullptr;
}

TechnoClass* NewSWType::FindDesignator(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct &Coords, int* memo) const
{
	// if the super weapon needs a designator, try to find one in range
	if(!pSWType->SW_Designators.empty() || pSWType->SW_AnyDesignator) {

		int start = (memo ? *memo : 0);
		for(int i=start; i<TechnoClass::Array->Count; ++i) {
			TechnoClass* pTechno = TechnoClass::Array->GetItem(i);

			// update our poor man's iterator
			if(memo) {
				*memo = i + 1;
			}

			if(IsDesignator(pSWType, pOwner, pTechno)) {
				auto pType = pTechno->GetTechnoType();
				auto pExt = TechnoTypeExt::ExtMap.Find(pType);

				// has to be closer than the designator range (which defaults to Sight)
				auto distance = Coords.DistanceFrom(pTechno->GetCell()->MapCoords);
				if(distance <= pExt->DesignatorRange.Get(pType->Sight)) {
					return pTechno;
				}
			}
		}
	}

	return nullptr;
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
