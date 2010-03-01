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
DEFINE_HOOK(448277, BuildingClass_UnloadPassengers_ChangeOwner_Sell, 5)
DEFINE_HOOK_AGAIN(447113, BuildingClass_UnloadPassengers_ChangeOwner_Sell, 6)
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
