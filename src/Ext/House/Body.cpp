#include "Body.h"
#include "../HouseType/Body.h"
#include "../Building/Body.h"
#include "../BuildingType/Body.h"
#include "../Rules/Body.h"
#include "../Side/Body.h"
#include "../SWType/Body.h"
#include "../TechnoType/Body.h"
#include "../../Enum/Prerequisites.h"
#include "../Techno/Body.h"
#include "../../Misc/SWTypes.h"
#include "../../Utilities/INIParser.h"

#include <FactoryClass.h>
#include <DiscreteSelectionClass.h>
#include <HouseClass.h>
#include <MouseClass.h>
#include <SuperClass.h>
#include <ScenarioClass.h>

#include "../../Misc/SavegameDef.h"

#include <functional>

template<> const DWORD Extension<HouseClass>::Canary = 0x12345678;
HouseExt::ExtContainer HouseExt::ExtMap;

bool HouseExt::IsAnyFirestormActive = false;

std::vector<int> HouseExt::AIProduction_CreationFrames;
std::vector<int> HouseExt::AIProduction_Values;
std::vector<int> HouseExt::AIProduction_BestChoices;

// =============================
// member funcs

void HouseExt::ExtData::LoadFromINIFile(CCINIClass* const pINI) {
	auto const pThis = this->OwnerObject();
	auto const* const pSection = pThis->PlainName;

	INI_EX exINI(pINI);

	this->Degrades.Read(exINI, pSection, "Degrades");
}

HouseExt::RequirementStatus HouseExt::RequirementsMet(
	HouseClass const* const pHouse, TechnoTypeClass const* const pItem)
{
	if(pItem->Unbuildable) {
		return RequirementStatus::Forbidden;
	}

	TechnoTypeExt::ExtData* pData = TechnoTypeExt::ExtMap.Find(pItem);
	if(!pItem) {
		return RequirementStatus::Forbidden;
	}
//	TechnoTypeClassExt::TechnoTypeClassData *pData = TechnoTypeClassExt::Ext_p[pItem];
	HouseExt::ExtData* pHouseExt = HouseExt::ExtMap.Find(pHouse);

	// this has to happen before the first possible "can build" response or NCO happens
	if(pItem->WhatAmI() != AbstractType::BuildingType
		&& HasFactory(pHouse, pItem, false) == FactoryState::NoFactory)
	{
		return RequirementStatus::Incomplete;
	}

	if(!(pData->PrerequisiteTheaters & (1 << static_cast<int>(ScenarioClass::Instance->Theater)))) { return RequirementStatus::Forbidden; }
	if(Prereqs::HouseOwnsAny(pHouse, pData->PrerequisiteNegatives)) { return RequirementStatus::Forbidden; }

	if(pData->ReversedByHouses.contains(pHouse)) {
		return RequirementStatus::Overridden;
	}

	if(pData->RequiredStolenTech.any()) {
		if((pHouseExt->StolenTech & pData->RequiredStolenTech) != pData->RequiredStolenTech) { return RequirementStatus::Incomplete; }
	}

	// yes, the game checks it here
	// hack value - skip real prereq check
	if(Prereqs::HouseOwnsAny(pHouse, pItem->PrerequisiteOverride)) { return RequirementStatus::Overridden; }

	if(pHouse->HasFromSecretLab(pItem)) { return RequirementStatus::Overridden; }

	if(pHouse->ControlledByHuman() && pItem->TechLevel == -1) { return RequirementStatus::Incomplete; }

	if(!pHouse->HasAllStolenTech(pItem)) { return RequirementStatus::Incomplete; }

	if(!pHouse->InRequiredHouses(pItem) || pHouse->InForbiddenHouses(pItem)) { return RequirementStatus::Forbidden; }

	if(!HouseExt::CheckFactoryOwners(pHouse, pItem)) { return RequirementStatus::Incomplete; }

	if(auto const pBldType = specific_cast<BuildingTypeClass const*>(pItem)) {
		if(HouseExt::IsDisabledFromShell(pHouse, pBldType)) {
			return RequirementStatus::Forbidden;
		}
	}

	return (pHouse->TechLevel >= pItem->TechLevel) ? RequirementStatus::Complete : RequirementStatus::Incomplete;
}

