#include "Body.h"
//include "Side.h"
#include "../../Enum/Prerequisites.h"
#include "../../Misc/Debug.h"

// =============================
// other hooks


DEFINE_HOOK(732D10, TacticalClass_CollectSelectedIDs, 5)
{
	return 0;
}

DEFINE_HOOK(5F8480, ObjectTypeClass_Load3DArt, 6)
{
	GET(ObjectTypeClass *, O, ESI);
	if(O->WhatAmI() == abs_UnitType) {
		TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(reinterpret_cast<TechnoTypeClass*>(O));
		if(pData->WaterAlt) {
			return 0x5F848C;
		}
	}
	return 0;
}


DEFINE_HOOK(715320, TechnoTypeClass_LoadFromINI_EarlyReader, 6)
{
	GET(CCINIClass *, pINI, EDI);
	GET(TechnoTypeClass *, pType, EBP);

	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(pType);

	if(pINI->ReadString(pType->get_ID(), "WaterVoxel", "", Ares::readBuffer, Ares::readLength)) {
		pData->WaterAlt = 1;
//		_strncpy
	}

	return 0;
}

DEFINE_HOOK(6A9A2A, TabCameoListClass_Draw, 6)
{
	GET_STACK(TechnoTypeClass *, pType, STACK_OFFS(0x4C4, 0x458));

	if(pType) {
		if(TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(pType)) {
			ConvertClass *pPalette = pData->CameoConvert ? pData->CameoConvert : FileSystem::CAMEO_PAL;

			R->EDX<ConvertClass *>(pPalette);
			return 0x6A9A30;
		}
	}
	return 0;
}
