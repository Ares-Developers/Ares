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
		bool Survivors_PassengersEscape;
		int Survivors_PilotChance;
		int Survivors_PassengerChance;

		bool Data_Initialized;
		void Initialize(TechnoTypeClass *pThis);
	};

	EXT_P_DEFINE(TechnoTypeClass);
	EXT_FUNCS(TechnoTypeClass);
	EXT_INI_FUNCS(TechnoTypeClass);
};

#endif