bool HouseExt::PrerequisitesMet(
	HouseClass const* const pHouse, TechnoTypeClass const* const pItem)
{
	if(!pItem) {
		return false;
	}

	auto const pData = TechnoTypeExt::ExtMap.Find(pItem);

	for(const auto& list : pData->PrerequisiteLists) {
		if(Prereqs::HouseOwnsAll(pHouse, list)) {
			return true;
		}
	}

	return false;
}

bool HouseExt::PrerequisitesListed(
	Prereqs::BTypeIter const& List, TechnoTypeClass const* const pItem)
{
	if(!pItem) {
		return false;
	}

	auto const pData = TechnoTypeExt::ExtMap.Find(pItem);

	for(const auto& list : pData->PrerequisiteLists) {
		if(Prereqs::ListContainsAll(List, list)) {
			return true;
		}
	}

	return false;
}

HouseExt::BuildLimitStatus HouseExt::CheckBuildLimit(
	HouseClass const* const pHouse, TechnoTypeClass const* const pItem,
	bool const includeQueued)
{
	int BuildLimit = pItem->BuildLimit;
	int Remaining = HouseExt::BuildLimitRemaining(pHouse, pItem);
	if(BuildLimit > 0) {
		if(Remaining <= 0) {
			return (includeQueued && FactoryClass::FindByOwnerAndProduct(pHouse, pItem))
				? BuildLimitStatus::NotReached
				: BuildLimitStatus::ReachedPermanently
			;
		}
	}
	return (Remaining > 0)
		? BuildLimitStatus::NotReached
		: BuildLimitStatus::ReachedTemporarily
	;
}

signed int HouseExt::BuildLimitRemaining(
	HouseClass const* const pHouse, TechnoTypeClass const* const pItem)
{
	auto const BuildLimit = pItem->BuildLimit;
	if(BuildLimit >= 0) {
		return BuildLimit - HouseExt::CountOwnedNowTotal(pHouse, pItem);
	} else {
		return -BuildLimit - pHouse->CountOwnedEver(pItem);
	}
}

int HouseExt::CountOwnedNowTotal(
	HouseClass const* const pHouse, TechnoTypeClass const* const pItem)
{
	int index = -1;
	int sum = 0;
	const BuildingTypeClass* pBType = nullptr;
	const UnitTypeClass* pUType = nullptr;
	const InfantryTypeClass* pIType = nullptr;
	const char* pPowersUp = nullptr;

	switch(pItem->WhatAmI()) {
	case AbstractType::BuildingType:
		pBType = static_cast<BuildingTypeClass const*>(pItem);
		pPowersUp = pBType->PowersUpBuilding;
		if(pPowersUp[0]) {
			if(auto const pTPowersUp = BuildingTypeClass::Find(pPowersUp)) {
				for(auto const& pBld : pHouse->Buildings) {
					if(pBld->Type == pTPowersUp) {
						for(auto const& pUpgrade : pBld->Upgrades) {
							if(pUpgrade == pBType) {
								++sum;
							}
						}
					}
				}
			}
		} else {
			sum = pHouse->CountOwnedNow(pBType);
			if(auto const pUndeploy = pBType->UndeploysInto) {
				sum += pHouse->CountOwnedNow(pUndeploy);
			}
		}
		break;

	case AbstractType::UnitType:
		pUType = static_cast<UnitTypeClass const*>(pItem);
		sum = pHouse->CountOwnedNow(pUType);
		if(auto const pDeploy = pUType->DeploysInto) {
			sum += pHouse->CountOwnedNow(pDeploy);
		}
		break;

	case AbstractType::InfantryType:
		pIType = static_cast<InfantryTypeClass const*>(pItem);
		sum = pHouse->CountOwnedNow(pIType);
		if(pIType->VehicleThief) {
			index = pIType->ArrayIndex;
			for(auto const& pUnit : *UnitClass::Array) {
				if(pUnit->HijackerInfantryType == index
					&& pUnit->Owner == pHouse)
				{
					++sum;
				}
			}
		}
		break;

	case AbstractType::AircraftType:
		sum = pHouse->CountOwnedNow(
			static_cast<AircraftTypeClass const*>(pItem));
		break;

	default:
		__assume(0);
	}

	return sum;
}

