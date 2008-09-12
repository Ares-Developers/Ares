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
	TechnoTypeClass *Type = (TechnoTypeClass *)pThis->GetType();
	RETZ_UNLESS(Type->get_Crewed() || pThis->get_Passengers());
	TechnoTypeClassExt::TechnoTypeClassData *pData = TechnoTypeClassExt::Ext_p[Type];
	int chance = pData->Survivors_PilotChance;

	CoordStruct loc = *pThis->get_Location();
	if(Type->get_Crewed() && Randomizer::Global()->RandomRanged(1, 100) <= chance)
	{
		InfantryTypeClass *PilotType = pData->Survivors_Pilots[pThis->get_Owner()->get_SideIndex()];
		RETZ_UNLESS(PilotType);

		InfantryClass *Pilot = new InfantryClass(PilotType, pThis->get_Owner());

		Pilot->set_Health(PilotType->get_Strength() >> 1);
		Pilot->get_Veterancy()->Veterancy = pThis->get_Veterancy()->Veterancy;
		if(!TechnoClassExt::ParadropSurvivor(Pilot, &loc, Select))
		{
			delete Pilot;
		}
	}

	if(pThis->get_Passengers() > 0)
	{
		int idx = 0;
		FootClass *passenger;
		while(pThis->get_Passengers()->GetFirstPassenger())
		{
			++idx; // start passengers on cell 1, cell 0 is for pilot
			passenger = pThis->get_Passengers()->RemoveFirstPassenger();
			bool toDelete = 1;
			if(Randomizer::Global()->RandomRanged(1, 100) <= chance)
			{
				CoordStruct tmpLoc = loc;
				CellStruct tmpCoords = CellSpread::GetCell(idx);
				tmpLoc.X += tmpCoords.X * 128;
				tmpLoc.Y += tmpCoords.Y * 128;
				toDelete = !TechnoClassExt::ParadropSurvivor(passenger, &tmpLoc, Select);
			}
			if(toDelete)
			{
				passenger->RegisterDestruction(pKiller); //(TechnoClass *)R->get_StackVar32(0x54));
				delete passenger;
			}
		}
	}
}

bool TechnoClassExt::ParadropSurvivor(FootClass *Survivor, CoordStruct *loc, bool Select)
{
	bool success;
	if(loc->Z > 64)
	{
		success = Survivor->SpawnParachuted(loc);
	}
	else
	{
		success = Survivor->Put(loc, Randomizer::Global()->RandomRanged(0, 7));
	}
	RET_UNLESS(success);
	Survivor->Scatter(0xB1CFE8, 1, 0);
	Survivor->QueueMission(Survivor->get_Owner()->ControlledByHuman() ? mission_Guard : mission_Hunt, 0);
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
