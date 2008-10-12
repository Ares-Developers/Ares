#include <YRPP.h>
#include "HouseExt.h"
#include "Prerequisites.h"
#include "TechnoTypeExt.h"

EXT_P_DEFINE(HouseClass);

EXT_CTOR(HouseClass)
{
	if(!CONTAINS(Ext_p, pThis))
	{
		ALLOC(ExtData, pData);

		Ext_p[pThis] = pData;
	}
}

EXT_DTOR(HouseClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		DEALLOC(Ext_p, pThis);
	}
}

EXT_LOAD(HouseClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		Create(pThis);

		ULONG out;
		pStm->Read(&Ext_p[pThis], sizeof(ExtData), &out);
	}
}

EXT_SAVE(HouseClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		ULONG out;
		pStm->Write(&Ext_p[pThis], sizeof(ExtData), &out);
	}
}

signed int HouseClassExt::RequirementsMet(HouseClass *pHouse, TechnoTypeClass *pItem)
{
	if(pItem->get_Unbuildable()) { return 0; }

	RET_UNLESS(CONTAINS(TechnoTypeClassExt::Ext_p, pItem));
	TechnoTypeClassExt::TechnoTypeClassData *pData = TechnoTypeClassExt::Ext_p[pItem];

	// this has to happen before the first possible "can build" response or NCO happens
	if(pItem->WhatAmI() != abs_BuildingType && !pHouse->HasFactoryForObject(pItem)) { return 0; }

	if(!(pData->PrerequisiteTheaters & (1 << ScenarioClass::Global()->get_Theater()))) { return 0; }
	if(Prereqs::HouseOwnsAny(pHouse, &pData->PrerequisiteNegatives)) { return 0; }

	// yes, the game checks it here
	// hack value - skip real prereq check
	if(Prereqs::HouseOwnsAny(pHouse, pItem->get_PrerequisiteOverride())) { return -1; }

	if(pHouse->HasFromSecretLab(pItem)) { return -1; }

	if(pItem->get_TechLevel() == -1) { return 0; }

	if(!pHouse->HasAllStolenTech(pItem)) { return 0; }

	if(!pHouse->InRequiredHouses(pItem) || pHouse->InForbiddenHouses(pItem)) { return 0; }

	if(!Unsorted::SWAllowed && pItem->WhatAmI() == abs_BuildingType)
	{
		BuildingTypeClass *pBld = (BuildingTypeClass*)pItem;
		if(pBld->get_SuperWeapon() != -1)
		{
			bool InTech = 0;
			// AND AGAIN DVC<>::FindItemIndex fails! cannot find last item in the vector
			DynamicVectorClass<BuildingTypeClass *> *dvc = RulesClass::Global()->get_BuildTech();
			for(int i = 0; i < dvc->get_Count(); ++i)
			{
				if(pBld == dvc->GetItem(i))
				{
					InTech = 1;
					break;
				}
			}

			if(!InTech)
			{
				if(pHouse->get_Supers()->GetItem(pBld->get_SuperWeapon())->get_Type()->get_DisableableFromShell())
				{
					return 0;
				}
			}
		}
	}

	if(pHouse->get_TechLevel() < pItem->get_TechLevel()) { return 0; }

	return 1;
}

bool HouseClassExt::PrerequisitesMet(HouseClass *pHouse, TechnoTypeClass *pItem)
{
	RET_UNLESS(CONTAINS(TechnoTypeClassExt::Ext_p, pItem));
	TechnoTypeClassExt::TechnoTypeClassData *pData = TechnoTypeClassExt::Ext_p[pItem];

	for(int i = 0; i < pData->PrerequisiteLists.get_Count(); ++i)
	{
		if(Prereqs::HouseOwnsAll(pHouse, pData->PrerequisiteLists[i]))
		{
			return 1;
		}
	}

	return 0;
}

signed int HouseClassExt::CheckBuildLimit(HouseClass *pHouse, TechnoTypeClass *pItem, bool IncludeQueued)
{
	int BuildLimit = pItem->get_BuildLimit();
	int Remaining = HouseClassExt::BuildLimitRemaining(pHouse, pItem);
	if(BuildLimit > 0)
	{
		if(Remaining <= 0 && IncludeQueued)
		{
			return FactoryClass::FindThisOwnerAndProduct(pHouse, pItem) ? 1 : -1;
		}
	}
	return Remaining > 0;
}

signed int HouseClassExt::BuildLimitRemaining(HouseClass *pHouse, TechnoTypeClass *pItem)
{
	int BuildLimit = pItem->get_BuildLimit();
	if(BuildLimit >= 0)
	{
		return BuildLimit > pHouse->CountOwnedNow(pItem);
	}
	else
	{
		return abs(BuildLimit) > pHouse->CountOwnedEver(pItem);
	}
}

signed int HouseClassExt::PrereqValidate
	(HouseClass *pHouse, TechnoTypeClass *pItem, bool BuildLimitOnly, bool IncludeQueued)
{
	if(!BuildLimitOnly)
	{
		signed int ReqsMet = HouseClassExt::RequirementsMet(pHouse, pItem);
		if(!ReqsMet)
		{
			return 0;
		}

		if(!pHouse->get_PlayerControl())
		{
			return 1;
		}

		if(ReqsMet == 1)
		{
			if(!HouseClassExt::PrerequisitesMet(pHouse, pItem))
			{
				return 0;
			}
		}
	}

	return HouseClassExt::CheckBuildLimit(pHouse, pItem, IncludeQueued);
}

EXPORT_FUNC(HouseClass_PrereqValidator)
{
	// int (TechnoTypeClass *item, bool BuildLimitOnly, bool includeQueued)
	/* return
		 1 - cameo shown
		 0 - cameo not shown
		-1 - cameo greyed out
	 */

	GET(HouseClass *, pHouse, ECX);
	TechnoTypeClass *pItem = (TechnoTypeClass *)R->get_StackVar32(0x4);
	bool BuildLimitOnly = R->get_StackVar8(0x8) != 0;
	bool IncludeQueued = R->get_StackVar8(0xC) != 0;

	R->set_EAX(HouseClassExt::PrereqValidate(pHouse, pItem, BuildLimitOnly, IncludeQueued));
	return 0x4F8361;
}
