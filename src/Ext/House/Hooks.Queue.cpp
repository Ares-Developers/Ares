#include "Body.h"
#include "../TechnoType/Body.h"
#include "../../Ares.h"

#include <FactoryClass.h>

// =============================
// multiqueue hooks

DEFINE_HOOK(4502F4, BuildingClass_Update_Factory, 6)
{
	GET(BuildingClass *, B, ESI);
	HouseClass * H = B->Owner;

	if(H->Production && !Ares::GlobalControls::AllowParallelAIQueues) {
		HouseExt::ExtData *pData = HouseExt::ExtMap.Find(H);
		BuildingClass **curFactory = nullptr;
		switch(B->Type->Factory) {
			case AbstractType::BuildingType:
				curFactory = &pData->Factory_BuildingType;
				break;
			case AbstractType::UnitType:
				curFactory = B->Type->Naval
				 ? &pData->Factory_NavyType
				 : &pData->Factory_VehicleType;
				break;
			case AbstractType::InfantryType:
				curFactory = &pData->Factory_InfantryType;
				break;
			case AbstractType::AircraftType:
				curFactory = &pData->Factory_AircraftType;
				break;
		}
		if(!curFactory) {
			Game::RaiseError(E_POINTER);
		} else if(!*curFactory) {
			*curFactory = B;
			return 0;
		} else if(*curFactory != B) {
			return 0x4503CA;
		}
	}

	return 0;
}

DEFINE_HOOK(4CA07A, FactoryClass_AbandonProduction, 8)
{
	GET(FactoryClass *, F, ESI);
	HouseClass * H = F->Owner;

	HouseExt::ExtData *pData = HouseExt::ExtMap.Find(H);
	TechnoClass * T = F->Object;
	switch(T->WhatAmI()) {
		case AbstractType::Building:
			pData->Factory_BuildingType = nullptr;
			break;
		case AbstractType::Unit:
			(T->GetTechnoType()->Naval
			 ? pData->Factory_NavyType
			 : pData->Factory_VehicleType) = nullptr;
			break;
		case AbstractType::Infantry:
			pData->Factory_InfantryType = nullptr;
			break;
		case AbstractType::Aircraft:
			pData->Factory_AircraftType = nullptr;
			break;
	}

	return 0;
}

DEFINE_HOOK(444119, BuildingClass_KickOutUnit_UnitType, 6)
{
	GET(UnitClass *, U, EDI);

	GET(BuildingClass *, Factory, ESI);

	HouseExt::ExtData *pData = HouseExt::ExtMap.Find(Factory->Owner);

	(U->Type->Naval
	 ? pData->Factory_NavyType
	 : pData->Factory_VehicleType) = nullptr;
	return 0;
}


DEFINE_HOOK(444131, BuildingClass_KickOutUnit_InfantryType, 6)
{
	GET(HouseClass  *, H, EAX);

	HouseExt::ExtMap.Find(H)->Factory_InfantryType = nullptr;
	return 0;
}

DEFINE_HOOK(44531F, BuildingClass_KickOutUnit_BuildingType, A)
{
	GET(HouseClass  *, H, EAX);

	HouseExt::ExtMap.Find(H)->Factory_BuildingType = nullptr;
	return 0;
}

DEFINE_HOOK(443CCA, BuildingClass_KickOutUnit_AircraftType, A)
{
	GET(HouseClass  *, H, EDX);

	HouseExt::ExtMap.Find(H)->Factory_AircraftType = nullptr;
	return 0;
}

// #1286800: build limit > 1 and queues
DEFINE_HOOK(50B3CB, HouseClass_ShouldDisableCameo_BuildLimit, 6)
{
	GET(FactoryClass*, pFactory, EDI);
	GET(TechnoTypeClass*, pType, ESI);
	GET(int, countQueued, EAX);

	// the object in production is counted twice: it appears in this factory
	// queue, and it is already counted in the house's counters. this only
	// affects positive build limits, for negative ones players could queue up
	// one more than BuildLimit.
	if(auto pObj = pFactory->Object) {
		if(countQueued && pObj->GetType() == pType && pType->BuildLimit > 0) {
			auto abs = pType->WhatAmI();
			// buildings can't queue, and pad aircraft has custom handling
			if(abs == UnitTypeClass::AbsID || abs == InfantryTypeClass::AbsID) {
				R->EAX(countQueued - 1);
			}
		}
	}

	return 0;
}
