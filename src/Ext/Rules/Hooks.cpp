#include "Body.h"
#include "../../Ares.h"

#include <OverlayClass.h>

DEFINE_HOOK(668BF0, RulesClass_Addition, 5)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

//	RulesClass::Initialized = false;
	Ares::GlobalControls::Load(pINI);
	RulesExt::LoadFromINIFile(pItem, pINI);
	return 0;
}

DEFINE_HOOK(679A15, RulesData_LoadBeforeTypeData, 6)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

//	RulesClass::Initialized = true;
	RulesExt::LoadBeforeTypeData(pItem, pINI);
	return 0;
}

DEFINE_HOOK(679CAF, RulesData_LoadAfterTypeData, 5)
{
	RulesClass* pItem = RulesClass::Global();
	GET(CCINIClass*, pINI, ESI);

	RulesExt::LoadAfterTypeData(pItem, pINI);
	return 0;
}

DEFINE_HOOK(518744, InfantryClass_ReceiveDamage_ElectricDeath, 6)
{
	AnimTypeClass *El = RulesExt::Global()->ElectricDeath;
	if(!El) {
		El = AnimTypeClass::Find("ELECTRO");
	}
	if(!El) {
		El = AnimTypeClass::Array->GetItem(1);
	}

	R->EDX(El);
	return 0x51874D;
}

DEFINE_HOOK(48A2D9, DamageArea_ExplodesThreshold, 6)
{
	GET(OverlayTypeClass*, pOverlay, EAX);
	GET_STACK(int, damage, 0x24);

	bool explodes = pOverlay->Explodes && damage >= RulesExt::Global()->OverlayExplodeThreshold;

	return explodes ? 0x48A2E7 : 0x48A433;
}

// TiberiumTransmogrify is never initialized explitly, thus do that here
DEFINE_HOOK(66748A, RulesClass_CTOR_TiberiumTransmogrify, 6)
{
	GET(RulesClass*, pThis, ESI);
	pThis->TiberiumTransmogrify = 0;
	return 0;
}
