#ifndef TECHNO_EXT_H
#define TECHNO_EXT_H

#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include <hash_map>
#include <vector>
#include <algorithm>

#include <AircraftClass.h>
#include <CellSpread.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <MapClass.h>
#include <Randomizer.h>
#include <TemporalClass.h>
#include <WeaponTypeClass.h>
#include <WarheadTypeClass.h>

#include "WarheadTypeExt.h"
#include "WeaponTypeExt.h"

class TechnoClassExt
{
	public:
	struct TechnoClassData
	{
		// weapon slots fsblargh
		BYTE idxSlot_Wave;
		BYTE idxSlot_Beam;
		BYTE idxSlot_Warp;

		TimerStruct CloakSkipTimer;
		SHPStruct * Insignia_Image;
	};

	EXT_P_DECLARE(TechnoClass);
	EXT_FUNCS(TechnoClass);
//	EXT_INI_FUNCS(TechnoClass);
	static void SpawnSurvivors(TechnoClass *pThis, TechnoClass *pKiller, bool Select);
	static bool ParadropSurvivor(FootClass *Survivor, CoordStruct *loc, bool Select);
	
	static int SelectWeaponAgainst(TechnoClass *pThis, TechnoClass *pTarget);
	static bool EvalWeaponAgainst(TechnoClass *pThis, TechnoClass *pTarget, WeaponTypeClass* W);
	static float EvalVersesAgainst(TechnoClass *pThis, TechnoClass *pTarget, WeaponTypeClass* W);
};

#endif
