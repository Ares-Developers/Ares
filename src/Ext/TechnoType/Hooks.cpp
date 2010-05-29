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

	if(pINI->ReadString(pType->ID, "WaterVoxel", "", Ares::readBuffer, Ares::readLength)) {
		pData->WaterAlt = 1;
//		_strncpy
	}

	return 0;
}
