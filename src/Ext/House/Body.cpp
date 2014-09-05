#include "Body.h"
#include "../HouseType/Body.h"
#include "../Building/Body.h"
#include "../BuildingType/Body.h"
#include "../Rules/Body.h"
#include "../Side/Body.h"
#include "../TechnoType/Body.h"
#include "../../Enum/Prerequisites.h"
#include "../Techno/Body.h"

#include <FactoryClass.h>
#include <HouseClass.h>
#include <MouseClass.h>
#include <SuperClass.h>

template<> const DWORD Extension<HouseClass>::Canary = 0x12345678;
Container<HouseExt> HouseExt::ExtMap;
bool HouseExt::IsAnyFirestormActive = false;

template<> HouseExt::TT *Container<HouseExt>::SavingObject = nullptr;
template<> IStream *Container<HouseExt>::SavingStream = nullptr;

// =============================
// member funcs

HouseExt::RequirementStatus HouseExt::RequirementsMet(HouseClass *pHouse, TechnoTypeClass *pItem)
{
	if(pItem->Unbuildable) {
		return Forbidden;
	}

	TechnoTypeExt::ExtData* pData = TechnoTypeExt::ExtMap.Find(pItem);
	if(!pItem) {
		return Forbidden;
	}
//	TechnoTypeClassExt::TechnoTypeClassData *pData = TechnoTypeClassExt::Ext_p[pItem];
	HouseExt::ExtData* pHouseExt = HouseExt::ExtMap.Find(pHouse);

	// this has to happen before the first possible "can build" response or NCO happens
	if(pItem->WhatAmI() != abs_BuildingType && !FactoryForObjectExists(pHouse, pItem)) { return Incomplete; }

	if(!(pData->PrerequisiteTheaters & (1 << ScenarioClass::Instance->Theater))) { return Forbidden; }
	if(Prereqs::HouseOwnsAny(pHouse, &pData->PrerequisiteNegatives)) { return Forbidden; }

	if(pData->ReversedByHouses.contains(pHouse)) {
		return Overridden;
	}

	if(pData->RequiredStolenTech.any()) {
		if((pHouseExt->StolenTech & pData->RequiredStolenTech) != pData->RequiredStolenTech) { return Incomplete; }
	}

	// yes, the game checks it here
	// hack value - skip real prereq check
	if(Prereqs::HouseOwnsAny(pHouse, &pItem->PrerequisiteOverride)) { return Overridden; }

	if(pHouse->HasFromSecretLab(pItem)) { return Overridden; }

	if(pHouse->ControlledByHuman() && pItem->TechLevel == -1) { return Incomplete; }

	if(!pHouse->HasAllStolenTech(pItem)) { return Incomplete; }

	if(!pHouse->InRequiredHouses(pItem) || pHouse->InForbiddenHouses(pItem)) { return Forbidden; }

	if(!HouseExt::CheckFactoryOwners(pHouse, pItem)) { return Incomplete; }

	if(!Unsorted::SWAllowed) {
		if(BuildingTypeClass *pBld = specific_cast<BuildingTypeClass*>(pItem)) {
			if(pBld->SuperWeapon != -1) {
				if(RulesClass::Instance->BuildTech.FindItemIndex(pBld) == -1) {
					if(pHouse->Supers.GetItem(pBld->SuperWeapon)->Type->DisableableFromShell) {
						return Forbidden;
					}
				}
			}
		}
	}

	return (pHouse->TechLevel >= pItem->TechLevel) ? Complete : Incomplete;
}

bool HouseExt::PrerequisitesMet(HouseClass *pHouse, TechnoTypeClass *pItem)
{
	if(!pItem) {
		return 0;
	}
	TechnoTypeExt::ExtData* pData = TechnoTypeExt::ExtMap.Find(pItem);

	for(size_t i = 0; i < pData->PrerequisiteLists.size(); ++i) {
		if(Prereqs::HouseOwnsAll(pHouse, pData->PrerequisiteLists[i].get())) {
			return 1;
		}
	}

	return 0;
}

bool HouseExt::PrerequisitesListed(const Prereqs::BTypeIter &List, TechnoTypeClass *pItem)
{
	if(!pItem) {
		return 0;
	}
	TechnoTypeExt::ExtData* pData = TechnoTypeExt::ExtMap.Find(pItem);

	for(size_t i = 0; i < pData->PrerequisiteLists.size(); ++i) {
		if(Prereqs::ListContainsAll(List, pData->PrerequisiteLists[i].get())) {
			return 1;
		}
	}

	return 0;
}

