#ifndef TECHNOTYPE_EXT_H
#define TECHNOTYPE_EXT_H

#include <YRPP.h>
#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include "Ares.h"

#include <hash_map>

class TechnoTypeClassExt
{
	public:
	struct TechnoTypeClassData
	{
		DynamicVectorClass<InfantryTypeClass *> Survivors_Pilots;
		int Survivors_PilotChance;
		int Survivors_PassengerChance;

		// animated cameos
//		int Cameo_Interval;
//		int Cameo_CurrentFrame;
//		TimerStruct Cameo_Timer;

		bool Data_Initialized;
		void Initialize(TechnoTypeClass *pThis);
	};

	EXT_P_DECLARE(TechnoTypeClass);
	EXT_FUNCS(TechnoTypeClass);
	EXT_INI_FUNCS(TechnoTypeClass);
};

#endif
