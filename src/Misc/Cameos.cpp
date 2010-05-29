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

// a global var ewww
ConvertClass * CurrentDrawnConvert = NULL;

DEFINE_HOOK(6A9948, TabCameoListClass_Draw_SW, 6)
{
	GET(SuperWeaponTypeClass *, pSW, EAX);
	if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW)) {
		CurrentDrawnConvert = pData->CameoPal.Convert;
	}
	return 0;
}

DEFINE_HOOK(6A9A2A, TabCameoListClass_Draw_Main, 6)
{
	GET_STACK(TechnoTypeClass *, pTech, STACK_OFFS(0x4C4, 0x458));

	ConvertClass *pPalette = NULL;
	if(pTech) {
		if(TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(pTech)) {
			pPalette = pData->CameoPal.Convert;
		}
	} else if(CurrentDrawnConvert) {
		pPalette = CurrentDrawnConvert;
		CurrentDrawnConvert = NULL;
	}

	if(!pPalette) {
		pPalette = FileSystem::CAMEO_PAL;
	}
	R->EDX<ConvertClass *>(pPalette);
	return 0x6A9A30;
}
