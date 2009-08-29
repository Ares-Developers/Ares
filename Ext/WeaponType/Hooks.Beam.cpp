#include "Body.h"
#include "..\Techno\Body.h"

DEFINE_HOOK(6FD64A, TechnoClass_FireRadBeam1, 6)
{
	byte idxWeapon = *(byte *)(R->get_StackVar32(0x18) + 0xC);

	TechnoClass *Techno = (TechnoClass *)R->get_StackVar32(0x14);
	TechnoExt::ExtMap.Find(Techno)->idxSlot_Beam = idxWeapon;

	R->set_StackVar32(0x0, idxWeapon);
	return 0;
}

// 6FD79C, 6
// custom RadBeam colors
DEFINE_HOOK(6FD79C, TechnoClass_FireRadBeam2, 6)
{
	GET(RadBeam *, Rad, ESI);
	WeaponTypeClass* pSource = (WeaponTypeClass *)R->get_StackVar32(0xC);

	WeaponTypeExt::ExtData *pData = WeaponTypeExt::ExtMap.Find(pSource);

	if(pData->Beam_IsHouseColor) {
		GET(TechnoClass *, SourceUnit, EDI);
		Rad->set_Color(SourceUnit->Owner->get_Color());
	} else {
		Rad->set_Color(pData->Beam_Color.GetEx());
	}
	Rad->set_Period(pData->Beam_Duration);
	Rad->set_Amplitude(pData->Beam_Amplitude);
	return 0x6FD7A8;
}
