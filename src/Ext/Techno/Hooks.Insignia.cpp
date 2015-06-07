#include "Body.h"
#include "../Rules/Body.h"
#include "../TechnoType/Body.h"

#include <HouseClass.h>

DEFINE_HOOK(70A990, TechnoClass_DrawVeterancy, 5)
{
	GET(TechnoClass *, T, ECX);
	GET_STACK(Point2D *, XY, 0x4);
	GET_STACK(RectangleStruct *, pRect, 0x8);

	Point2D offset = *XY;

	SHPStruct *iFile = FileSystem::PIPS_SHP;
	int iFrame = -1;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());

	bool canSee = T->Owner->IsAlliedWith(HouseClass::Player)
		|| HouseClass::IsPlayerObserver()
		|| pTypeData->Insignia_ShowEnemy.Get(RulesExt::Global()->EnemyInsignia);

	if(!canSee) {
		iFrame = -1;
	} else if(SHPStruct *fCustom = pTypeData->Insignia.Get(T)) {
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
		if(T->WhatAmI() != AbstractType::Infantry) {
			offset.X += 5;
			offset.Y += 4;
		}

		DSurface::Hidden_2->DrawSHP(
			FileSystem::THEATER_PAL, iFile, iFrame, &offset, pRect, BlitterFlags(0xE00), 0, -2, 0, 1000, 0, 0, 0, 0, 0);
	}

	return 0x70AA5B;
}
