#ifndef TECHNO_EXT_H
#define TECHNO_EXT_H

#include <YRPP.h>
#include "Ares.h"

#include HASHMAP

class TechnoClassExt
{
	public:
	struct TechnoClassData
	{
	};

	EXT_P_DEFINE(TechnoClass);
	EXT_FUNCS(TechnoClass);
//	EXT_INI_FUNCS(TechnoClass);
	static void SpawnSurvivors(TechnoClass *pThis, TechnoClass *pKiller);
	static bool ParadropSurvivor(FootClass *Survivor, CoordStruct *loc, bool Select);
};

#endif
