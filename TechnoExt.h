#ifndef TECHNO_EXT_H
#define TECHNO_EXT_H

#include <YRPP.h>
#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include "Ares.h"

#include <hash_map>

class TechnoClassExt
{
	public:
	struct TechnoClassData
	{
		// weapon slots fsblargh
		byte idxSlot_Wave;
		byte idxSlot_Beam;
		
		TimerStruct CloakSkipTimer;
	};

	EXT_P_DECLARE(TechnoClass);
	EXT_FUNCS(TechnoClass);
//	EXT_INI_FUNCS(TechnoClass);
	static void SpawnSurvivors(TechnoClass *pThis, TechnoClass *pKiller, bool Select);
	static bool ParadropSurvivor(FootClass *Survivor, CoordStruct *loc, bool Select);
};

#endif
