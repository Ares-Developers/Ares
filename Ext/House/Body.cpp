#include "Body.h"
#include "..\TechnoType\Body.h"
#include "..\..\Enum\Prerequisites.h"

const DWORD Extension<HouseClass>::Canary = 0x12345678;
Container<HouseExt> HouseExt::ExtMap;

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

	// this has to happen before the first possible "can build" response or NCO happens
	if(pItem->WhatAmI() != abs_BuildingType && !pHouse->HasFactoryForObject(pItem)) { return 0; }

	if(!(pData->PrerequisiteTheaters & (1 << ScenarioClass::Global()->Theater))) { return 0; }
	if(Prereqs::HouseOwnsAny(pHouse, &pData->PrerequisiteNegatives)) { return 0; }

	// yes, the game checks it here
	// hack value - skip real prereq check
	if(Prereqs::HouseOwnsAny(pHouse, pItem->get_PrerequisiteOverride())) { return -1; }

	if(pHouse->HasFromSecretLab(pItem)) { return -1; }

	if(pItem->TechLevel == -1) { return 0; }

	if(!pHouse->HasAllStolenTech(pItem)) { return 0; }

	if(!pHouse->InRequiredHouses(pItem) || pHouse->InForbiddenHouses(pItem)) { return 0; }

	if(!Unsorted::SWAllowed && pItem->WhatAmI() == abs_BuildingType) {
		BuildingTypeClass *pBld = (BuildingTypeClass*)pItem;
		if(pBld->SuperWeapon != -1) {
			bool InTech = 0;
			// AND AGAIN DVC<>::FindItemIndex fails! cannot find last item in the vector
			DynamicVectorClass<BuildingTypeClass *> *dvc = RulesClass::Global()->get_BuildTech();
			for(int i = 0; i < dvc->Count; ++i) {
				if(pBld == dvc->GetItem(i)) {
					InTech = 1;
					break;
				}
			}

			if(!InTech) {
				if(pHouse->get_Supers()->GetItem(pBld->SuperWeapon)->Type->DisableableFromShell) {
					return 0;
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
			for(int j = 0; j < pData->PrerequisiteLists[i]->Count; ++j) {
			}
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

		if(!pHouse->PlayerControl) {
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
