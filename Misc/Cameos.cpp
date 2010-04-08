#include "../Ext/TechnoType/Body.h"
#include "../Ext/SWType/Body.h"

// bugfix #277 revisited: VeteranInfantry and friends don't show promoted cameos
DEFINE_HOOK(712045, TechnoTypeClass_GetCameo, 5)
{
	// egads and gadzooks
	retfunc<SHPStruct *> ret(R, 0x7120C6);

	GET(TechnoTypeClass *, T, ECX);
	GET(HouseClass *, House, EAX);
	HouseTypeClass *Country = House->Type;

	SHPStruct *Cameo = T->Cameo;
	SHPStruct *Alt = T->AltCameo;

	if(!Alt) {
		return ret(Cameo);
	}

	switch(T->WhatAmI()) {
		case abs_InfantryType:
			if(House->BarracksInfiltrated && !T->Naval && T->Trainable) {
				return ret(Alt);
			} else {
				return ret(Country->VeteranInfantry.FindItemIndex((InfantryTypeClass **)&T) == -1 ? Cameo : Alt);
			}
		case abs_UnitType:
			if(House->WarFactoryInfiltrated && !T->Naval && T->Trainable) {
				return ret(Alt);
			} else {
				return ret(Country->VeteranUnits.FindItemIndex((UnitTypeClass **)&T) == -1 ? Cameo : Alt);
			}
		case abs_AircraftType:
			return ret(Country->VeteranAircraft.FindItemIndex((AircraftTypeClass **)&T) == -1 ? Cameo : Alt);
		case abs_BuildingType:
			if(TechnoTypeClass *Item = T->UndeploysInto) {
				return ret(Country->VeteranUnits.FindItemIndex((UnitTypeClass **)&Item) == -1 ? Cameo : Alt);
			}
	}

	return ret(Cameo);
}

DEFINE_HOOK(6A9A2A, TabCameoListClass_Draw, 6)
{
	GET_STACK(ObjectTypeClass *, pType, STACK_OFFS(0x4C4, 0x458));

	if(pType) {
		ConvertClass *pPalette = NULL;
		eAbstractType absId = pType->WhatAmI();
		TechnoTypeClass *pTech = NULL;
		SuperWeaponTypeClass *pSW = NULL;
		switch(absId) {
			case SuperWeaponTypeClass::AbsID:
				pSW = reinterpret_cast<SuperWeaponTypeClass *>(pType);
				if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW)) {
					pPalette = pData->CameoPal.Convert;
				}
				break;
			case UnitTypeClass::AbsID:
			case AircraftTypeClass::AbsID:
			case BuildingTypeClass::AbsID:
			case InfantryTypeClass::AbsID:
				pTech = reinterpret_cast<TechnoTypeClass *>(pType);
				if(TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(pTech)) {
					pPalette = pData->CameoPal.Convert;
				}
				break;
		}
		if(!pPalette) {
			pPalette = FileSystem::CAMEO_PAL;
		}

		R->EDX<ConvertClass *>(pPalette);
		return 0x6A9A30;
	}
	return 0;
}
