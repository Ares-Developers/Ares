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
		SuperWeaponTypeClass *pSW, CCINIClass *pINI)
	{
		const char * section = pSW->get_ID();
		if(!CONTAINS(SuperWeaponTypeClassExt::Ext_p, pSW) || !pINI->GetSection(section))
		{
			return;
		}

		if(!pData->SW_Initialized)
		{
			pData->Initialize();
		}

		char buffer[256];
		PARSE_ANIM("Animation.Type", pData->Anim_Type);
		pData->Anim_ExtraZ = pINI->ReadInteger(section, "Animation.Height", pData->Anim_ExtraZ);
	}

	virtual bool SW_Animation::CanFireAt(CellStruct *pCoords)
	{
		return 1;
	}

	virtual bool SW_Animation::Launch(SuperClass* pThis, CellStruct* pCoords)
	{
		SuperWeaponTypeClass *pType = pThis->get_Type();
		RET_UNLESS(CONTAINS(SuperWeaponTypeClassExt::Ext_p, pType));
		SuperWeaponTypeClassExt::SuperWeaponTypeClassData *pData = SuperWeaponTypeClassExt::Ext_p[pType];

		CoordStruct coords;
		MapClass::Global()->GetCellAt(pCoords)->GetCoords(&coords);
		coords.Z += pData->Anim_ExtraZ;
		new AnimClass(pData->Anim_Type, coords);

		Unsorted::CurrentSWType = -1;
		return 1;
	}

};

#endif
