#include "Body.h"
#include "../TechnoType/Body.h"

DEFINE_HOOK(70A990, TechnoClass_DrawVeterancy, 5)
{
	GET(TechnoClass *, T, ECX);
	GET_STACK(Point2D *, XY, 0x4);
	GET_STACK(RectangleStruct *, pRect, 0x8);

	Point2D offset = *XY;

	SHPStruct *iFile = FileSystem::PIPS_SHP;
	int iFrame = -1;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());

	pTypeData->Insignia.BindTo(T);
	SHPStruct *fCustom = pTypeData->Insignia.Get();

	if(fCustom) {
		iFile = fCustom;
		iFrame = 0;
	} else {
		VeterancyStruct *XP = &T->Veterancy;
		if(XP->IsElite()) {
			iFrame = 15;
		} else if(XP->IsVeteran()) {
			iFrame = 14;
		}
	}

	if(iFrame != -1 && iFile) {
		offset.X += 5;
		offset.Y += 2;
		if(T->WhatAmI() != abs_Infantry) {
			offset.X += 5;
			offset.Y += 4;
		}

		DSurface::Hidden_2->DrawSHP(
			FileSystem::THEATER_PAL, iFile, iFrame, &offset, pRect, bf_Alpha | bf_400 | bf_200, 0, -2, 0, 1000, 0, 0, 0, 0, 0);
	}

	return 0x70AA5B;
}
