#include "Body.h"
#include "../BulletType/Body.h"

#include <AnimClass.h>
#include <BulletClass.h>

DEFINE_HOOK(4232CE, AnimClass_Draw_SetPalette, 6)
{
	GET(AnimTypeClass *, AnimType, EAX);

	auto pData = AnimTypeExt::ExtMap.Find(AnimType);

	if(pData->Palette.Convert) {
		R->ECX<ConvertClass *>(pData->Palette.Convert);
		return 0x4232D4;
	}

	return 0;
}

DEFINE_HOOK(468379, BulletClass_Draw_SetAnimPalette, 6)
{
	GET(BulletClass *, Bullet, ESI);
	auto pExt = BulletTypeExt::ExtMap.Find(Bullet->Type);

	if(ConvertClass* Convert = pExt->GetConvert()) {
		R->EBX<ConvertClass *>(Convert);
		return 0x4683D7;
	}

	return 0;
}
