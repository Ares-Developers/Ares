#include "Body.h"
#include "../BuildingType/Body.h"
#include "../HouseType/Body.h"
#include "../Techno/Body.h"
#include "../../Misc/Network.h"

#include <SpecificStructures.h>
#include <ScenarioClass.h>
#include <InfantryClass.h>
#include <CellClass.h>

#include <cmath>

/* #754 - evict Hospital/Armory contents */
DEFINE_HOOK(448277, BuildingClass_UnloadPassengers_ChangeOwner_SellAndLeaveBomb, 5)
{
	GET(BuildingClass *, B, ESI);

	BuildingExt::KickOutHospitalArmory(B);
	return 0x448293;
}

DEFINE_HOOK(447113, BuildingClass_UnloadPassengers_ChangeOwner_Sell, 6)
{
	GET(BuildingClass *, B, ESI);

	BuildingExt::KickOutHospitalArmory(B);
	return 0;
}

DEFINE_HOOK(44D8A1, BuildingClass_UnloadPassengers_Unload, 6)
{
	GET(BuildingClass *, B, EBP);

	BuildingExt::KickOutHospitalArmory(B);
	return 0;
}


/* 	#218 - specific occupiers -- see Hooks.Trenches.cpp */

// EMP'd power plants don't produce power
DEFINE_HOOK(44E855, BuildingClass_PowerProduced_EMP, 6) {
	GET(BuildingClass*, pBld, ESI);
	return ((pBld->EMPLockRemaining > 0) ? 0x44E873 : 0);
}

// VeteranBuildings
DEFINE_HOOK(43BA48, BuildingClass_CTOR_VeteranBuildings, 6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(BuildingTypeClass*, pType, EAX);

	if(auto pOwner = pThis->Owner) {
		if(auto pExt = HouseTypeExt::ExtMap.Find(pOwner->Type)) {
			if(pExt->VeteranBuildings.Contains(pType)) {
				pThis->Veterancy.SetVeteran(true);
			}
		}
	}

	return 0;
}

// restore pip count for tiberium storage (building and house)
DEFINE_HOOK(44D755, BuildingClass_GetPipFillLevel_Tiberium, 6)
{
	GET(BuildingClass*, pThis, ECX);
	GET(BuildingTypeClass*, pType, ESI);

	double amount = 0.0;
	if(pType->Storage > 0) {
		amount = pThis->Tiberium.GetTotalAmount() / pType->Storage;
	} else {
		amount = pThis->Owner->GetStoragePercentage();
	}

	int ret = Game::F2I(pType->GetPipMax() * amount);
	R->EAX(ret);
	return 0x44D750;
}

// the game specifically hides tiberium building pips. allow them, but
// take care they don't show up for the original game
DEFINE_HOOK(709B4E, TechnoClass_DrawPipscale_SkipSkipTiberium, 6)
{
	GET(TechnoClass*, pThis, EBP);

	bool showTiberium = true;
	if(auto pType = specific_cast<BuildingTypeClass*>(pThis->GetTechnoType())) {
		if((pType->Refinery || pType->ResourceDestination) && pType->Storage > 0) {
			// show only if this refinary uses storage. otherwise, the original
			// refineries would show an unused tiberium pip scale
			auto pExt = TechnoTypeExt::ExtMap.Find(pType);
			showTiberium = pExt->Refinery_UseStorage;
		}
	}

	return showTiberium ? 0x709B6E : 0x70A980;
}
