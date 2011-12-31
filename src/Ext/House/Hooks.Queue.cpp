#include "Body.h"
#include "../TechnoType/Body.h"
#include "../../Ares.h"

// =============================
// multiqueue hooks

DEFINE_HOOK(0x4502F4, BuildingClass_Update_Factory, 0x6)
{
	GET(BuildingClass *, B, ESI);
	HouseClass * H = B->Owner;

	if(H->Production && !Ares::GlobalControls::AllowParallelAIQueues) {
		HouseExt::ExtData *pData = HouseExt::ExtMap.Find(H);
		BuildingClass **curFactory = NULL;
		switch(B->Type->Factory) {
			case abs_BuildingType:
				curFactory = &pData->Factory_BuildingType;
				break;
			case abs_UnitType:
				curFactory = B->Type->Naval
				 ? &pData->Factory_NavyType
				 : &pData->Factory_VehicleType;
				break;
			case abs_InfantryType:
				curFactory = &pData->Factory_InfantryType;
				break;
			case abs_AircraftType:
				curFactory = &pData->Factory_AircraftType;
				break;
		}
		if(!*curFactory) {
			*curFactory = B;
			return 0;
		} else if(*curFactory != B) {
			return 0x4503CA;
		}
	}

	return 0;
}

DEFINE_HOOK(0x4CA07A, FactoryClass_AbandonProduction, 0x8)
{
	GET(FactoryClass *, F, ESI);
	HouseClass * H = F->Owner;

	HouseExt::ExtData *pData = HouseExt::ExtMap.Find(H);
	TechnoClass * T = F->InProduction;
	switch(T->WhatAmI()) {
		case abs_Building:
			pData->Factory_BuildingType = NULL;
			break;
		case abs_Unit:
			(T->GetTechnoType()->Naval
			 ? pData->Factory_NavyType
			 : pData->Factory_VehicleType) = NULL;
			break;
		case abs_Infantry:
			pData->Factory_InfantryType = NULL;
			break;
		case abs_Aircraft:
			pData->Factory_AircraftType = NULL;
			break;
	}

	return 0;
}

DEFINE_HOOK(0x444119, BuildingClass_KickOutUnit_UnitType, 0x6)
{
	GET(UnitClass *, U, EDI);

	GET(BuildingClass *, Factory, ESI);

	HouseExt::ExtData *pData = HouseExt::ExtMap.Find(Factory->Owner);

	(U->Type->Naval
	 ? pData->Factory_NavyType
	 : pData->Factory_VehicleType) = NULL;
	return 0;
}


DEFINE_HOOK(0x444131, BuildingClass_KickOutUnit_InfantryType, 0x6)
{
	GET(HouseClass  *, H, EAX);

	HouseExt::ExtMap.Find(H)->Factory_InfantryType = NULL;
	return 0;
}

DEFINE_HOOK(0x44531F, BuildingClass_KickOutUnit_BuildingType, 0xA)
{
	GET(HouseClass  *, H, EAX);

	HouseExt::ExtMap.Find(H)->Factory_BuildingType = NULL;
	return 0;
}

DEFINE_HOOK(0x443CCA, BuildingClass_KickOutUnit_AircraftType, 0xA)
{
	GET(HouseClass  *, H, EDX);

	HouseExt::ExtMap.Find(H)->Factory_AircraftType = NULL;
	return 0;
}