signed int HouseExt::PrereqValidate(
	HouseClass const* const pHouse, TechnoTypeClass const* const pItem,
	bool const buildLimitOnly, bool const includeQueued)
{
	if(!buildLimitOnly) {
		RequirementStatus ReqsMet = HouseExt::RequirementsMet(pHouse, pItem);
		if(ReqsMet == RequirementStatus::Forbidden || ReqsMet == RequirementStatus::Incomplete) {
			return 0;
		}

		if(!pHouse->ControlledByHuman()) {
			if(Ares::GlobalControls::AllowBypassBuildLimit[pHouse->GetAIDifficultyIndex()]) {
				return 1;
			} else {
				return static_cast<signed int>(HouseExt::CheckBuildLimit(pHouse, pItem, includeQueued));
			}
		}

		if(ReqsMet == RequirementStatus::Complete) {
			if(!HouseExt::PrerequisitesMet(pHouse, pItem)) {
				return 0;
			}
		}
	}

	auto const state = HouseExt::HasFactory(pHouse, pItem, true);
	if(state == FactoryState::NoFactory) {
		Debug::Log(Debug::Severity::Error, "[NCO Bug detected] "
			"House %ls meets all requirements to build %s, but doesn't have a suitable factory!\n",
			pHouse->UIName, pItem->ID);
	}
	if(state != FactoryState::Available) {
		return 0;
	}

	return static_cast<signed int>(HouseExt::CheckBuildLimit(pHouse, pItem, includeQueued));
}

bool HouseExt::IsDisabledFromShell(
	HouseClass const* const pHouse, BuildingTypeClass const* const pItem)
{
	// SWAllowed does not apply to campaigns any more
	if(SessionClass::Instance->GameMode == GameMode::Campaign
		|| GameModeOptionsClass::Instance->SWAllowed)
	{
		return false;
	}

	if(pItem->SuperWeapon != -1) {
		// allow SWs only if not disableable from shell
		auto const pItem2 = const_cast<BuildingTypeClass*>(pItem);
		auto const& BuildTech = RulesClass::Instance->BuildTech;
		if(BuildTech.FindItemIndex(pItem2) == -1) {
			auto const pSuper = pHouse->Supers[pItem->SuperWeapon];
			if(pSuper->Type->DisableableFromShell) {
				return true;
			}
		}
	}

	return false;
}

size_t HouseExt::FindOwnedIndex(
	HouseClass const* const, int const idxParentCountry,
	Iterator<TechnoTypeClass const*> const items, size_t const start)
{
	auto const bitOwner = 1u << idxParentCountry;

	for(auto i = start; i < items.size(); ++i) {
		auto const pItem = items[i];

		if(pItem->InOwners(bitOwner)) {
			return i;
		}
	}

	return items.size();
}

size_t HouseExt::FindBuildableIndex(
	HouseClass const* const pHouse, int const idxParentCountry,
	Iterator<TechnoTypeClass const*> const items, size_t const start)
{
	for(auto i = start; i < items.size(); ++i) {
		auto const pItem = items[i];

		if(pHouse->CanExpectToBuild(pItem, idxParentCountry)) {
			auto const pBld = abstract_cast<const BuildingTypeClass*>(pItem);
			if(pBld && HouseExt::IsDisabledFromShell(pHouse, pBld)) {
				continue;
			}

			return i;
		}
	}

	return items.size();
}

