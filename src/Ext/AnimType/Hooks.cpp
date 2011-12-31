#include "Body.h"
#include <AnimClass.h>
#include <BulletClass.h>


DEFINE_HOOK(0x4232CE, AnimClass_Draw_SetPalette, 0x6)
{
	GET(AnimTypeClass *, AnimType, EAX);

	auto pData = AnimTypeExt::ExtMap.Find(AnimType);

	if(pData->Palette.Convert) {
		R->ECX<ConvertClass *>(pData->Palette.Convert);
		return 0x4232D4;
	}

	return 0;
}


DEFINE_HOOK(0x468379, BulletClass_Draw_SetAnimPalette, 0x6)
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
