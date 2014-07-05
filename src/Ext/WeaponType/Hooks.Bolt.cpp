#include "Body.h"
#include "../Techno/Body.h"

DEFINE_HOOK(6FD469, TechnoClass_FireEBolt, 9)
{
	//GET(TechnoClass*, pThis, EDI);
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFS(0x30, -0x8));

	R->EAX(WeaponTypeExt::CreateBolt(pWeapon));
	R->ESI(0);

	return 0x6FD480;
}

DEFINE_HOOK(4C2951, EBolt_DTOR, 5)
{
	GET(EBolt *, Bolt, ECX);
	WeaponTypeExt::BoltExt.erase(Bolt);
	return 0;
}

DEFINE_HOOK(4C24BE, EBolt_Draw_Color1, 5)
{
	GET_STACK(EBolt *, Bolt, 0x40);

	if(auto pData = WeaponTypeExt::BoltExt.get_or_default(Bolt)) {
		auto &clr = pData->Bolt_Color1;
		if(clr.isset()) {
			R->EAX(Drawing::Color16bit(clr));
			return 0x4C24E4;
		}
	}

	return 0;
}

DEFINE_HOOK(4C25CB, EBolt_Draw_Color2, 5)
{
	GET_STACK(EBolt *, Bolt, 0x40);

	if(auto pData = WeaponTypeExt::BoltExt.get_or_default(Bolt)) {
		auto &clr = pData->Bolt_Color2;
		if(clr.isset()) {
			R->Stack<int>(0x18, Drawing::Color16bit(clr));
			return 0x4C25FD;
		}
	}

	return 0;
}

DEFINE_HOOK(4C26C7, EBolt_Draw_Color3, 5)
{
	GET_STACK(EBolt *, Bolt, 0x40);
	
	if(auto pData = WeaponTypeExt::BoltExt.get_or_default(Bolt)) {
		auto &clr = pData->Bolt_Color3;
		if(clr.isset()) {
			R->EBX(R->EBX() - 2);
			R->EAX(Drawing::Color16bit(clr));
			return 0x4C26EE;
		}
	}

	return 0;
}
