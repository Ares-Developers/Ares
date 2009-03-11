#include "Body.h"
#include "..\TechnoType\Body.h"

DEFINE_HOOK(70A990, TechnoClass_DrawVeterancy, 5)
{
	GET(TechnoClass *, T, ECX);
	GET_STACK(Point2D *, XY, 0x4);
	GET_STACK(RectangleStruct *, pRect, 0x8);

	Point2D offset = *XY;

	SHPStruct *iFile = FileSystem::PIPS_SHP;
	int iFrame = -1;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());

	SHPStruct *fCustom = NULL;

	switch(T->get_Veterancy()->GetRemainingLevel()) {
		case rank_Rookie:
			fCustom = pTypeData->Insignia_R;
			if(fCustom) {
				iFile = fCustom;
				iFrame = 0;
			} else {
				iFrame = -1;
			}
			break;
		case rank_Veteran:
			fCustom = pTypeData->Insignia_V;
			if(fCustom) {
				iFile = fCustom;
				iFrame = 0;
			} else {
				iFrame = 14;
			}
			break;
		case rank_Elite:
			fCustom = pTypeData->Insignia_E;
			if(fCustom) {
				iFile = fCustom;
				iFrame = 0;
			} else {
				iFrame = 15;
			}
			break;
	}

	offset.X += 5;
	offset.Y += 2;
	if(T->WhatAmI() != abs_Infantry) {
		offset.X += 5;
		offset.Y += 4;
	}

	if(iFrame != -1 && iFile) {
		Drawing::DSurface_Hidden_2->DrawSHP(
			FileSystem::THEATER_PAL, iFile, iFrame, &offset, pRect, bf_Alpha | bf_400 | bf_200, 0, -2, 0, 1000, 0, 0, 0, 0, 0);
	}

	return 0x70AA5B;
}