HouseExt::BuildLimitStatus HouseExt::CheckBuildLimit(HouseClass *pHouse, TechnoTypeClass *pItem, bool IncludeQueued)
{
	int BuildLimit = pItem->BuildLimit;
	int Remaining = HouseExt::BuildLimitRemaining(pHouse, pItem);
	if(BuildLimit > 0) {
		if(Remaining <= 0) {
			return (IncludeQueued && FactoryClass::FindThisOwnerAndProduct(pHouse, pItem))
				? NotReached
				: ReachedPermanently
			;
		}
	}
	return (Remaining > 0)
		? NotReached
		: ReachedTemporarily
	;
}

signed int HouseExt::BuildLimitRemaining(HouseClass *pHouse, TechnoTypeClass *pItem)
{
	int BuildLimit = pItem->BuildLimit;
	if(BuildLimit >= 0) {
		BuildLimit -= pHouse->CountOwnedNowTotal(pItem);
	} else {
		BuildLimit = std::abs(BuildLimit);
		BuildLimit -= pHouse->CountOwnedEver(pItem);
	}
	return std::min(BuildLimit, 0x7FFFFFFF);
}

signed int HouseExt::PrereqValidate
	(HouseClass *pHouse, TechnoTypeClass *pItem, bool BuildLimitOnly, bool IncludeQueued)
{
	if(!BuildLimitOnly) {
		RequirementStatus ReqsMet = HouseExt::RequirementsMet(pHouse, pItem);
		if(ReqsMet == Forbidden || ReqsMet == Incomplete) {
			return 0;
		}

		if(!pHouse->ControlledByHuman()) {
			if(Ares::GlobalControls::AllowBypassBuildLimit[pHouse->AIDifficulty]) {
				return 1;
			} else {
				return static_cast<signed int>(HouseExt::CheckBuildLimit(pHouse, pItem, IncludeQueued));
			}
		}

		if(ReqsMet == Complete) {
			if(!HouseExt::PrerequisitesMet(pHouse, pItem)) {
				return 0;
			}
		}
	}

	if(!HouseExt::HasNeededFactory(pHouse, pItem)) {
		Debug::DevLog(Debug::Error, "[NCO Bug detected] "
			"House %ls meets all requirements to build %s, but doesn't have a suitable factory!\n",
			pHouse->UIName, pItem->ID);
		return 0;
	}

	return static_cast<signed int>(HouseExt::CheckBuildLimit(pHouse, pItem, IncludeQueued));
}

bool HouseExt::HasNeededFactory(HouseClass *pHouse, TechnoTypeClass *pItem) {
	DWORD ItemOwners = pItem->GetOwners();
	eAbstractType WhatAmI = pItem->WhatAmI();

	for(int i = 0; i < pHouse->Buildings.Count; ++i) {
		auto pBld = pHouse->Buildings[i];
		if(!pBld->InLimbo && pBld->HasPower) {
			if(pBld->Type->Factory == WhatAmI) {
				if(pBld->GetCurrentMission() != Mission::Selling && pBld->QueuedMission != Mission::Selling) {
					if((pBld->Type->GetOwners() & ItemOwners) != 0) {
						return true;
					}
				}
			}
		}
	}

	return false;
}

// this only verifies the existence, it does not check whether the building is currently
// in a state that allows it to kick out units. however, it respects BuiltAt.
bool HouseExt::FactoryForObjectExists(HouseClass *pHouse, TechnoTypeClass *pItem) {
	eAbstractType WhatAmI = pItem->WhatAmI();
	auto pExt = TechnoTypeExt::ExtMap.Find(pItem);

	for(int i = 0; i < pHouse->Buildings.Count; ++i) {
		BuildingTypeClass *pType = pHouse->Buildings[i]->Type;
		if(pType->Factory == WhatAmI
			&& pType->Naval == pItem->Naval
			&& pExt->CanBeBuiltAt(pType)) {
			return true;
		}
	}
	return false;
}

bool HouseExt::CheckFactoryOwners(HouseClass *pHouse, TechnoTypeClass *pItem) {
	return HouseExt::CheckFactoryOwner(pHouse, pItem)
		&& HouseExt::CheckForbiddenFactoryOwner(pHouse, pItem);
}

