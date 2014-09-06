#include "Body.h"

#include "../BuildingType/Body.h"

#include <InfantryClass.h>
#include <AircraftClass.h>

// maintain the houses' academy lists

DEFINE_HOOK(446366, BuildingClass_Place_Academy, 6)
{
	GET(BuildingClass*, pThis, EBP);
	auto pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if(pExt->IsAcademy()) {
		if(auto pData = HouseExt::ExtMap.Find(pThis->Owner)) {
			pData->UpdateAcademy(pThis, true);
		}
	}

	return 0;
}

DEFINE_HOOK(445905, BuildingClass_Remove_Academy, 6)
{
	GET(BuildingClass*, pThis, ESI);
	auto pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if(pThis->IsOnMap && pExt->IsAcademy()) {
		if(auto pData = HouseExt::ExtMap.Find(pThis->Owner)) {
			pData->UpdateAcademy(pThis, false);
		}
	}

	return 0;
}

DEFINE_HOOK(448AB2, BuildingClass_ChangeOwnership_Remove_Academy, 6)
{
	GET(BuildingClass*, pThis, ESI);
	auto pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if(pThis->IsOnMap && pExt->IsAcademy()) {
		if(auto pData = HouseExt::ExtMap.Find(pThis->Owner)) {
			pData->UpdateAcademy(pThis, false);
		}
	}

	return 0;
}

DEFINE_HOOK(4491D5, BuildingClass_ChangeOwnership_Add_Academy, 6)
{
	GET(BuildingClass*, pThis, ESI);
	auto pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if(pExt->IsAcademy()) {
		if(auto pData = HouseExt::ExtMap.Find(pThis->Owner)) {
			pData->UpdateAcademy(pThis, true);
		}
	}

	return 0;
}


// apply the academy effect
// TODO: consider Organic, NotHuman(?), and ConsiderAircraft

DEFINE_HOOK(517D51, InfantryClass_Init_Academy, 6)
{
	GET(InfantryClass*, pThis, ESI);

	if(auto pExt = HouseExt::ExtMap.Find(pThis->Owner)) {
		pExt->ApplyAcademy(pThis, pExt->AcademyInfantry);
	}

	return 0;
}

DEFINE_HOOK_AGAIN(735678, UnitClass_Init_Academy, 6) // inlined in CTOR
DEFINE_HOOK(74689B, UnitClass_Init_Academy, 6)
{
	GET(UnitClass*, pThis, ESI);

	if(auto pExt = HouseExt::ExtMap.Find(pThis->Owner)) {
		pExt->ApplyAcademy(pThis, pExt->AcademyVehicle);
	}

	return 0;
}

DEFINE_HOOK(413FD2, AircraftClass_Init_Academy, 6)
{
	GET(AircraftClass*, pThis, ESI);

	if(auto pExt = HouseExt::ExtMap.Find(pThis->Owner)) {
		pExt->ApplyAcademy(pThis, pExt->AcademyAircraft);
	}

	return 0;
}

DEFINE_HOOK(442D1B, BuildingClass_Init_Academy, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if(auto pExt = HouseExt::ExtMap.Find(pThis->Owner)) {
		pExt->ApplyAcademy(pThis, pExt->AcademyBuilding);
	}

	return 0;
}
