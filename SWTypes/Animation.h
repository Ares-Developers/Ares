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

	virtual void SW_Animation::LoadFromINI(
		SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData, 
		SuperWeaponTypeClass *pSW, CCINIClass *pINI);

	virtual bool SW_Animation::CanFireAt(CellStruct *pCoords);

	virtual bool SW_Animation::Launch(SuperClass* pThis, CellStruct* pCoords);

};

#endif
