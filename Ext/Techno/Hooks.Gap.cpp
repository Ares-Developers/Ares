#include "Body.h"
#include "../TechnoType/Body.h"

#include "../../Misc/Debug.h"

DEFINE_HOOK(6FB191, TechnoClass_CreateGap, 8)
{
	GET(TechnoClass *, T, ESI);
	bool canGap = false;
	if(T->WhatAmI() != abs_Building) {
		canGap = T->IsAlive && !T->InLimbo && !T->Deactivated;
	} else {
		canGap = reinterpret_cast<BuildingClass *>(T)->IsPowerOnline();
	}
	return canGap ? 0x6FB1A1 : 0x6FB45C;
}

DEFINE_HOOK(4D8642, FootClass_UpdatePosition, 6)
{
	GET(FootClass *, F, ESI);
	TechnoTypeClass *Type = F->GetTechnoType();
	if(Type->GapGenerator && Type->GapRadiusInCells > 0) {
		CellStruct curLocation;
		F->GetMapCoords(&curLocation);
		if(curLocation != F->LastMapCoords) {
			TechnoExt::NeedsRegap = 1;
			F->CreateGap();
			F->DestroyGap();
		}
	}
	return 0;
}

// lolhax
DEFINE_HOOK(6FB4B1, TechnoClass_DeleteGap_new, 6)
{
	GET(FootClass *, F, ESI);
	if(F->WhatAmI() == abs_Building) {
		R->EDX<CoordStruct *>(&F->Location);
	} else {
		CoordStruct *XYZ = new CoordStruct;
		MapClass::Instance->GetCellAt(&F->LastMapCoords)->GetCoords(XYZ);
		R->EDX<CoordStruct *>(XYZ);
	}
	return 0x6FB4B7;
}

DEFINE_HOOK(6FB4D1, TechnoClass_DeleteGap_delete, 5)
{
	GET(FootClass *, F, ESI);
	if(F->WhatAmI() != abs_Building) {
		GET(CoordStruct *, XYZ, EDX);
		delete XYZ;
	}
	return 0;
}

DEFINE_HOOK(6F6B66, TechnoClass_Remove_DeleteGap, A)
{
	GET(TechnoClass *, T, ESI);
	if(T->WhatAmI() == abs_Building) {
		T->DestroyGap();
	} else {
		TechnoExt::NeedsRegap = 1;
		FootClass *F = reinterpret_cast<FootClass *>(T);
		CellStruct buffer = F->LastMapCoords;
		F->LastMapCoords = F->CurrentMapCoords;
		F->DestroyGap();
		F->LastMapCoords = buffer;
	}
	return 0x6F6B70;
}

DEFINE_HOOK(6FB446, TechnoClass_CreateGap_RefreshMap, 5)
{
	return TechnoExt::NeedsRegap ? 0x6FB45C : 0;
}

DEFINE_HOOK(6FB723, TechnoClass_DeleteGap_RefreshMap, 5)
{
	return TechnoExt::NeedsRegap ? 0x6FB739 : 0;
}

DEFINE_HOOK(55AFB3, LogicClass_Update_Gaps, 6)
{
	if(TechnoExt::NeedsRegap) {
		TechnoExt::NeedsRegap = 0;
		MapClass::Instance->sub_657CE0();
		MapClass::Instance->sub_4F42F0(2);
	}
	return 0;
}
