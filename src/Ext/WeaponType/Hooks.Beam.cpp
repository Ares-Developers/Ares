#include "Body.h"
#include "../Techno/Body.h"

#include <HouseClass.h>

DEFINE_HOOK(6FD64A, TechnoClass_FireRadBeam1, 6)
{
	byte idxWeapon = *reinterpret_cast<byte*>(R->Stack32(0x18) + 0xC); // hack! 0x18 fetches the caller's EBP, which gives us access to its locals, including idxWeapon
	GET_STACK(TechnoClass *, Techno, 0x14);

	// get the default color
	GET(TechnoClass *, SourceUnit, EDI);
	GET(RadBeam *, Rad, ESI);
	if(SourceUnit && SourceUnit->Owner) {
		Rad->Color = SourceUnit->Owner->Color;
	} else {
		Rad->Color = RulesClass::Instance->RadColor;
	}

	TechnoExt::ExtMap.Find(Techno)->idxSlot_Beam = idxWeapon;

	R->Stack<int>(0x0, idxWeapon);
	return 0;
}

// 6FD79C, 6
// custom RadBeam colors
DEFINE_HOOK(6FD79C, TechnoClass_FireRadBeam2, 6)
{
	GET(RadBeam *, Rad, ESI);
	GET_STACK(WeaponTypeClass *, pSource, 0xC);

	WeaponTypeExt::ExtData *pData = WeaponTypeExt::ExtMap.Find(pSource);

	if(!pData->Beam_IsHouseColor) {
		Rad->Color = pData->GetBeamColor();
	}
	Rad->Period = pData->Beam_Duration;
	Rad->Amplitude = pData->Beam_Amplitude;
	return 0x6FD7A8;
}
