#include "Body.h"
#include "../Techno/Body.h"

DEFINE_HOOK(0x6FD64A, TechnoClass_FireRadBeam1, 0x6)
{
	byte idxWeapon = *(byte *)(R->Stack32(0x18) + 0xC); // hack! 0x18 fetches the caller's EBP, which gives us access to its locals, including idxWeapon
	GET_STACK(TechnoClass *, Techno, 0x14);

	TechnoExt::ExtMap.Find(Techno)->idxSlot_Beam = idxWeapon;

	R->Stack<int>(0x0, idxWeapon);
	return 0;
}

// 6FD79C, 6
// custom RadBeam colors
DEFINE_HOOK(0x6FD79C, TechnoClass_FireRadBeam2, 0x6)
{
	GET(RadBeam *, Rad, ESI);
	GET_STACK(WeaponTypeClass *, pSource, 0xC);

	WeaponTypeExt::ExtData *pData = WeaponTypeExt::ExtMap.Find(pSource);

	if(pData->Beam_IsHouseColor) {
		GET(TechnoClass *, SourceUnit, EDI);
		Rad->Color = SourceUnit->Owner->Color;
	} else {
		Rad->Color = pData->Beam_Color;
	}
	Rad->Period = pData->Beam_Duration;
	Rad->Amplitude = pData->Beam_Amplitude;
	return 0x6FD7A8;
}
