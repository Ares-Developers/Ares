#include "Body.h"
#include "../TechnoType/Body.h"

#include <Helpers/Template.h>

const DWORD Extension<TechnoClass>::Canary = 0x55555555;
Container<TechnoExt> TechnoExt::ExtMap;

TechnoExt::TT *Container<TechnoExt>::SavingObject = NULL;
IStream *Container<TechnoExt>::SavingStream = NULL;

eFiringState TechnoExt::FiringStateCache = -1;

bool TechnoExt::NeedsRegap = false;

void TechnoExt::SpawnSurvivors(TechnoClass *pThis, TechnoClass *pKiller, bool Select)
{
	TechnoTypeClass *Type = pThis->GetTechnoType();

	HouseClass *pOwner = pThis->get_Owner();
	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(Type);
	TechnoExt::ExtData *pSelfData = TechnoExt::ExtMap.Find(pThis);
//	RETZ_UNLESS(pData->Survivors_PilotChance || pData->Survivors_PassengerChance);
	RETZ_UNLESS(!pSelfData->Survivors_Done);

	CoordStruct loc = *pThis->get_Location();

	int chance = pData->Survivors_PilotChance.BindTo(pThis)->Get();
	// remove check for Crewed if it is accounted for outside of this function
	// checks if Crewed=yes is set and there is a chance pilots survive, and, if yes...
	// ...attempts to spawn one Survivors_PilotCount times
	if(Type->Crewed && chance) {
		for(int i = 0; i < pData->Survivors_PilotCount; ++i) {
			if(Randomizer::Global()->RandomRanged(1, 100) <= chance) {
				InfantryTypeClass *PilotType = pData->Survivors_Pilots[pOwner->SideIndex];
				if(PilotType) {
					InfantryClass *Pilot = reinterpret_cast<InfantryClass *>(PilotType->CreateObject(pOwner));

					Pilot->set_Health(PilotType->Strength / 2);
					Pilot->get_Veterancy()->Veterancy = pThis->get_Veterancy()->Veterancy;
					CoordStruct destLoc, tmpLoc = loc;
					CellStruct tmpCoords = CellSpread::GetCell(Randomizer::Global()->RandomRanged(0, 7));

					tmpLoc.X += tmpCoords.X * 144;
					tmpLoc.Y += tmpCoords.Y * 144;

					CellClass * tmpCell = MapClass::Global()->GetCellAt(&tmpLoc);

					tmpCell->FindInfantrySubposition(&destLoc, &tmpLoc, 0, 0, 0);

					destLoc.Z = loc.Z;

					if(!TechnoExt::ParadropSurvivor(Pilot, &destLoc, Select)) {
						Pilot->RegisterDestruction(pKiller); //(TechnoClass *)R->get_StackVar32(0x54));
						GAME_DEALLOC(Pilot);
					}
				}
			}
		}
	}

	chance = pData->Survivors_PassengerChance.BindTo(pThis)->Get();
	while(pThis->get_Passengers()->FirstPassenger) {
		FootClass *passenger;
		bool toDelete = 1;
		passenger = pThis->get_Passengers()->RemoveFirstPassenger();
		if(chance) {
			if(Randomizer::Global()->RandomRanged(1, 100) <= chance) {
				CoordStruct destLoc, tmpLoc = loc;
				CellStruct tmpCoords = CellSpread::GetCell(Randomizer::Global()->RandomRanged(0, 7));

				tmpLoc.X += tmpCoords.X * 128;
				tmpLoc.Y += tmpCoords.Y * 128;

				CellClass * tmpCell = MapClass::Global()->GetCellAt(&tmpLoc);

				tmpCell->FindInfantrySubposition(&destLoc, &tmpLoc, 0, 0, 0);

				destLoc.Z = loc.Z;

				toDelete = !TechnoExt::ParadropSurvivor(passenger, &destLoc, Select);
			}
		}
		if(toDelete) {
			passenger->RegisterDestruction(pKiller); //(TechnoClass *)R->get_StackVar32(0x54));
			passenger->UnInit();
		}
	}

	pSelfData->Survivors_Done = 1;
}

bool TechnoExt::ParadropSurvivor(FootClass *Survivor, CoordStruct *loc, bool Select)
{
	bool success;
	int floorZ = MapClass::Global()->GetCellFloorHeight(loc);
	Debug::Log("Spawning survivor: loc->Z = %X, floorZ = %X\n", loc->Z, floorZ);
	++Unsorted::SomeMutex;
	if(loc->Z - floorZ > 100) {
		success = Survivor->SpawnParachuted(loc);
	} else {
		success = Survivor->Put(loc, Randomizer::Global()->RandomRanged(0, 7));
	}
	--Unsorted::SomeMutex;
	RET_UNLESS(success);
	Survivor->Scatter(0xB1CFE8, 1, 0);
	Survivor->QueueMission(Survivor->Owner->ControlledByHuman() ? mission_Guard : mission_Hunt, 0);
	if(Select) {
		Survivor->Select();
	}
	return 1;
	// TODO: Tag
}

void TechnoExt::PointerGotInvalid(void *ptr) {
	AnnounceInvalidPointerMap(AlphaExt, ptr);
	AnnounceInvalidPointerMap(SpotlightExt, ptr);
	AnnounceInvalidPointer(ActiveBuildingLight, ptr);
}

// =============================
// load/save

void Container<TechnoExt>::Load(TechnoClass *pThis, IStream *pStm) {
	TechnoExt::ExtData* pData = this->LoadKey(pThis, pStm);

	SWIZZLE(pData->Insignia_Image);
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

DEFINE_HOOK(70BF50, TechnoClass_SaveLoad_Prefix, 5)
DEFINE_HOOK_AGAIN(70C250, TechnoClass_SaveLoad_Prefix, 8)
{
	GET_STACK(TechnoExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<TechnoExt>::SavingObject = pItem;
	Container<TechnoExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(70C249, TechnoClass_Load_Suffix, 5)
{
	TechnoExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(70C264, TechnoClass_Save_Suffix, 5)
{
	TechnoExt::ExtMap.SaveStatic();
	return 0;
}
