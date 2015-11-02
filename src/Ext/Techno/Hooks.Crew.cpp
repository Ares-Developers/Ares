#include "Body.h"

#include "../House/Body.h"
#include "../Side/Body.h"
#include "../TechnoType/Body.h"

#include <ScenarioClass.h>

DEFINE_HOOK(6F3950, TechnoClass_GetCrewCount, 8)
{
	GET(TechnoClass*, pThis, ECX);
	auto pType = pThis->GetTechnoType();

	// previous default for crew count was -1
	int count = -1;
	if(auto pExt = TechnoTypeExt::ExtMap.Find(pType)) {
		count = pExt->Survivors_PilotCount;
	}

	// default to original formula
	if(count < 0) {
		count = pType->Crewed ? 1 : 0;
	}

	R->EAX(count);
	return 0x6F3967;
}

DEFINE_HOOK(451330, BuildingClass_GetCrewCount, A)
{
	GET(BuildingClass*, pThis, ECX);

	int count = 0;

	if(!pThis->NoCrew && pThis->Type->Crewed) {
		auto pHouse = pThis->Owner;

		// get the divisor
		auto pExt = HouseExt::ExtMap.Find(pHouse);
		int divisor = pExt->GetSurvivorDivisor();

		if(divisor > 0) {
			// if captured, less survivors
			if(pThis->HasBeenCaptured) {
				divisor *= 2;
			}

			// value divided by "cost per survivor"
			count = pThis->Type->GetRefund(pHouse, 0) / divisor;

			// clamp between 1 and 5
			if(count < 1) {
				count = 1;
			}
			if(count > 5) {
				count = 5;
			}
		}
	}

	R->EAX(count);
	return 0x4513CD;
}

DEFINE_HOOK(707D20, TechnoClass_GetCrew, 5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	auto pHouse = pThis->Owner;
	auto pHouseExt = HouseExt::ExtMap.Find(pHouse);

	InfantryTypeClass* pCrewType = nullptr;

	// YR defaults to 15 for armed objects,
	int TechnicianChance = pThis->IsArmed() ? 15 : 0;

	// Ares < 0.5 defaulted to 0 for non-buildings.
	if(abstract_cast<FootClass*>(pThis)) {
		TechnicianChance = 0;
	}
	TechnicianChance = pExt->Crew_TechnicianChance.Get(TechnicianChance);

	if(pType->Crewed) {
		// for civilian houses always technicians. random for others
		bool isTechnician = false;
		if(pHouse->Type->SideIndex == -1) {
			isTechnician = true;
		} else if(TechnicianChance > 0 && ScenarioClass::Instance->Random.RandomRanged(0, 99) < TechnicianChance) {
			isTechnician = true;
		}

		// chose the appropriate type
		if(!isTechnician) {
			// customize with this techno's pilot type
			auto& pPilots = pExt->Survivors_Pilots;
			int index = pHouse->SideIndex;

			// only use it if non-null, as documented
			if(auto pPilotType = pPilots.GetItemOrDefault(index)) {
				pCrewType = pPilotType;
			} else {
				// get the side's crew
				pCrewType = pHouseExt->GetCrew();
			}
		} else {
			// either civilian side or chance
			pCrewType = pHouseExt->GetTechnician();
		}
	}

	R->EAX(pCrewType);
	return 0x707DCF;
}

DEFINE_HOOK(44EB10, BuildingClass_GetCrew, 9)
{
	GET(BuildingClass*, pThis, ECX);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	InfantryTypeClass* pType = nullptr;

	// YR defaults to 25 for buildings producing buildings
	int EngineerChance = (pThis->Type->Factory == BuildingTypeClass::AbsID) ? 25 : 0;
	EngineerChance = pTypeExt->Crew_EngineerChance.Get(EngineerChance);

	// with some luck, and if the building has not been captured, spawn an engineer
	if(!pThis->HasBeenCaptured
		&& EngineerChance > 0
		&& ScenarioClass::Instance->Random.RandomRanged(0, 99) < EngineerChance)
	{
		auto pExt = HouseExt::ExtMap.Find(pThis->Owner);
		pType = pExt->GetEngineer();
	} else {
		// call base
		SET_REG32(ECX, pThis);
		CALL(0x707D20);
		GET_REG32(pType, EAX);
	}

	R->EAX(pType);
	return 0x44EB5B;
}