HouseExt::FactoryState HouseExt::HasFactory(
	HouseClass const* const pHouse, TechnoTypeClass const* const pItem,
	bool const requirePower)
{
	auto const pExt = TechnoTypeExt::ExtMap.Find(pItem);
	auto const bitsOwners = pItem->GetOwners();
	auto const isNaval = pItem->Naval;
	auto const abs = pItem->WhatAmI();

	auto ret = FactoryState::NoFactory;

	for(auto const& pBld : pHouse->Buildings) {
		if(pBld->InLimbo
			|| pBld->GetCurrentMission() == Mission::Selling
			|| pBld->QueuedMission == Mission::Selling)
		{
			continue;
		}

		auto const pType = pBld->Type;

		if(pType->Factory != abs
			|| (abs == AbstractType::UnitType && pType->Naval != isNaval)
			|| !pExt->CanBeBuiltAt(pType)
			|| !pType->InOwners(bitsOwners))
		{
			continue;
		}

		if(!requirePower || pBld->HasPower) {
			return FactoryState::Available;
		}

		ret = FactoryState::Unpowered;
	}

	return ret;
}

bool HouseExt::CheckFactoryOwners(
	HouseClass const* const pHouse, TechnoTypeClass const* const pItem)
{
	return HouseExt::CheckFactoryOwner(pHouse, pItem)
		&& HouseExt::CheckForbiddenFactoryOwner(pHouse, pItem);
}

bool HouseExt::CheckFactoryOwner(
	HouseClass const* const pHouse, TechnoTypeClass const* const pItem)
{
	auto const pExt = TechnoTypeExt::ExtMap.Find(pItem);
	auto const pHouseExt = HouseExt::ExtMap.Find(pHouse);

	if(!pExt->FactoryOwners.empty()) {
		for(auto const& pOwner : pExt->FactoryOwners) {
			if(pHouseExt->FactoryOwners_GatheredPlansOf.Contains(pOwner)) {
				return true;
			}
		}

		auto const abs = pItem->WhatAmI();

		for(auto const& pBld : pHouse->Buildings) {
			if(pBld->Type->Factory == abs) {
				auto const FactoryExt = TechnoExt::ExtMap.Find(pBld);

				if(pExt->FactoryOwners.Contains(FactoryExt->OriginalHouseType)) {
					return true;
				}
			}
		}

		return false;
	}

	return true;
}

bool HouseExt::CheckForbiddenFactoryOwner(
	HouseClass const* const pHouse, TechnoTypeClass const* pItem)
{
	auto const pExt = TechnoTypeExt::ExtMap.Find(pItem);
	auto const pHouseExt = HouseExt::ExtMap.Find(pHouse);

	auto const& Forbidden = pExt->ForbiddenFactoryOwners;

	if(!Forbidden.empty()) {
		// return true if not a single forbidden house is in the gathered plans
		// (only if there are any)
		auto const& Gathered = pHouseExt->FactoryOwners_GatheredPlansOf;
		if(!Gathered.empty()) {
			auto const hasGathered = std::any_of(Forbidden.begin(), Forbidden.end(),
				[&Gathered](HouseTypeClass* const pForbidden)
			{
				return Gathered.Contains(pForbidden);
			});

			if(!hasGathered) {
				return true;
			}
		}

		auto const abs = pItem->WhatAmI();

		for(auto const& pBld : pHouse->Buildings) {
			if(pBld->Type->Factory == abs) {
				auto const pFactoryExt = TechnoExt::ExtMap.Find(pBld);

				if(!Forbidden.Contains(pFactoryExt->OriginalHouseType)) {
					return true;
				}
			}
		}

		return false;
	}

	return true;
}

bool HouseExt::UpdateAnyFirestormActive(bool const lastChange) {
	IsAnyFirestormActive = lastChange;

	// if last change activated one, there is at least one. else...
	if(!lastChange) {
		for(auto const& pHouse : *HouseClass::Array) {
			auto const pData = HouseExt::ExtMap.Find(pHouse);
			if(pData && pData->FirewallActive) {
				IsAnyFirestormActive = true;
				break;
			}
		}
	}

	return IsAnyFirestormActive;
}

