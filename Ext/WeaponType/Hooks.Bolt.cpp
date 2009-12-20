#include "Body.h"
#include "../Techno/Body.h"

DEFINE_HOOK(6FD480, TechnoClass_FireEBolt, 6)
{
//	GET(TechnoClass *, OwnerUnit, EDI);
	GET_STACK(WeaponTypeClass *, Weapon, 0x38);

	GET(EBolt *, Bolt, EAX);

	if(Weapon && Bolt) {
		WeaponTypeExt::BoltExt[Bolt] = WeaponTypeExt::ExtMap.Find(Weapon);
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
		ColorStruct * clr = pData->Bolt_Color1.GetEx();
		if(clr) {
			R->EAX(Drawing::Color16bit(clr));
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
		ColorStruct * clr = pData->Bolt_Color2.GetEx();
		if(clr) {
			R->Stack<int>(0x18, Drawing::Color16bit(clr));
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
		ColorStruct * clr = pData->Bolt_Color3.GetEx();
		if(clr) {
			R->EBX(R->EBX() - 2);
			R->EAX(Drawing::Color16bit(clr));
			return 0x4C26EE;
		}
	}

	return 0;
}
