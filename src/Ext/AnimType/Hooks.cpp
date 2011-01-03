#include "Body.h"
#include <AnimClass.h>

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
