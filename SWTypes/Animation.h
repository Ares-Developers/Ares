#ifndef SUPERTYPE_EXT_ANIM_H
#define SUPERTYPE_EXT_ANIM_H

#include <YRPP.h>
#include "..\Ares.h"
#include "..\SuperWeaponTypeExt.h"

#include <hash_map>

class SW_Animation : NewSWType
{
	public:
		SW_Animation() : NewSWType()
			{ };

		virtual ~SW_Animation()
			{ };

		virtual const char * GetTypeString()
			{ return "Animation"; }

	virtual void LoadFromINI(
		SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData, 
		SuperWeaponTypeClass *pSW, CCINIClass *pINI);

	virtual bool CanFireAt(CellStruct *pCoords);

	virtual bool Launch(SuperClass* pThis, CellStruct* pCoords);

};

#endif