HouseClass* HouseExt::GetHouseKind(
	OwnerHouseKind const kind, bool const allowRandom,
	HouseClass* const pDefault, HouseClass* const pInvoker,
	HouseClass* const pKiller, HouseClass* const pVictim)
{
	switch(kind) {
	case OwnerHouseKind::Invoker:
		return pInvoker ? pInvoker : pDefault;
	case OwnerHouseKind::Killer:
		return pKiller ? pKiller : pDefault;
	case OwnerHouseKind::Victim:
		return pVictim ? pVictim : pDefault;
	case OwnerHouseKind::Civilian:
		return HouseClass::FindCivilianSide();
	case OwnerHouseKind::Special:
		return HouseClass::FindSpecial();
	case OwnerHouseKind::Neutral:
		return HouseClass::FindNeutral();
	case OwnerHouseKind::Random:
		if(allowRandom) {
			auto& Random = ScenarioClass::Instance->Random;
			return HouseClass::Array->GetItem(
				Random.RandomRanged(0, HouseClass::Array->Count - 1));
		} else {
			return pDefault;
		}
	case OwnerHouseKind::Default:
	default:
		return pDefault;
	}
}

HouseExt::ExtData::~ExtData()
{
	if(Ares::bShuttingDown) {
		return;
	}

	for(auto const pType : *TechnoTypeClass::Array) {
		if(auto const pTypeData = TechnoTypeExt::ExtMap.Find(pType)) {
			pTypeData->ReversedByHouses.erase(this->OwnerObject());
		}
	}
}

void HouseExt::ExtData::SetFirestormState(bool const active) {
	auto const pHouse = this->OwnerObject();
	auto const pData = HouseExt::ExtMap.Find(pHouse);

	if(pData->FirewallActive == active) {
		return;
	}

	pData->FirewallActive = active;
	UpdateAnyFirestormActive(active);

	DynamicVectorClass<CellStruct> AffectedCoords;

	for(auto const& pBld : pHouse->Buildings) {
		auto const pTypeData = BuildingTypeExt::ExtMap.Find(pBld->Type);
		if(pTypeData->Firewall_Is) {
			auto const pExt = BuildingExt::ExtMap.Find(pBld);
			pExt->UpdateFirewall();
			auto const temp = pBld->GetMapCoords();
			AffectedCoords.AddItem(temp);
		}
	}

	MapClass::Instance->Update_Pathfinding_1();
	MapClass::Instance->Update_Pathfinding_2(AffectedCoords);
};

/**
 * moved the fix for #917 here - check a house's ability to handle base plan
 * before it actually tries to generate a base plan, not at game start (we have
 * no idea what houses at game start are supposed to be able to do base
 * planning, so mission maps fuck up)
 */