bool HouseExt::CheckFactoryOwner(HouseClass *pHouse, TechnoTypeClass *pItem){
	auto pExt = TechnoTypeExt::ExtMap.Find(pItem);
	auto HouseExt = HouseExt::ExtMap.Find(pHouse);

	if (!pExt->FactoryOwners.empty()) {
		for (auto pOwner : pExt->FactoryOwners) {
			if(HouseExt->FactoryOwners_GatheredPlansOf.Contains(pOwner)) {
				return true;
			}
		}

		eAbstractType WhatAmI = pItem->WhatAmI();

		for (auto pBld : pHouse->Buildings) {
			if (pBld->Type->Factory == WhatAmI) {
				auto FactoryExt = TechnoExt::ExtMap.Find(pBld);

				if(pExt->FactoryOwners.Contains(FactoryExt->OriginalHouseType)) {
					return true;
				}
			}
		}

		return false;
	}

	return true;
}

bool HouseExt::CheckForbiddenFactoryOwner(HouseClass *pHouse, TechnoTypeClass *pItem){
	auto pExt = TechnoTypeExt::ExtMap.Find(pItem);
	auto HouseExt = HouseExt::ExtMap.Find(pHouse);

	auto &forbidden = pExt->ForbiddenFactoryOwners;

	if (!forbidden.empty()) {
		// return true if not a single forbidden house is in the gathered plans (only if there are any)
		if(HouseExt->FactoryOwners_GatheredPlansOf.size()) {
			if(!std::any_of(forbidden.begin(), forbidden.end(), [HouseExt](HouseTypeClass* pForbidden) {
				return HouseExt->FactoryOwners_GatheredPlansOf.Contains(pForbidden);
			}))	{
				return true;
			}
		}

		eAbstractType WhatAmI = pItem->WhatAmI();

		for (auto pBld : pHouse->Buildings) {
			if (pBld->Type->Factory == WhatAmI) {
				auto FactoryExt = TechnoExt::ExtMap.Find(pBld);

				if(!forbidden.Contains(FactoryExt->OriginalHouseType)) {
					return true;
				}
			}
		}

		return false;
	}

	return true;
}

bool HouseExt::UpdateAnyFirestormActive() {
	IsAnyFirestormActive = false;

	for(int i = 0; i < HouseClass::Array->Count; ++i) {
		HouseExt::ExtData *pData = HouseExt::ExtMap.Find(HouseClass::Array->Items[i]);
		if(pData && pData->FirewallActive) {
			IsAnyFirestormActive = true;
			break;
		}
	}

	return IsAnyFirestormActive;
}

HouseExt::ExtData::~ExtData()
{
	if(Ares::bShuttingDown) {
		return;
	}

	for(auto Type : *TechnoTypeClass::Array) {
		if(auto TypeData = TechnoTypeExt::ExtMap.Find(Type)) {
			TypeData->ReversedByHouses.erase(this->AttachedToObject);
		}
	}
}

void HouseExt::ExtData::SetFirestormState(bool Active) {
	HouseClass *pHouse = this->AttachedToObject;
	HouseExt::ExtData* pData = HouseExt::ExtMap.Find(pHouse);

	if(pData->FirewallActive == Active) {
		return;
	}

	pData->FirewallActive = Active;
	UpdateAnyFirestormActive();

	DynamicVectorClass<CellStruct> AffectedCoords;

	for(int i = 0; i < pHouse->Buildings.Count; ++i) {
		BuildingClass *B = pHouse->Buildings[i];
		BuildingTypeExt::ExtData *pBuildTypeData = BuildingTypeExt::ExtMap.Find(B->Type);
		if(pBuildTypeData->Firewall_Is) {
			BuildingExt::ExtData * pBldData = BuildingExt::ExtMap.Find(B);
			pBldData->UpdateFirewall();
			CellStruct temp = B->GetMapCoords();
			AffectedCoords.AddItem(temp);
		}
	}

	MapClass::Instance->Update_Pathfinding_1();
	MapClass::Instance->Update_Pathfinding_2(&AffectedCoords);
};

/**
 * moved the fix for #917 here - check a house's ability to handle base plan before it actually tries to generate a base plan, not at game start
 * (we have no idea what houses at game start are supposed to be able to do base planning, so mission maps fuck up)
 */
