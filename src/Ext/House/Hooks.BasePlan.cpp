#include "Body.h"
#include "../HouseType/Body.h"
#include <BuildingTypeClass.h>

// #917 - validate build list before it needs to be generated
DEFINE_HOOK(5054B0, HouseClass_GenerateAIBuildList_EnsureSanity, 6)
{
	GET(HouseClass *, pHouse, ECX);
	auto pData = HouseExt::ExtMap.Find(pHouse);
	pData->CheckBasePlanSanity();
	return 0;// allow the list to be generated even if it will crash the game - sanity check will log potential problems and thou shalt RTFLog
}

// #917 - handle the case of no shipyard gracefully
DEFINE_HOOK(50610E, HouseClass_FindPositionForBuilding_FixShipyard, 7)
{
	GET(BuildingTypeClass *, pShipyard, EAX);
	if(pShipyard) {
		R->ESI<int>(pShipyard->GetFoundationWidth() + 2);
		R->EAX<int>(pShipyard->GetFoundationHeight(false));
		return 0x506134;
	} else {
		return 0x5060CE;
	}
}

// #917 - stupid copying logic
/**
 * v2[0] = v1[0];
 * v2[1] = v1[1];
 * v2[2] = v1[2];
 * for(int i = 3; i < v1.Count; ++i) {
 *  v2[i] = v1[i];
 * }
 * care to guess what happens when v1.Count is < 3?
 *
 * fixed old fix, which was quite broken itself...
 */

DEFINE_HOOK(505B58, HouseClass_GenerateAIBuildList_SkipManualCopy, 6)
{
	LEA_STACK(DynamicVectorClass<BuildingTypeClass *> *, PlannedBase1, STACK_OFFS(0xA4, 0x90));
	LEA_STACK(DynamicVectorClass<BuildingTypeClass *> *, PlannedBase2, STACK_OFFS(0xA4, 0x78));
	PlannedBase2->SetCapacity(PlannedBase1->Capacity, nullptr);
	return 0x505C2C;
}

DEFINE_HOOK(505C34, HouseClass_GenerateAIBuildList_FullAutoCopy, 5)
{
	R->EDI<int>(0);
	return 0x505C39;
}

// I am crying all inside
DEFINE_HOOK(505CF1, HouseClass_GenerateAIBuildList_PadWithN1, 5)
{
	LEA_STACK(DynamicVectorClass<BuildingTypeClass *> *, PlannedBase2, STACK_OFFS(0xA4, 0x78));
	GET(int, DefenseCount, EAX);
	while(PlannedBase2->Count <= 3) {
		PlannedBase2->AddItem(reinterpret_cast<BuildingTypeClass *>(-1));
		--DefenseCount;
	}
	R->EDI<int>(DefenseCount);
	R->EBX<int>(-1);
	return (DefenseCount > 0)
		? 0x505CF6
		: 0x505D8D
	;
}

// don't crash if you can't find a base unit
// I imagine we'll have a pile of hooks like this sooner or later
DEFINE_HOOK(4F65BF, HouseClass_CanAffordBase, 6)
{
	GET(UnitTypeClass*, pBaseUnit, ECX);
	if(pBaseUnit) {
		return 0;
	}
//	GET(HouseClass *, pHouse, ESI);
//	Debug::DevLog(Debug::Error, "AI House of country [%s] cannot build anything from [General]BaseUnit=.\n", pHouse->Type->ID);
	return 0x4F65DA;
}

DEFINE_HOOK(5D705E, MPGameMode_SpawnBaseUnit, 6)
{
	enum { hasBaseUnit = 0x5D7084, hasNoBaseUnit = 0x5D70DB };

	GET(HouseClass *, pHouse, EDI);
	GET(UnitTypeClass *, pBaseUnit, EAX);
	if(!pBaseUnit) {
		Debug::DevLog(Debug::Severity::Error, "House of country [%s] cannot build anything from [General]BaseUnit=.\n", pHouse->Type->ID);
		return hasNoBaseUnit;
	}

	auto Unit = static_cast<UnitClass *>(pBaseUnit->CreateObject(pHouse));
	R->ESI<UnitClass *>(Unit);
	return hasBaseUnit;
}

DEFINE_HOOK(688B37, MPGameModeClass_CreateStartingUnits_B, 5)
{
	enum { hasBaseUnit = 0x688B75, hasNoBaseUnit = 0x688C09 };

	GET_STACK(HouseClass *, pHouse, 0x10);

	auto pArray = &RulesClass::Instance->BaseUnit;
	bool canBuild = false;
	UnitTypeClass* Item = nullptr;
	auto const idxParent = pHouse->Type->FindParentCountryIndex();
	for(int i = 0; i < pArray->Count; ++i) {
		Item = pArray->GetItem(i);
		if(pHouse->CanExpectToBuild(Item, idxParent)) {
			canBuild = true;
			break;
		}
	}
	if(!canBuild) {
		Debug::DevLog(Debug::Severity::Error, "House of country [%s] cannot build anything from [General]BaseUnit=. \n", pHouse->Type->ID);

		return hasNoBaseUnit;
	}

	auto Unit = static_cast<UnitClass *>(Item->CreateObject(pHouse));
	R->ESI<UnitClass *>(Unit);
	R->EBP(0);
	R->EDI<HouseClass *>(pHouse);
	return hasBaseUnit;
}

DEFINE_HOOK(5D721A, MPGameMode_CreateStartingUnits, 5)
{
	GET_STACK(int, UnitCount, 0x40);
	GET_STACK(HouseClass*, pHouse, 0x4C);
	if(!UnitCount) {
		Debug::DevLog(Debug::Severity::Error, "House of country [%s] cannot build anything from [General]BaseUnit=. \n", pHouse->Type->ID);
	}
	return 0;
}

// fixes SWs not being available in campaigns if they have been turned off in a multiplayer mode

DEFINE_HOOK(505288, HouseClass_FindFirstBuildableBuildingTypeFromArray_SWAllowed, 6)
{
	R->CL(SessionClass::Instance->GameMode == GameMode::Campaign || GameModeOptionsClass::Instance->SWAllowed);
	return 0x50528E;
}

DEFINE_HOOK(5055D8, HouseClass_GenerateAIBuildList_SWAllowed, 5)
{
	R->AL(SessionClass::Instance->GameMode == GameMode::Campaign || GameModeOptionsClass::Instance->SWAllowed);
	return 0x5055DD;
}
