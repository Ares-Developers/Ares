#include "HunterSeeker.h"
#include "../../Ares.h"
#include "../../Ext/Side/Body.h"
#include "../../Ext/Rules/Body.h"
#include "../../Ext/Techno/Body.h"

#include "../../Utilities/Helpers.Alex.h"
#include "../../Utilities/INIParser.h"
#include "../../Utilities/TemplateDef.h"

#include <HouseClass.h>

#include <vector>

void SW_HunterSeeker::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// Defaults to HunterSeeker values
	pData->SW_MaxCount = 1;

	pData->EVA_Detected = VoxClass::FindIndex("EVA_HunterSeekerDetected");
	pData->EVA_Ready = VoxClass::FindIndex("EVA_HunterSeekerReady");
	pData->EVA_Activated = VoxClass::FindIndex("EVA_HunterSeekerLaunched");

	pData->SW_AITargetingType = SuperWeaponAITargetingMode::HunterSeeker;
	pData->SW_AffectsHouse = SuperWeaponAffectedHouse::Enemies;

	pData->Text_Ready = CSFText("TXT_RELEASE");

	// hardcoded
	pSW->Action = Action::None;
	pData->SW_RadarEvent = false;
}

void SW_HunterSeeker::LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);

	pData->HunterSeeker_Type.Read(exINI, section, "HunterSeeker.Type");
	pData->HunterSeeker_RandomOnly.Read(exINI, section, "HunterSeeker.RandomOnly");
	pData->HunterSeeker_Buildings.Read(exINI, section, "HunterSeeker.Buildings");

	// hardcoded
	pSW->Action = Action::None;
	pData->SW_RadarEvent = false;
}

bool SW_HunterSeeker::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	HouseClass* pOwner = pThis->Owner;
	auto pExt = SWTypeExt::ExtMap.Find(pThis->Type);

	// get the appropriate hunter seeker type
	UnitTypeClass* pType = pExt->HunterSeeker_Type;
	if(!pType) {
		if(auto pSide = SideClass::Array->GetItemOrDefault(pOwner->SideIndex)) {
			auto pSideExt = SideExt::ExtMap.Find(pSide);
			pType = pSideExt->HunterSeeker;
		}
	}

	// no type found
	if(!pType) {
		Debug::Log(Debug::Severity::Error, "HunterSeeker super weapon \"%s\" could not be launched. "
			"No HunterSeeker unit type set for house \"%ls\".\n", pThis->Type->ID, pOwner->UIName);
		return false;
	}

	// get the appropriate launch buildings list
	auto HSBuilding = &RulesExt::Global()->HunterSeekerBuildings;
	if(pExt->HunterSeeker_Buildings.size()) {
		HSBuilding = &pExt->HunterSeeker_Buildings;
	}

	auto IsEligible = [&HSBuilding](BuildingClass* pBld) {
		return HSBuilding->Contains(pBld->Type);
	};

	// the maximum number of buildings to fire. negative means all.
	const auto Count = (pExt->SW_MaxCount >= 0)
		? static_cast<size_t>(pExt->SW_MaxCount)
		: std::numeric_limits<size_t>::max();

	// only call on up to Count buildings that suffice IsEligible
	size_t Success = 0;
	Helpers::Alex::for_each_if_n(pOwner->Buildings.begin(), pOwner->Buildings.end(),
		Count, IsEligible, [=, &Success](BuildingClass* pBld)
	{
		auto cell = this->GetLaunchCell(pExt, pBld);

		if(cell == CellStruct::Empty) {
			return;
		}

		// create a hunter seeker
		if(auto pHunter = GameCreate<UnitClass>(pType, pOwner)) {
			auto pData = TechnoExt::ExtMap.Find(pHunter);
			pData->SuperWeapon = pThis;

			// put it on the map and let it go
			CoordStruct crd = CellClass::Cell2Coord(cell);

			if(pHunter->Put(crd, 64)) {
				pHunter->Locomotor->Acquire_Hunter_Seeker_Target();
				pHunter->QueueMission(Mission::Attack, false);
				pHunter->NextMission();
				++Success;
			} else {
				GameDelete(pHunter);
			}
		}
	});

	// no launch building found
	if(!Success) {
		Debug::Log(Debug::Severity::Error, "HunterSeeker super weapon \"%s\" could not be launched. House \"%ls\" "
			"does not own any HSBuilding (%u types set).\n", pThis->Type->ID, pOwner->UIName, HSBuilding->size());
	}

	return Success != 0;
}

CellStruct SW_HunterSeeker::GetLaunchCell(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const
{
	auto position = CellClass::Coord2Cell(pBuilding->GetCoords());

	auto cell = MapClass::Instance->Pathfinding_Find(position, SpeedType::Foot,
		-1, MovementZone::Normal, false, 1, 1, false, false, false, true,
		CellStruct::Empty, false, false);

	return MapClass::Instance->IsWithinUsableArea(cell, true) ? cell : CellStruct::Empty;
}
