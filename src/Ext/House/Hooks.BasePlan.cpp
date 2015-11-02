#include "Body.h"
#include "../BuildingType/Body.h"
#include "../HouseType/Body.h"
#include "../Side/Body.h"

#include <BuildingTypeClass.h>
#include <ScenarioClass.h>

// #917 - validate build list before it needs to be generated
DEFINE_HOOK(5054B0, HouseClass_GenerateAIBuildList_EnsureSanity, 6)
{
	GET(HouseClass* const, pThis, ECX);

	auto const pExt = HouseExt::ExtMap.Find(pThis);
	pExt->CheckBasePlanSanity();

	// allow the list to be generated even if it will crash the game - sanity
	// check will log potential problems and thou shalt RTFLog
	return 0;
}

// fixes SWs not being available in campaigns if they have been turned off in a
// multiplayer mode
DEFINE_HOOK(5055D8, HouseClass_GenerateAIBuildList_SWAllowed, 5)
{
	auto const allowed = SessionClass::Instance->GameMode == GameMode::Campaign
		|| GameModeOptionsClass::Instance->SWAllowed;

	R->EAX(allowed);
	return 0x5055DD;
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
	REF_STACK(DynamicVectorClass<BuildingTypeClass*>, PlannedBase1, STACK_OFFS(0xA4, 0x90));
	REF_STACK(DynamicVectorClass<BuildingTypeClass*>, PlannedBase2, STACK_OFFS(0xA4, 0x78));
	PlannedBase2.SetCapacity(PlannedBase1.Capacity, nullptr);
	return 0x505C2C;
}

DEFINE_HOOK(505C34, HouseClass_GenerateAIBuildList_FullAutoCopy, 5)
{
	R->EDI(0);
	return 0x505C39;
}

DEFINE_HOOK(505C95, HouseClass_GenerateAIBuildList_CountExtra, 7)
{
	GET(HouseClass* const, pThis, EBX);
	REF_STACK(DynamicVectorClass<BuildingTypeClass*>, BuildList, STACK_OFFS(0xA4, 0x78));

	auto const idxDifficulty = pThis->GetAIDifficultyIndex();
	auto& Random = ScenarioClass::Instance->Random;

	// optionally add the same buildings more than once, but ignore the
	// construction yard at index 0
	for(auto i = 1; i < BuildList.Count; ++i) {
		auto const pItem = BuildList[i];

		// only handle if occurs for the first time, otherwise we have an
		// escalating probability of desaster.
		auto const handled = make_iterator(BuildList.begin(), i);

		if(!handled.contains(pItem)) {
			auto const pExt = BuildingTypeExt::ExtMap.Find(pItem);
			if(idxDifficulty < pExt->AIBuildCounts.size()) {
				// fixed number of buildings, one minimum (exists already)
				auto count = Math::max(pExt->AIBuildCounts[idxDifficulty], 1);

				// random optional building counts
				if(idxDifficulty < pExt->AIExtraCounts.size()) {
					auto const& max = pExt->AIExtraCounts[idxDifficulty];
					count += Random.RandomRanged(0, Math::max(max, 0));
				}

				// account for the one that already exists
				for(auto j = 1; j < count; ++j) {
					auto const idx = Random.RandomRanged(
						i + 1, BuildList.Count);
					BuildList.AddItem(pItem);
					std::rotate(BuildList.begin() + idx, BuildList.end() - 1,
						BuildList.end());
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(505C95, HouseClass_GenerateAIBuildList_BaseDefenseCounts, 7)
{
	GET(HouseClass* const, pThis, EBX);
	GET_STACK(int const, idxSide, 0x80);

	if(auto const pSide = SideClass::Array->GetItemOrDefault(idxSide)) {
		auto const pExt = SideExt::ExtMap.Find(pSide);
		
		auto const idxDiff = pThis->GetAIDifficultyIndex();

		auto const it = pExt->GetBaseDefenseCounts();
		if(idxDiff < it.size()) {
			R->EAX(it.at(idxDiff));
			return 0x505CE9;
		} else {
			Debug::Log("WTF! vector has %u items, requested item #%u\n",
				it.size(), idxDiff);
		}
	}
	return 0;
}

// I am crying all inside
DEFINE_HOOK(505CF1, HouseClass_GenerateAIBuildList_PadWithN1, 5)
{
	REF_STACK(DynamicVectorClass<BuildingTypeClass*>, PlannedBase2, STACK_OFFS(0xA4, 0x78));
	GET(int, DefenseCount, EAX);
	while(PlannedBase2.Count <= 3) {
		PlannedBase2.AddItem(reinterpret_cast<BuildingTypeClass*>(-1));
		--DefenseCount;
	}
	R->EDI(DefenseCount);
	R->EBX(-1);
	return (DefenseCount > 0) ? 0x505CF6u : 0x505D8Du;
}

// replaced the entire function, to have one centralized implementation
DEFINE_HOOK(5051E0, HouseClass_FirstBuildableFromArray, 5)
{
	GET(HouseClass const* const, pThis, ECX);
	GET_STACK(const DynamicVectorClass<BuildingTypeClass*>* const, pList, 0x4);

	auto const idxParentCountry = pThis->Type->FindParentCountryIndex();
	auto const pItem = HouseExt::FindBuildable(
		pThis, idxParentCountry, make_iterator(*pList));

	R->EAX(pItem);
	return 0x505300;
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

// don't crash if you can't find a base unit
// I imagine we'll have a pile of hooks like this sooner or later
DEFINE_HOOK(4F65BF, HouseClass_CanAffordBase, 6)
{
	GET(UnitTypeClass*, pBaseUnit, ECX);
	if(pBaseUnit) {
		return 0;
	}
//	GET(HouseClass *, pHouse, ESI);
//	Debug::Log(Debug::Error, "AI House of country [%s] cannot build anything from [General]BaseUnit=.\n", pHouse->Type->ID);
	return 0x4F65DA;
}

DEFINE_HOOK(5D705E, MPGameMode_SpawnBaseUnit, 6)
{
	enum { hasBaseUnit = 0x5D7084, hasNoBaseUnit = 0x5D70DB };

	GET(HouseClass *, pHouse, EDI);
	GET(UnitTypeClass *, pBaseUnit, EAX);
	if(!pBaseUnit) {
		Debug::Log(Debug::Severity::Fatal, "House of country [%s] cannot build anything from [General]BaseUnit=.\n", pHouse->Type->ID);
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
		Debug::Log(Debug::Severity::Fatal, "House of country [%s] cannot build anything from [General]BaseUnit=.\n", pHouse->Type->ID);

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
		Debug::Log(Debug::Severity::Fatal, "House of country [%s] cannot build anything from [General]BaseUnit=.\n", pHouse->Type->ID);
	}
	return 0;
}
