#include "Body.h"
#include "../BuildingType/Body.h"
#include "../Techno/Body.h"
#include "../../Misc/Network.h"

#include <SpecificStructures.h>
#include <ScenarioClass.h>
#include <InfantryClass.h>
#include <CellClass.h>

#include <cmath>

/* #754 - evict Hospital/Armory contents */
DEFINE_HOOK(0x448277, BuildingClass_UnloadPassengers_ChangeOwner_SellAndLeaveBomb, 0x5)
{
	GET(BuildingClass *, B, ESI);

	BuildingExt::KickOutHospitalArmory(B);
	return 0x448293;
}

DEFINE_HOOK(0x447113, BuildingClass_UnloadPassengers_ChangeOwner_Sell, 0x6)
{
	GET(BuildingClass *, B, ESI);

	BuildingExt::KickOutHospitalArmory(B);
	return 0;
}

DEFINE_HOOK(0x44D8A1, BuildingClass_UnloadPassengers_Unload, 0x6)
{
	GET(BuildingClass *, B, EBP);

	BuildingExt::KickOutHospitalArmory(B);
	return 0;
}


/* 	#218 - specific occupiers -- see Hooks.Trenches.cpp */

// EMP'd power plants don't produce power
DEFINE_HOOK(0x44E855, BuildingClass_PowerProduced_EMP, 0x6) {
	GET(BuildingClass*, pBld, ESI);
	return ((pBld->EMPLockRemaining > 0) ? 0x44E873 : 0);
}
