#include "Body.h"
#include "..\TechnoType\Body.h"

const DWORD Extension<TechnoClass>::Canary = 0x55555555;
Container<TechnoExt> TechnoExt::ExtMap;

void TechnoExt::SpawnSurvivors(TechnoClass *pThis, TechnoClass *pKiller, bool Select)
{
	TechnoTypeClass *Type = pThis->GetTechnoType();

	HouseClass *pOwner = pThis->get_Owner();
	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(Type);
	RETZ_UNLESS(pData->Survivors_PilotChance || pData->Survivors_PassengerChance);

	CoordStruct loc = *pThis->get_Location();

	int chance = pData->Survivors_PilotChance;
	if(chance && Randomizer::Global()->RandomRanged(1, 100) <= chance) {
		InfantryTypeClass *PilotType = pData->Survivors_Pilots[pOwner->get_SideIndex()];
		if(PilotType) {
			InfantryClass *Pilot = new InfantryClass(PilotType, pOwner);

			Pilot->set_Health(PilotType->Strength / 2);
			Pilot->get_Veterancy()->Veterancy = pThis->get_Veterancy()->Veterancy;
			CoordStruct tmpLoc = loc;
			CellStruct tmpCoords = CellSpread::GetCell(Randomizer::Global()->RandomRanged(0, 7));
			tmpLoc.X += tmpCoords.X * 144;
			tmpLoc.Y += tmpCoords.Y * 144;

			if(!TechnoExt::ParadropSurvivor(Pilot, &tmpLoc, Select)) {
				Pilot->RegisterDestruction(pKiller); //(TechnoClass *)R->get_StackVar32(0x54));
				delete Pilot;
			}
		}
	}

	chance = pData->Survivors_PassengerChance;
	while(pThis->get_Passengers()->FirstPassenger) {
		FootClass *passenger;
		bool toDelete = 1;
		passenger = pThis->get_Passengers()->RemoveFirstPassenger();
		if(chance) {
			if(Randomizer::Global()->RandomRanged(1, 100) <= chance) {
				CoordStruct tmpLoc = loc;
				CellStruct tmpCoords = CellSpread::GetCell(Randomizer::Global()->RandomRanged(0, 7));
				tmpLoc.X += tmpCoords.X * 128;
				tmpLoc.Y += tmpCoords.Y * 128;
				toDelete = !TechnoExt::ParadropSurvivor(passenger, &tmpLoc, Select);
			}
		}
		if(toDelete) {
			passenger->RegisterDestruction(pKiller); //(TechnoClass *)R->get_StackVar32(0x54));
			passenger->UnInit();
		}
	}
}

bool TechnoExt::ParadropSurvivor(FootClass *Survivor, CoordStruct *loc, bool Select)
{
	bool success;
	int floorZ = MapClass::Global()->GetCellFloorHeight(loc);
	if(loc->Z > floorZ) {
		success = Survivor->SpawnParachuted(loc);
	} else {
		success = Survivor->Put(loc, Randomizer::Global()->RandomRanged(0, 7));
	}
	RET_UNLESS(success);
	Survivor->Scatter(0xB1CFE8, 1, 0);
	Survivor->QueueMission(Survivor->Owner->ControlledByHuman() ? mission_Guard : mission_Hunt, 0);
	if(Select) {
		Survivor->Select();
	}
	return 1;
	// TODO: Tag
}


// =============================
// container hooks

DEFINE_HOOK(6F3260, TechnoClass_CTOR, 5)
{
	GET(TechnoClass*, pItem, ESI);

	TechnoExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(6F4500, TechnoClass_DTOR, 5)
{
	GET(TechnoClass*, pItem, ECX);

	TechnoExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(70C249, TechnoClass_Load, 5)
{
	GET_STACK(TechnoClass*, pItem, 0xC);
	GET_STACK(IStream*, pStm, 0x10);

	TechnoExt::ExtMap.Load(pItem, pStm);
	return 0;
}

DEFINE_HOOK(70C264, TechnoClass_Save, 5)
{
	GET_STACK(TechnoClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TechnoExt::ExtMap.Save(pItem, pStm);
	return 0;
}
