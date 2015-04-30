//#include "Body.h"

#include "../TechnoType/Body.h"

#include <HouseClass.h>
#include <InfantryClass.h>

DEFINE_HOOK(4140EB, AircraftClass_DTOR_Prereqs, 6)
{
	GET(UnitClass* const, pThis, EDI);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	if(pExt->IsGenericPrerequisite()) {
		pThis->Owner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_HOOK(517DF2, InfantryClass_DTOR_Prereqs, 6)
{
	GET(InfantryClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	if(pExt->IsGenericPrerequisite()) {
		pThis->Owner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_HOOK(7357F6, UnitClass_DTOR_Prereqs, 6)
{
	GET(UnitClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	if(pExt->IsGenericPrerequisite()) {
		pThis->Owner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_HOOK(4D7221, FootClass_Put_Prereqs, 6)
{
	GET(FootClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	if(pExt->IsGenericPrerequisite()) {
		pThis->Owner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_HOOK_AGAIN(6F4A37, TechnoClass_DiscoveredBy_Prereqs, 5)
DEFINE_HOOK(6F4A1D, TechnoClass_DiscoveredBy_Prereqs, 6)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	if(pThis->WhatAmI() != AbstractType::Building && pExt->IsGenericPrerequisite()) {
		pThis->Owner->RecheckTechTree = true;
	}

	return 0;
}

DEFINE_HOOK(7015EB, TechnoClass_ChangeOwnership_Prereqs, 7)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(HouseClass* const, pNewOwner, EBP);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	if(pThis->WhatAmI() != AbstractType::Building && pExt->IsGenericPrerequisite()) {
		pThis->Owner->RecheckTechTree = true;
		pNewOwner->RecheckTechTree = true;
	}

	return 0;
}
