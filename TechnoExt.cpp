#include "TechnoExt.h"
#include "TechnoTypeExt.h"

EXT_P_DEFINE(TechnoClass);

EXT_CTOR(TechnoClass)
{
	if(!CONTAINS(Ext_p, pThis))
	{
		ALLOC(ExtData, pData);

		pData->idxSlot_Wave = 0;
		pData->idxSlot_Beam = 0;

		pData->CloakSkipTimer.Stop();

		Ext_p[pThis] = pData;
	}
}

EXT_DTOR(TechnoClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		DEALLOC(Ext_p, pThis);
	}
}

EXT_LOAD(TechnoClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		Create(pThis);

		ULONG out;
		pStm->Read(&Ext_p[pThis], sizeof(ExtData), &out);
	}
}

EXT_SAVE(TechnoClass)
{
	if(CONTAINS(Ext_p, pThis))
	{
		ULONG out;
		pStm->Write(&Ext_p[pThis], sizeof(ExtData), &out);
	}
}

void TechnoClassExt::SpawnSurvivors(TechnoClass *pThis, TechnoClass *pKiller, bool Select)
{
	TechnoTypeClass *Type = pThis->GetTechnoType();

	Ares::Log("survivors - start for [%s]\n", Type->get_ID());

	HouseClass *pOwner = pThis->get_Owner();
	TechnoTypeClassExt::TechnoTypeClassData *pData = TechnoTypeClassExt::Ext_p[Type];
	RETZ_UNLESS(pData->Survivors_PilotChance || pData->Survivors_PassengerChance);

	Ares::Log("survivors - chance\n");

	CoordStruct loc = *pThis->get_Location();

	int chance = pData->Survivors_PilotChance;
	if(chance && Randomizer::Global()->RandomRanged(1, 100) <= chance)
	{
		InfantryTypeClass *PilotType = pData->Survivors_Pilots[pOwner->get_SideIndex()];
		if(PilotType)
		{
			InfantryClass *Pilot = new InfantryClass(PilotType, pOwner);

			Pilot->set_Health(PilotType->get_Strength() >> 1);
			Pilot->get_Veterancy()->Veterancy = pThis->get_Veterancy()->Veterancy;
			CoordStruct tmpLoc = loc;
			CellStruct tmpCoords = CellSpread::GetCell(Randomizer::Global()->RandomRanged(0, 7));
			tmpLoc.X += tmpCoords.X * 144;
			tmpLoc.Y += tmpCoords.Y * 144;

			if(!TechnoClassExt::ParadropSurvivor(Pilot, &tmpLoc, Select))
			{
				Pilot->RegisterDestruction(pKiller); //(TechnoClass *)R->get_StackVar32(0x54));
				delete Pilot;
			}
		}
	}

	chance = pData->Survivors_PassengerChance;
	while(pThis->get_Passengers()->FirstPassenger)
	{
		Ares::Log("survivors - pasg loop\n");
		int idx = 0;
		FootClass *passenger;
		bool toDelete = 1;
		passenger = pThis->get_Passengers()->RemoveFirstPassenger();
		Ares::Log("survivors - got pasg %s\n", passenger->GetType()->get_ID());
		if(chance)
		{
			++idx; // start passengers on cell 2, cell 1 is for pilot
			Ares::Log("survivors - pasg good chance\n");
			if(Randomizer::Global()->RandomRanged(1, 100) <= chance)
			{
				CoordStruct tmpLoc = loc;
				CellStruct tmpCoords = CellSpread::GetCell(idx);
				tmpLoc.X += tmpCoords.X * 128;
				tmpLoc.Y += tmpCoords.Y * 128;
				Ares::Log("survivors - pasg spawn\n");
				toDelete = !TechnoClassExt::ParadropSurvivor(passenger, &tmpLoc, Select);
			}
		}
		if(toDelete)
		{
			Ares::Log("survivors - no dice, purge\n");
			passenger->RegisterDestruction(pKiller); //(TechnoClass *)R->get_StackVar32(0x54));
			passenger->UnInit();
		}
	}
}

bool TechnoClassExt::ParadropSurvivor(FootClass *Survivor, CoordStruct *loc, bool Select)
{
	bool success;
	int floorZ = MapClass::Global()->GetCellFloorHeight(loc);
	Ares::Log("locZ %d, floorz %d\n", loc->Z, floorZ);
	if(loc->Z > floorZ)
	{
		Ares::Log("survivors - deltaz , chute\n");
		success = Survivor->SpawnParachuted(loc);
	}
	else
	{
		Ares::Log("survivors - no deltaz, place\n");
		success = Survivor->Put(loc, Randomizer::Global()->RandomRanged(0, 7));
	}
	RET_UNLESS(success);
	Ares::Log("survivor - in place\n");
	Survivor->Scatter(0xB1CFE8, 1, 0);
	Survivor->QueueMission(Survivor->get_Owner()->ControlledByHuman() ? mission_Guard : mission_Hunt, 0);
	Ares::Log("survivors - queued up mission\n");
	if(Select)
	{
		Survivor->Select();
	}
	return 1;
	// TODO: Tag
}

// bugfix #297: Crewed=yes jumpjets spawn parachuted infantry on destruction, not idle
// 7381A9, 6
EXPORT_FUNC(UnitClass_ReceiveDamage)
{
	GET(TechnoClass *, t, ESI);

	TechnoClassExt::SpawnSurvivors(t, (TechnoClass *)R->get_StackVar32(0x54), !!R->get_StackVar8(0x13));

	return 0x73838A;
}

// bugfix #297: Crewed=yes AircraftTypes spawn parachuting infantry on death
// 41668B, 6
EXPORT_FUNC(AircraftClass_ReceiveDamage)
{
	GET(AircraftClass *, a, ESI);

	TechnoClass *Killer = (TechnoClass *)R->get_StackVar32(0x2C);
	
	bool select = a->get_IsSelected() && a->get_Owner()->ControlledByPlayer();

	TechnoClassExt::SpawnSurvivors(a, Killer, select);

	return 0;
}

// 6F9E50, 5
EXPORT_FUNC(TechnoClass_Update)
{
	GET(TechnoClass *, Source, ECX);

	RET_UNLESS(CONTAINS(TechnoClassExt::Ext_p, Source));
	TechnoClassExt::TechnoClassData *pData = TechnoClassExt::Ext_p[Source];

	if(pData->CloakSkipTimer.IsDone())
	{
		pData->CloakSkipTimer.Stop();
		Source->set_Cloakable(Source->GetTechnoType()->get_Cloakable());
	}
	else if(pData->CloakSkipTimer.GetTimeLeft() > 0)
	{
		Source->set_Cloakable(0);
	}
	return 0;
}
