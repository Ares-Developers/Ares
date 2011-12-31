#include "Body.h"
#include "../Building/Body.h"
//include "Side.h"
#include "../../Enum/Prerequisites.h"
#include "../../Misc/Debug.h"

// =============================
// other hooks


DEFINE_HOOK(0x732D10, TacticalClass_CollectSelectedIDs, 0x5)
{
	return 0;
}

/*
A_FINE_HOOK(5F8480, ObjectTypeClass_Load3DArt, 6)
{
	GET(ObjectTypeClass *, O, ESI);
	if(O->WhatAmI() == abs_UnitType) {
		TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(reinterpret_cast<TechnoTypeClass*>(O));
		if(pData->WaterAlt) {
			return 0x5F848C;
		}
	}
	return 0;
}
*/

DEFINE_HOOK(0x715320, TechnoTypeClass_LoadFromINI_EarlyReader, 0x6)
{
	GET(CCINIClass *, pINI, EDI);
	GET(TechnoTypeClass *, pType, EBP);

	INI_EX exINI(pINI);
	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(pType);

	pData->WaterImage.Parse(&exINI, pType->ID, "WaterImage");

	return 0;
}


DEFINE_HOOK(0x5F79B0, TechnoTypeClass_FindFactory, 0x6)
{
	enum { Eligible = 0x5F79C7, Ineligible = 0x5F7A57 };

	GET(TechnoTypeClass *, pProduction, EDI);
	GET(BuildingClass *, pFactory, ESI);

	auto pData = TechnoTypeExt::ExtMap.Find(pProduction);

	return (pData->CanBeBuiltAt(pFactory->Type))
		? Eligible
		: Ineligible
	;
}

DEFINE_HOOK(0x4444E2, BuildingClass_KickOutUnit_FindAlternateKickout, 0x6)
{
	GET(BuildingClass *, Src, ESI);
	GET(BuildingClass *, Tst, EBP);
	GET(TechnoClass *, Production, EDI);

	auto pData = TechnoTypeExt::ExtMap.Find(Production->GetTechnoType());

	if(Src != Tst
	 && Tst->GetCurrentMission() == mission_Guard
	 && Tst->Type->Factory == Src->Type->Factory
	 && Tst->Type->Naval == Src->Type->Naval
	 && pData->CanBeBuiltAt(Tst->Type)
	 && !Tst->Factory)
	{
		return 0x44451F;
	}

	return 0x444508;
}


DEFINE_HOOK(0x444159, BuildingClass_KickOutUnit_Clone, 0x6) {
	GET(TechnoClass *, Production, EDI);
	GET(BuildingClass *, Factory, ESI);

	auto pFactoryData = BuildingExt::ExtMap.Find(Factory);
	pFactoryData->KickOutClones(Production);

	return 0;
}

DEFINE_HOOK(0x4449DF, BuildingClass_KickOutUnit_PreventClone, 0x6)
{
	return 0x444A53;
}
