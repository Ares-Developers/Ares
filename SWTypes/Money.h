#ifndef SUPERTYPE_EXT_MONEY_H
#define SUPERTYPE_EXT_MONEY_H

#include <YRPP.h>
#include "..\Ares.h"
#include "..\SuperWeaponTypeExt.h"

#include <hash_map>

class SW_Money : NewSWType
{
	public:
		SW_Money() : NewSWType()
			{ };

		virtual ~SW_Money()
			{ };

		virtual const char * GetTypeString()
			{ return "Money"; }

	virtual void LoadFromINI(
		SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI);
	virtual bool Launch(SuperClass* pThis, CellStruct* pCoords);
};
#endif