bool HouseExt::ExtData::CheckBasePlanSanity() {
	auto House = this->AttachedToObject;
	// this shouldn't happen, but you never know
	if(House->ControlledByHuman() || House->IsNeutral()) {
		return true;
	}

	const char *errorMsg = "AI House of country [%s] cannot build any object in %s. The AI ain't smart enough for that.\n";
	bool AllIsWell(true);

	// if you don't have a base unit buildable, how did you get to base planning?
	// only through crates or map actions, so have to validate base unit in other situations
	auto pArray = &RulesClass::Instance->BaseUnit;
	bool canBuild = false;
	for(int i = 0; i < pArray->Count; ++i) {
		auto Item = pArray->GetItem(i);
		if(House->CanExpectToBuild(Item)) {
			canBuild = true;
			break;
		}
	}
	if(!canBuild) {
		AllIsWell = false;
		Debug::DevLog(Debug::Error, errorMsg, House->Type->ID, "BaseUnit");
	}

	auto CheckList = [House, errorMsg, &AllIsWell]
			(DynamicVectorClass<BuildingTypeClass *> *const List, char * const ListName) -> void {
		if(!House->FirstBuildableFromArray(List)) {
			AllIsWell = false;
			Debug::DevLog(Debug::Error, errorMsg, House->Type->ID, ListName);
		}
	};

	// commented out lists that do not cause a crash, according to testers
//	CheckList(&RulesClass::Instance->Shipyard, "Shipyard");
	CheckList(&RulesClass::Instance->BuildPower, "BuildPower");
	CheckList(&RulesClass::Instance->BuildRefinery, "BuildRefinery");
	CheckList(&RulesClass::Instance->BuildWeapons, "BuildWeapons");

//			CheckList(&RulesClass::Instance->BuildConst, "BuildConst");
//			CheckList(&RulesClass::Instance->BuildBarracks, "BuildBarracks");
//			CheckList(&RulesClass::Instance->BuildTech, "BuildTech");
//			CheckList(&RulesClass::Instance->BuildRadar, "BuildRadar");
//			CheckList(&RulesClass::Instance->ConcreteWalls, "ConcreteWalls");
//			CheckList(&RulesClass::Instance->BuildDummy, "BuildDummy");
//			CheckList(&RulesClass::Instance->BuildNavalYard, "BuildNavalYard");

	auto pCountryData = HouseTypeExt::ExtMap.Find(House->Type);
	auto Powerplants = pCountryData->GetPowerplants();
	DynamicVectorClass<BuildingTypeClass*> Dummy(Powerplants.size(), const_cast<BuildingTypeClass**>(Powerplants.begin()));
	CheckList(&Dummy, "Powerplants");

//			auto pSide = SideClass::Array->GetItem(curHouse->Type->SideIndex);
//			auto pSideData = SideExt::ExtMap.Find(pSide);
//			CheckList(&pSideData->BaseDefenses, "Base Defenses");

	return AllIsWell;
}

