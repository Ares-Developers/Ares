#include "DropPod.h"

#include "../../Ext/Techno/Body.h"
#include "../../Ext/Rules/Body.h"

#include "../../Utilities/INIParser.h"
#include "../../Utilities/TemplateDef.h"

#include <BuildingTypeClass.h>
#include <HouseClass.h>
#include <VoxClass.h>
#include <ScenarioClass.h>

void SW_DropPod::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	pData->EVA_Detected = VoxClass::FindIndex("EVA_DropPodDetected");
	pData->EVA_Ready = VoxClass::FindIndex("EVA_DropPodReady");
	pData->EVA_Activated = VoxClass::FindIndex("EVA_DropPodActivated");

	pData->SW_AITargetingType = SuperWeaponAITargetingMode::ParaDrop;
	pData->SW_Cursor = MouseCursor::GetCursor(MouseCursorType::ParaDrop);
}

void SW_DropPod::LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);
	pData->DropPod_Minimum.Read(exINI, section, "DropPod.Minimum");
	pData->DropPod_Maximum.Read(exINI, section, "DropPod.Maximum");
	pData->DropPod_Veterancy.Read(exINI, section, "DropPod.Veterancy");
	pData->DropPod_Types.Read(exINI, section, "DropPod.Types");
}

bool SW_DropPod::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	SuperWeaponTypeClass *pSW = pThis->Type;
	auto pData = SWTypeExt::ExtMap.Find(pSW);

	HouseClass* pOwner = pThis->Owner;
	CellStruct cell = Coords;

	// collect the options
	auto const& Types = !pData->DropPod_Types.empty()
		? pData->DropPod_Types
		: RulesExt::Global()->DropPodTypes;
	int cMin = pData->DropPod_Minimum.Get(RulesExt::Global()->DropPodMinimum);
	int cMax = pData->DropPod_Maximum.Get(RulesExt::Global()->DropPodMaximum);
	double veterancy = std::min(pData->DropPod_Veterancy.Get(), RulesClass::Instance->VeteranCap);

	// quick way out
	if(Types.empty()) {
		Debug::Log(Debug::Severity::Error, "DropPod super weapon \"%s\" could not be launched. House \"%ls\" "
			"does not have any types to drop set.\n", pSW->ID, pOwner->UIName);
		return false;
	}

	// three times more tries than units to place.
	int count = ScenarioClass::Instance->Random.RandomRanged(cMin, cMax);
	for(int i = 3 * count; i; --i) {

		// get a random type from the list and create an instance
		int index = ScenarioClass::Instance->Random.RandomRanged(0, Types.size() - 1);
		TechnoTypeClass* pType = Types[index];

		if(pType->WhatAmI() == BuildingTypeClass::AbsID) {
			Debug::Log(Debug::Severity::Error, "DropPod super weapon \"%s\" contains building types. Aborting.\n", pSW->ID);
			break;
		}

		FootClass* pFoot = static_cast<FootClass*>(pType->CreateObject(pOwner));

		// update veterancy only if higher
		if(veterancy > pFoot->Veterancy.Veterancy) {
			pFoot->Veterancy.Veterancy = static_cast<float>(veterancy);
		}

		// select a free cell the unit can enter
		CellStruct tmpCell = MapClass::Instance->Pathfinding_Find(cell, SpeedType::Wheel, -1,
			MovementZone::Normal, false, 1, 1, false, false, false, false, CellStruct::Empty, false, false);

		CoordStruct crd = CellClass::Cell2Coord(tmpCell);

		// let the locomotor take care of the rest
		if(TechnoExt::CreateWithDroppod(pFoot, crd)) {
			if(!--count) {
				break;
			}
		}

		// randomize the target coodinates
		CellClass* pCell = MapClass::Instance->GetCellAt(tmpCell);
		int rnd = ScenarioClass::Instance->Random.RandomRanged(0, 7);
		for(int j = 0; j < 8; ++j) {
			// get the direction in an overly verbose way
			int dir = ((j + rnd) % 8) & 7;

			CellClass* pNeighbour = pCell->GetNeighbourCell(dir);
			if(pFoot->IsCellOccupied(pNeighbour, -1, -1, nullptr, true) == Move::OK) {
				cell = pNeighbour->MapCoords;
				break;
			}
		}

		// failed to place
		if(pFoot->InLimbo) {
			pFoot->UnInit();
		}
	}

	return true;
}
