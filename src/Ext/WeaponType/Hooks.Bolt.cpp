#include "Body.h"
#include "../Techno/Body.h"

void Saturate(BYTE &val, int delta) {
	int res = val + delta;
	if(res < 0) {
		res = 0;
	} else if(res > 255) {
		res = 255;
	}
	const BYTE result = res;
	val = result;
	//return result;
};

DEFINE_HOOK(6FD480, TechnoClass_FireEBolt, 6)
{
	GET(TechnoClass *, OwnerUnit, EDI);
	GET_STACK(WeaponTypeClass *, Weapon, 0x38);

	GET(EBolt *, Bolt, EAX);

	if(Weapon && Bolt) {
		WeaponTypeExt::BoltExt[Bolt] = WeaponTypeExt::ExtMap.Find(Weapon);
		WeaponTypeExt::ExtData *BoltAres = WeaponTypeExt::ExtMap.Find(Weapon);
		if (OwnerUnit) {
			if(!!BoltAres->Bolt_IsHouseColor) {
				BoltAres->Bolt_HouseColorBase = OwnerUnit->Owner->Color;
			}
		} else {
			BoltAres->Bolt_IsHouseColor = false;
		} //If HouseColor can't be obtained, remove it from the weapon
	}
	return 0;
}


DEFINE_HOOK(4C2951, EBolt_DTOR, 5)
{
	GET(EBolt *, Bolt, ECX);
	hash_boltExt::iterator i = WeaponTypeExt::BoltExt.find(Bolt);
	if(i != WeaponTypeExt::BoltExt.end()) {
		WeaponTypeExt::BoltExt.erase(i);
	}

	return 0;
}

DEFINE_HOOK(4C24BE, EBolt_Draw_Color1, 5)
{
	GET_STACK(EBolt *, Bolt, 0x40);
	WeaponTypeExt::ExtData *pData = WeaponTypeExt::BoltExt[Bolt];

	if(pData) {
		WORD Packed = 0;
		if(!!pData->Bolt_IsHouseColor) {
			ColorStruct tmp(pData->Bolt_HouseColorBase);
			Packed = Drawing::Color16bit(&tmp);
		} else if(ColorStruct *clr = pData->Bolt_Color1) {
			Packed = Drawing::Color16bit(clr);
		}
		if(Packed) {
			R->EAX(Packed);
			return 0x4C24E4;
		}
	}

	return 0;
}

DEFINE_HOOK(4C25CB, EBolt_Draw_Color2, 5)
{
	GET_STACK(EBolt *, Bolt, 0x40);
	WeaponTypeExt::ExtData *pData = WeaponTypeExt::BoltExt[Bolt];

	if(pData) {
		WORD Packed = 0;
		if(!!pData->Bolt_IsHouseColor) {
			ColorStruct tmp;
			if (pData->Bolt_ColorSpread==-1) {
				tmp.R=248;
				tmp.G=252;
				tmp.B=248;
			} else {
				tmp=pData->Bolt_HouseColorBase;
				int delta = pData->Bolt_ColorSpread;
				Saturate(tmp.R, delta);
				Saturate(tmp.G, delta);
				Saturate(tmp.B, delta);
			}
			Packed = Drawing::Color16bit(&tmp);
		} else if(ColorStruct *clr = pData->Bolt_Color2) {
			Packed = Drawing::Color16bit(clr);
		}
		if(Packed) {
			R->Stack<int>(0x18, Packed);
			return 0x4C25FD;
		}
	}

	return 0;
}


DEFINE_HOOK(4C26C7, EBolt_Draw_Color3, 5)
{
	GET_STACK(EBolt *, Bolt, 0x40);
	WeaponTypeExt::ExtData *pData = WeaponTypeExt::BoltExt[Bolt];

	if(pData) {
		WORD Packed = 0;
		if(!!pData->Bolt_IsHouseColor) {
			ColorStruct tmp(pData->Bolt_HouseColorBase);
			if (pData->Bolt_ColorSpread!=-1) {
				int delta = -pData->Bolt_ColorSpread;
				Saturate(tmp.R, delta);
				Saturate(tmp.G, delta);
				Saturate(tmp.B, delta);
			}
			Packed = Drawing::Color16bit(&tmp);
		} else if(ColorStruct *clr = pData->Bolt_Color3) {
			Packed = Drawing::Color16bit(clr);
		}
		if(Packed) {
			R->EBX(R->EBX() - 2);
			R->EAX(Packed);
			return 0x4C26EE;
		}
	}

	return 0;
}