bool HouseExt::ExtData::CheckBasePlanSanity() {
	auto const pThis = this->OwnerObject();
	// this shouldn't happen, but you never know
	if(pThis->ControlledByHuman() || pThis->IsNeutral()) {
		return true;
	}

	auto AllIsWell = true;

	auto const pRules = RulesClass::Instance;
	auto const pType = pThis->Type;

	auto const errorMsg = "AI House of country [%s] cannot build any object in "
		"%s. The AI ain't smart enough for that.\n";

	// if you don't have a base unit buildable, how did you get to base
	// planning? only through crates or map actions, so have to validate base
	// unit in other situations
	auto const idxParent = pType->FindParentCountryIndex();
	auto const canBuild = std::any_of(
		pRules->BaseUnit.begin(), pRules->BaseUnit.end(),
		[pThis, idxParent] (UnitTypeClass const* const pItem)
	{
		return pThis->CanExpectToBuild(pItem, idxParent);
	});

	if(!canBuild) {
		AllIsWell = false;
		Debug::Log(Debug::Severity::Error, errorMsg, pType->ID, "BaseUnit");
	}

	auto CheckList = [pThis, pType, idxParent, errorMsg, &AllIsWell] (
		Iterator<BuildingTypeClass const*> const list,
		const char* const ListName) -> void
	{
		if(!HouseExt::FindBuildable(pThis, idxParent, list)) {
			AllIsWell = false;
			Debug::Log(Debug::Severity::Error, errorMsg, pType->ID, ListName);
		}
	};

	// commented out lists that do not cause a crash, according to testers
	//CheckList(make_iterator(pRules->Shipyard), "Shipyard");
	CheckList(make_iterator(pRules->BuildPower), "BuildPower");
	CheckList(make_iterator(pRules->BuildRefinery), "BuildRefinery");
	CheckList(make_iterator(pRules->BuildWeapons), "BuildWeapons");
	//CheckList(make_iterator(pRules->BuildConst), "BuildConst");
	//CheckList(make_iterator(pRules->BuildBarracks), "BuildBarracks");
	//CheckList(make_iterator(pRules->BuildTech), "BuildTech");
	//CheckList(make_iterator(pRules->BuildRadar), "BuildRadar");
	//CheckList(make_iterator(pRules->ConcreteWalls), "ConcreteWalls");
	//CheckList(make_iterator(pRules->BuildDummy), "BuildDummy");
	//CheckList(make_iterator(pRules->BuildNavalYard), "BuildNavalYard");

	auto const pCountryData = HouseTypeExt::ExtMap.Find(pType);
	auto const Powerplants = pCountryData->GetPowerplants();
	CheckList(Powerplants, "Powerplants");

	//auto const pSide = SideClass::Array->GetItemOrDefault(pType->SideIndex);
	//if(auto const pSideExt = SideExt::ExtMap.Find(pSide)) {
	//	CheckList(make_iterator(pSideExt->BaseDefenses), "Base Defenses");
	//}

	return AllIsWell;
}

