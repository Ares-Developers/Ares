#include "Animation.h"

void SW_Animation::LoadFromINI(
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

bool SW_Animation::CanFireAt(CellStruct *pCoords)
{
	return 1;
}

bool SW_Animation::Launch(SuperClass* pThis, CellStruct* pCoords)
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