void HouseExt::ExtData::UpdateTogglePower() {
	const auto pThis = this->AttachedToObject;

	auto pRulesExt = RulesExt::Global();

	if(!pRulesExt->TogglePowerAllowed
		|| pRulesExt->TogglePowerDelay <= 0
		|| pRulesExt->TogglePowerIQ < 0
		|| pRulesExt->TogglePowerIQ > pThis->IQLevel2
		|| pThis->Buildings.Count == 0
		|| pThis->IsBeingDrained 
		|| pThis->ControlledByHuman()
		|| pThis->PowerBlackoutTimer.InProgress())
	{
		return;
	}

	if(Unsorted::CurrentFrame % pRulesExt->TogglePowerDelay == 0) {
		struct ExpendabilityStruct {
			int Compare(const ExpendabilityStruct& lhs, const ExpendabilityStruct& rhs) {
				if(lhs.Value < rhs.Value) {
					return -1;
				}

				if(this->Value > rhs.Value) {
					return 1;
				}

				return 0;
			}

			bool operator < (const ExpendabilityStruct& rhs) {
				if(auto res = Compare(*this, rhs)) {
					return res < 0;
				}

				// tie breaker to prevent desyncs
				return *this->Building < *rhs.Building;
			}

			bool operator >(const ExpendabilityStruct& rhs) {
				if(auto res = Compare(*this, rhs)) {
					return res > 0;
				}

				// tie breaker to prevent desyncs
				return *this->Building < *rhs.Building;
			}

			BuildingClass* Building;
			int Value;
		};

		// properties: the higher this value is, the more likely
		// this building is turned off (expendability)
		auto GetExpendability = [](BuildingClass* pBld) -> int {
			auto pType = pBld->Type;

			// disable super weapons, because a defenseless base is
			// worse than one without super weapons
			if(pType->SuperWeapon != -1 || pType->SuperWeapon2 != -1) {
				return pType->PowerDrain * 20 / 10;
			}

			// non-base defenses should be disabled before going
			// to the base defenses. but power intensive defenses
			// might still evaluate worse
			if(!pType->IsBaseDefense) {
				return pType->PowerDrain * 15 / 10;
			}

			// default case, use power
			return pType->PowerDrain;
		};

		// create a list of all buildings that can be powered down
		// and give each building an expendability value
		std::vector<ExpendabilityStruct> Buildings;
		Buildings.reserve(pThis->Buildings.Count);

		const auto HasLowPower = pThis->HasLowPower();

		for(auto pBld : pThis->Buildings) {
			auto pType = pBld->Type;
			if(pType->CanTogglePower() && pType->PowerDrain > 0) {
				// if low power, we get buildings with StuffEnabled, if enough
				// power, we look for builidings that are disabled
				if(pBld->StuffEnabled == HasLowPower) {
					Buildings.emplace_back(ExpendabilityStruct{pBld, GetExpendability(pBld)});
				}
			}
		}

		int Surplus = pThis->PowerOutput - pThis->PowerDrain;

		if(HasLowPower) {
			// most expendable building first
			std::sort(Buildings.begin(), Buildings.end(), std::greater<>());

			// turn off the expendable buildings until power is restored
			for(const auto& item : Buildings) {
				auto Drain = item.Building->Type->PowerDrain;

				item.Building->GoOffline();
				Surplus += Drain;

				if(Surplus >= 0) {
					break;
				}
			}
		} else {
			// least expendable building first
			std::sort(Buildings.begin(), Buildings.end(), std::less<>());

			// turn on as many of them as possible
			for(const auto& item : Buildings) {
				auto Drain = item.Building->Type->PowerDrain;
				if(Surplus - Drain >= 0) {
					item.Building->GoOnline();
					Surplus -= Drain;
				}
			}
		}
	}
}

SideClass* HouseExt::GetSide(HouseClass* pHouse) {
	return SideClass::Array->GetItemOrDefault(pHouse->SideIndex);
}

int HouseExt::ExtData::GetSurvivorDivisor() const {
	if(auto pExt = SideExt::ExtMap.Find(HouseExt::GetSide(this->AttachedToObject))) {
		return pExt->GetSurvivorDivisor();
	}

	return 0;
}

InfantryTypeClass* HouseExt::ExtData::GetCrew() const {
	if(auto pExt = SideExt::ExtMap.Find(HouseExt::GetSide(this->AttachedToObject))) {
		return pExt->GetCrew();
	}

	return RulesClass::Instance->Technician;
}

InfantryTypeClass* HouseExt::ExtData::GetEngineer() const {
	if(auto pExt = SideExt::ExtMap.Find(HouseExt::GetSide(this->AttachedToObject))) {
		return pExt->GetEngineer();
	}

	return RulesClass::Instance->Engineer;
}

InfantryTypeClass* HouseExt::ExtData::GetTechnician() const {
	if(auto pExt = SideExt::ExtMap.Find(HouseExt::GetSide(this->AttachedToObject))) {
		return pExt->GetTechnician();
	}

	return RulesClass::Instance->Technician;
}

InfantryTypeClass* HouseExt::ExtData::GetDisguise() const {
	if(auto pExt = SideExt::ExtMap.Find(HouseExt::GetSide(this->AttachedToObject))) {
		return pExt->GetDisguise();
	}

	return RulesClass::Instance->ThirdDisguise;
}

// =============================
// container hooks

DEFINE_HOOK(4F6532, HouseClass_CTOR, 5)
{
	GET(HouseClass*, pItem, EAX);

	HouseExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(4F7371, HouseClass_DTOR, 6)
{
	GET(HouseClass*, pItem, ESI);

	HouseExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(504080, HouseClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(503040, HouseClass_SaveLoad_Prefix, 5)
{
	GET_STACK(HouseExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<HouseExt>::PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(504069, HouseClass_Load_Suffix, 7)
{
	HouseExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(5046DE, HouseClass_Save_Suffix, 7)
{
	HouseExt::ExtMap.SaveStatic();
	return 0;
}
