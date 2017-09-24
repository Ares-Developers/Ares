#include "Body.h"

#include "../TechnoType/Body.h"

#include <HouseClass.h>
#include <RadarEventClass.h>
#include <VoxClass.h>

DEFINE_HOOK(70DA95, TechnoClass_RadarTrackingUpdate_AnnounceDetected, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(int, detect, 0x10);

	auto pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if(detect && pTypeExt->SensorArray_Warn) {
		switch(detect) {
		case 1:
			VoxClass::Play("EVA_CloakedUnitDetected");
			break;
		case 2:
			VoxClass::Play("EVA_SubterraneanUnitDetected");
			break;
		}

		CellStruct cell = CellClass::Coord2Cell(pThis->Location);
		RadarEventClass::Create(RadarEventType::EnemySensed, cell);
	}

	return 0x70DADC;
}

DEFINE_HOOK(455828, BuildingClass_SensorArrayActivate, 8)
{
	GET(BuildingClass*, pBld, ECX);
	auto pExt = BuildingExt::ExtMap.Find(pBld);

	// only add sensor if ActiveCount was zero
	if(pBld->IsPowerOnline() && !pBld->Deactivated && !pExt->SensorArrayActiveCounter++) {
		return 0x455838;
	}

	return 0x45596F;
}

DEFINE_HOOK(4556E1, BuildingClass_SensorArrayDeactivate, 7)
{
	GET(BuildingClass*, pBld, ECX);
	auto pExt = BuildingExt::ExtMap.Find(pBld);

	// don't do the same work twice
	if(pExt->SensorArrayActiveCounter > 0) {
		pExt->SensorArrayActiveCounter = 0;

		// this fixes the issue where the removed area does not match the
		// added area. adding uses SensorsSight, so we remove that one here.
		R->EBP(pBld->Type->SensorsSight);

		return 0x4556E8;
	}

	return 0x45580D;
}

DEFINE_HOOK_AGAIN(4557BC, BuildingClass_SensorArray_BuildingRedraw, 6)
DEFINE_HOOK(455923, BuildingClass_SensorArray_BuildingRedraw, 6)
{
	GET(CellClass*, pCell, ESI);

	// mark detected buildings for redraw
	if(auto pBld = pCell->GetBuilding()) {
		if(pBld->Owner != HouseClass::Player && pBld->VisualCharacter(VARIANT_FALSE, nullptr) != VisualType::Normal) {
			pBld->NeedsRedraw = true;
		}
	}

	return 0;
}

// powered state changed
DEFINE_HOOK_AGAIN(454B5F, BuildingClass_UpdatePowered_SensorArray, 6)
DEFINE_HOOK(4549F8, BuildingClass_UpdatePowered_SensorArray, 6)
{
	GET(BuildingClass*, pBld, ESI);
	auto pExt = BuildingExt::ExtMap.Find(pBld);
	pExt->UpdateSensorArray();
	return 0;
}

// something changed to the worse, like toggle power
DEFINE_HOOK(4524A3, BuildingClass_DisableThings, 6)
{
	GET(BuildingClass*, pBld, EDI);
	auto pExt = BuildingExt::ExtMap.Find(pBld);
	pExt->UpdateSensorArray();
	return 0;
}

// check every frame
DEFINE_HOOK(43FE69, BuildingClass_Update_SensorArray, A)
{
	GET(BuildingClass*, pBld, ESI);
	auto pExt = BuildingExt::ExtMap.Find(pBld);
	pExt->UpdateSensorArray();
	return 0;
}

// capture and mind-control support: deactivate the array for the original
// owner, then activate it a few instructions later for the new owner.
DEFINE_HOOK(448B70, BuildingClass_ChangeOwnership_SensorArrayA, 6)
{
	GET(BuildingClass*, pBld, ESI);
	if(pBld->Type->SensorArray) {
		pBld->SensorArrayDeactivate();
	}
	return 0;
}

DEFINE_HOOK(448C3E, BuildingClass_ChangeOwnership_SensorArrayB, 6)
{
	GET(BuildingClass*, pBld, ESI);
	if(pBld->Type->SensorArray) {
		pBld->SensorArrayActivate();
	}
	return 0;
}

// remove sensor on destruction
DEFINE_HOOK(4416A2, BuildingClass_Destroy_SensorArray, 6)
{
	GET(BuildingClass*, pBld, ESI);
	if(pBld->Type->SensorArray) {
		pBld->SensorArrayDeactivate();
	}
	return 0;
}

// sensor arrays show SensorsSight instead of CloakRadiusInCells
DEFINE_HOOK(4566F9, BuildingClass_GetRangeOfRadial_SensorArray, 6)
{
	GET(BuildingClass*, pThis, ESI);
	auto pType =  pThis->Type;

	if(pType->SensorArray) {
		R->EAX(pType->SensorsSight);
		return 0x45674B;
	}

	return 0x456703;
}
