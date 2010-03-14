#include "Body.h"
#include "../BuildingType/Body.h"
#include "../TechnoType/Body.h"
#include "../../Enum/Prerequisites.h"

template<> const DWORD Extension<HouseClass>::Canary = 0x12345678;
Container<HouseExt> HouseExt::ExtMap;

template<> HouseExt::TT *Container<HouseExt>::SavingObject = NULL;
template<> IStream *Container<HouseExt>::SavingStream = NULL;

// =============================
// member funcs

signed int HouseExt::RequirementsMet(HouseClass *pHouse, TechnoTypeClass *pItem)
{
	if(pItem->Unbuildable) {
		return 0;
	}

	TechnoTypeExt::ExtData* pData = TechnoTypeExt::ExtMap.Find(pItem);
	if(!pItem) {
		return 0;
	}
//	TechnoTypeClassExt::TechnoTypeClassData *pData = TechnoTypeClassExt::Ext_p[pItem];
	HouseExt::ExtData* pHouseExt = HouseExt::ExtMap.Find(pHouse);

	// this has to happen before the first possible "can build" response or NCO happens
	if(pItem->WhatAmI() != abs_BuildingType && !pHouse->HasFactoryForObject(pItem)) { return 0; }

	if(!(pData->PrerequisiteTheaters & (1 << ScenarioClass::Global()->Theater))) { return 0; }
	if(Prereqs::HouseOwnsAny(pHouse, &pData->PrerequisiteNegatives)) { return 0; }

	if((pHouseExt->StolenTech & pData->RequiredStolenTech) != pData->RequiredStolenTech) { return 0; }

	// yes, the game checks it here
	// hack value - skip real prereq check
	if(Prereqs::HouseOwnsAny(pHouse, pItem->get_PrerequisiteOverride())) { return -1; }

	if(pHouse->HasFromSecretLab(pItem)) { return -1; }

	if(pItem->TechLevel == -1) { return 0; }

	if(!pHouse->HasAllStolenTech(pItem)) { return 0; }

	if(!pHouse->InRequiredHouses(pItem) || pHouse->InForbiddenHouses(pItem)) { return 0; }

	if(!Unsorted::SWAllowed) {
		if(BuildingTypeClass *pBld = specific_cast<BuildingTypeClass*>(pItem)) {
			if(pBld->SuperWeapon != -1) {
				if(RulesClass::Instance->BuildTech.FindItemIndex(&pBld) == -1) {
					if(pHouse->Supers.GetItem(pBld->SuperWeapon)->Type->DisableableFromShell) {
						return 0;
					}
				}
			}
		}
	}

	return (pHouse->TechLevel >= pItem->TechLevel) ? 1 : 0;
}

bool HouseExt::PrerequisitesMet(HouseClass *pHouse, TechnoTypeClass *pItem)
{
	TechnoTypeExt::ExtData* pData = TechnoTypeExt::ExtMap.Find(pItem);
	if(!pItem) {
		return 0;
	}

	for(int i = 0; i < pData->PrerequisiteLists.Count; ++i) {
		if(Prereqs::HouseOwnsAll(pHouse, pData->PrerequisiteLists[i])) {
			return 1;
		}
	}

	return 0;
}

signed int HouseExt::CheckBuildLimit(HouseClass *pHouse, TechnoTypeClass *pItem, bool IncludeQueued)
{
	int BuildLimit = pItem->BuildLimit;
	int Remaining = HouseExt::BuildLimitRemaining(pHouse, pItem);
	if(BuildLimit > 0) {
		if(Remaining <= 0 && IncludeQueued) {
			return FactoryClass::FindThisOwnerAndProduct(pHouse, pItem) ? 1 : -1;
		}
	}
	return Remaining > 0;
}

signed int HouseExt::BuildLimitRemaining(HouseClass *pHouse, TechnoTypeClass *pItem)
{
	int BuildLimit = pItem->BuildLimit;
	if(BuildLimit >= 0) {
		return BuildLimit > pHouse->CountOwnedNow(pItem);
	} else {
		return abs(BuildLimit) > pHouse->CountOwnedEver(pItem);
	}
}

signed int HouseExt::PrereqValidate
	(HouseClass *pHouse, TechnoTypeClass *pItem, bool BuildLimitOnly, bool IncludeQueued)
{
	if(!BuildLimitOnly) {
		signed int ReqsMet = HouseExt::RequirementsMet(pHouse, pItem);
		if(!ReqsMet) {
			return 0;
		}

		if(!pHouse->IsHumanoid()) {
			return 1;
		}

		if(ReqsMet == 1) {
			if(!HouseExt::PrerequisitesMet(pHouse, pItem)) {
				return 0;
			}
		}
	}

	return HouseExt::CheckBuildLimit(pHouse, pItem, IncludeQueued);
}

void HouseExt::Firestorm_SetState(HouseClass *pHouse, bool Active) {
	HouseExt::ExtData* pData = HouseExt::ExtMap.Find(pHouse);

	if(pData->FirewallActive == Active) {
		return;
	}

	pData->FirewallActive = Active;

	DynamicVectorClass<CellStruct> AffectedCoords;

	for(int i = 0; i < BuildingClass::Array->Count; ++i) {
		BuildingClass *B = BuildingClass::Array->Items[i];
		if(B->Owner == pHouse) {
			BuildingTypeExt::ExtData *pBuildTypeData = BuildingTypeExt::ExtMap.Find(B->Type);
			if(pBuildTypeData->Firewall_Is) {
				CellStruct temp;
				B->GetMapCoords(&temp);
				AffectedCoords.AddItem(temp);
			}
		}
	}

	MapClass::Global()->Update_Pathfinding_1();
	MapClass::Global()->Update_Pathfinding_2(&AffectedCoords);
};

// =============================
// load/save

void Container<HouseExt>::Load(HouseClass *pThis, IStream *pStm) {
	HouseExt::ExtData* pData = this->LoadKey(pThis, pStm);

	ULONG out;
	SWIZZLE(pData->Factory_BuildingType);
	SWIZZLE(pData->Factory_InfantryType);
	SWIZZLE(pData->Factory_VehicleType);
	SWIZZLE(pData->Factory_NavyType);
	SWIZZLE(pData->Factory_AircraftType);
}

// =============================
// container hooks

DEFINE_HOOK(4F6532, HouseClass_CTOR, 5)
{
	GET(HouseClass*, pItem, EAX);

	HouseExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(4F7140, HouseClass_DTOR, 6)
{
	GET(HouseClass*, pItem, ECX);

	HouseExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(503040, HouseClass_SaveLoad_Prefix, 5)
DEFINE_HOOK_AGAIN(504080, HouseClass_SaveLoad_Prefix, 5)
{
	GET_STACK(HouseExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<HouseExt>::SavingObject = pItem;
	Container<HouseExt>::SavingStream = pStm;

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
