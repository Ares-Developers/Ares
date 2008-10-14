#ifndef TECHNO_EXT_H
#define TECHNO_EXT_H

#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include <hash_map>

#include <AircraftClass.h>
#include <CellSpread.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <MapClass.h>
#include <Randomizer.h>

class TechnoClassExt
{
	public:
	struct TechnoClassData
	{
		// weapon slots fsblargh
		BYTE idxSlot_Wave;
		BYTE idxSlot_Beam;
		
		TimerStruct CloakSkipTimer;
	};

	EXT_P_DECLARE(TechnoClass);
	EXT_FUNCS(TechnoClass);
//	EXT_INI_FUNCS(TechnoClass);
	static void SpawnSurvivors(TechnoClass *pThis, TechnoClass *pKiller, bool Select);
	static bool ParadropSurvivor(FootClass *Survivor, CoordStruct *loc, bool Select);
};

#endif
