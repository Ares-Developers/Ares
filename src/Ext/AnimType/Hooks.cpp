#include "Body.h"
#include <AnimClass.h>
#include <BulletClass.h>

DEFINE_HOOK(423122, AnimClass_Draw_SetPalette, 6)
{
	GET(AnimTypeClass *, AnimType, EAX);
	LEA_STACK(ConvertClass **, Palette, 0x30);

	auto pData = AnimTypeExt::ExtMap.Find(AnimType);

	if(pData->Palette.Convert) {
		*Palette = pData->Palette.Convert;
		return 0x423358;
	}

	return 0;
}


DEFINE_HOOK(468379, BulletClass_Draw_SetAnimPalette, 6)
{
	GET(BulletClass *, Bullet, ESI);
	if(AnimTypeClass * AnimType = AnimTypeClass::Find(Bullet->Type->ImageFile)) {
		auto pData = AnimTypeExt::ExtMap.Find(AnimType);

		if(pData->Palette.Convert) {
			R->EBX<ConvertClass *>(pData->Palette.Convert);
			return 0x4683D7;
		}
	}
	return 0;
}
