#include "HunterSeeker.h"
#include "../../Ares.h"
#include "../../Ext/Side/Body.h"
#include "../../Ext/Rules/Body.h"
#include "../../Ext/Techno/Body.h"

#include "../../Utilities/INIParser.h"

#include <HouseClass.h>

#include <vector>

void SW_HunterSeeker::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// Defaults to HunterSeeker values
	pData->HunterSeeker_Type = nullptr;
	pData->HunterSeeker_RandomOnly = false;

	pData->EVA_Detected = VoxClass::FindIndex("EVA_HunterSeekerDetected");
	pData->EVA_Ready = VoxClass::FindIndex("EVA_HunterSeekerReady");
	pData->EVA_Activated = VoxClass::FindIndex("EVA_HunterSeekerLaunched");

	pData->SW_AITargetingType = SuperWeaponAITargetingMode::NoTarget;
	pData->SW_AffectsHouse = SuperWeaponAffectedHouse::Enemies;

	pData->Text_Ready = CSFText("TXT_RELEASE");

	// hardcoded
	pSW->Action = 0;
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
	pSW->Action = 0;
	pData->SW_RadarEvent = false;
}

bool SW_HunterSeeker::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	HouseClass* pOwner = pThis->Owner;
	auto pExt = SWTypeExt::ExtMap.Find(pThis->Type);

	BuildingClass* pBld = nullptr;
	CellStruct cell = CellStruct::Empty;

	// get the appropriate launch buildings list
	auto HSBuilding = &RulesExt::Global()->HunterSeekerBuildings;
	if(pExt->HunterSeeker_Buildings.size()) {
		HSBuilding = &pExt->HunterSeeker_Buildings;
	}

	// find a hunter seeker building that can launch me
	for(auto i : pOwner->Buildings) {

		if(HSBuilding->Contains(i->Type)) {
			// verify the building coordinates
			CoordStruct crd = i->GetCoords();

			CellStruct tmp = CellClass::Coord2Cell(crd);
			cell = MapClass::Instance->Pathfinding_Find(tmp, SpeedType::Foot, -1, MovementZone::Normal, false, 1, 1, false, false, false, true, CellStruct::Empty, false, false);

			if(MapClass::Instance->IsWithinUsableArea(cell, true)) {
				pBld = i;
				break;
			}
		}
	}

	// no launch building found
	if(!pBld) {
		Debug::DevLog(Debug::Warning, "HunterSeeker super weapon \"%s\" could not be launched. House \"%ls\" "
			"does not own any HSBuilding (%d types set).\n", pThis->Type->ID, pOwner->UIName, HSBuilding->size());
		return false;
	}

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
		Debug::DevLog(Debug::Warning, "HunterSeeker super weapon \"%s\" could not be launched. "
			"No HunterSeeker unit type set for house \"%ls\".\n", pThis->Type->ID, pOwner->UIName);
		return false;
	}

	// create a hunter seeker
	auto pHunter = GameCreate<UnitClass>(pType, pOwner);

	// put it on the map and let it go
	if(pHunter) {
		auto pData = TechnoExt::ExtMap.Find(pHunter);
		pData->HunterSeekerSW = pThis;

		CoordStruct crd = CellClass::Cell2Coord(cell);

		if(pHunter->Put(crd, 64)) {
			if(!pHunter->Locomotor) {
				Game::RaiseError(E_POINTER);
			}

			pHunter->Locomotor->Acquire_Hunter_Seeker_Target();

			pHunter->QueueMission(mission_Attack, false);
			pHunter->NextMission();
		} else {
			GameDelete(pHunter);
		}
	}

	return true;
}