void HouseExt::ExtData::UpdateTogglePower() {
	const auto pThis = this->OwnerObject();

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
		private:
			std::tuple<const int&, BuildingClass&> Tie() const {
				// compare with tie breaker to prevent desyncs
				return std::tie(this->Value, *this->Building);
			}

		public:
			bool operator < (const ExpendabilityStruct& rhs) const {
				return this->Tie() < rhs.Tie();
			}

			bool operator > (const ExpendabilityStruct& rhs) const {
				return this->Tie() > rhs.Tie();
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
			if(pType->HasSuperWeapon()) {
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
	if(auto pExt = SideExt::ExtMap.Find(HouseExt::GetSide(this->OwnerObject()))) {
		return pExt->GetSurvivorDivisor();
	}

	return 0;
}

InfantryTypeClass* HouseExt::ExtData::GetCrew() const {
	if(auto pExt = SideExt::ExtMap.Find(HouseExt::GetSide(this->OwnerObject()))) {
		return pExt->GetCrew();
	}

	return RulesClass::Instance->Technician;
}

InfantryTypeClass* HouseExt::ExtData::GetEngineer() const {
	if(auto pExt = SideExt::ExtMap.Find(HouseExt::GetSide(this->OwnerObject()))) {
		return pExt->GetEngineer();
	}

	return RulesClass::Instance->Engineer;
}

InfantryTypeClass* HouseExt::ExtData::GetTechnician() const {
	if(auto pExt = SideExt::ExtMap.Find(HouseExt::GetSide(this->OwnerObject()))) {
		return pExt->GetTechnician();
	}

	return RulesClass::Instance->Technician;
}

InfantryTypeClass* HouseExt::ExtData::GetDisguise() const {
	if(auto pExt = SideExt::ExtMap.Find(HouseExt::GetSide(this->OwnerObject()))) {
		return pExt->GetDisguise();
	}

	return RulesClass::Instance->ThirdDisguise;
}

void HouseExt::ExtData::UpdateAcademy(BuildingClass* pAcademy, bool added) {
	// check if added and there already, or removed and not there
	auto it = std::find(this->Academies.cbegin(), this->Academies.cend(), pAcademy);
	if(added == (it != this->Academies.cend())) {
		return;
	}

	// now this can be unconditional
	if(added) {
		this->Academies.push_back(pAcademy);
	} else {
		this->Academies.erase(it);
	}
}

void HouseExt::ExtData::ApplyAcademy(
	TechnoClass* const pTechno, AbstractType const considerAs) const
{
	// mutex in effect, ignore academies to fix preplaced order issues.
	// also triggered in game for certain "conversions" like deploy
	if(Unsorted::IKnowWhatImDoing) {
		return;
	}

	auto const pType = pTechno->GetTechnoType();

	// get the academy data for this type
	Valueable<double> BuildingTypeExt::ExtData::* pmBonus = nullptr;
	if(considerAs == AbstractType::Infantry) {
		pmBonus = &BuildingTypeExt::ExtData::AcademyInfantry;
	} else if(considerAs == AbstractType::Aircraft) {
		pmBonus = &BuildingTypeExt::ExtData::AcademyAircraft;
	} else if(considerAs == AbstractType::Unit) {
		pmBonus = &BuildingTypeExt::ExtData::AcademyVehicle;
	} else if(considerAs == AbstractType::Building) {
		pmBonus = &BuildingTypeExt::ExtData::AcademyBuilding;
	}

	auto veterancyBonus = 0.0;

	// aggregate the bonuses
	for(auto const& pBld : this->Academies) {
		auto const pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

		auto const isWhitelisted = pExt->AcademyWhitelist.empty()
			|| pExt->AcademyWhitelist.Contains(pType);

		if(isWhitelisted && !pExt->AcademyBlacklist.Contains(pType)) {
			const auto& data = pExt->*pmBonus;
			veterancyBonus = std::max(veterancyBonus, data.Get());
		}
	}

	// apply the bonus
	if(pType->Trainable) {
		auto& value = pTechno->Veterancy.Veterancy;
		if(veterancyBonus > value) {
			value = static_cast<float>(std::min(
				veterancyBonus, RulesClass::Instance->VeteranCap));
		}
	}
}

// =============================
// load / save

template <typename T>
void HouseExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->Degrades)
		.Process(this->IonSensitive)
		.Process(this->FirewallActive)
		.Process(this->SWLastIndex)
		.Process(this->Factory_BuildingType)
		.Process(this->Factory_InfantryType)
		.Process(this->Factory_VehicleType)
		.Process(this->Factory_NavyType)
		.Process(this->Factory_AircraftType)
		.Process(this->StolenTech)
		.Process(this->RadarPersist)
		.Process(this->FactoryOwners_GatheredPlansOf)
		.Process(this->Academies);
}

void HouseExt::ExtData::LoadFromStream(AresStreamReader &Stm) {
	Extension<HouseClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void HouseExt::ExtData::SaveToStream(AresStreamWriter &Stm) {
	Extension<HouseClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool HouseExt::LoadGlobals(AresStreamReader& Stm) {
	return Stm
		.Process(IsAnyFirestormActive)
		.Success();
}

bool HouseExt::SaveGlobals(AresStreamWriter& Stm) {
	return Stm
		.Process(IsAnyFirestormActive)
		.Success();
}

// =============================
// container

HouseExt::ExtContainer::ExtContainer() : Container("HouseClass") {
}

HouseExt::ExtContainer::~ExtContainer() = default;

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
	GET_STACK(HouseClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	HouseExt::ExtMap.PrepareStream(pItem, pStm);

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

DEFINE_HOOK(50114D, HouseClass_InitFromINI, 5)
{
	GET(HouseClass* const, pThis, EBX);
	GET(CCINIClass* const, pINI, ESI);

	HouseExt::ExtMap.LoadFromINI(pThis, pINI);

	return 0;
}
