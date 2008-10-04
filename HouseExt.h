#ifndef HOUSE_EXT_H
#define HOUSE_EXT_H

#include <YRPP.h>
#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include "Ares.h"

#include <hash_map>

class HouseClassExt
{
	public:
	struct HouseClassData
	{
		// Generic
		bool Is_Initialized;

		void Initialize(HouseClass* pThis);
	};

	EXT_P_DECLARE(HouseClass);
	EXT_FUNCS(HouseClass);
};

#endif
