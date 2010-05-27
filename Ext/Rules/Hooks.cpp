#include "Body.h"
#include "../../Ares.h"

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

DEFINE_HOOK(671DF1, RulesClass_Addition_General_MultiEngineer, 7) {
	GET(RulesClass*, pRules, ESI);
	GET(CCINIClass*, pINI, EDI);

	// read the engineer capture level ourselves. we default to 100%,
	// the game defaulted to 0% so engineers could only capture buildings
	// when they have been destroyed.
	pRules->EngineerCaptureLevel = pINI->ReadDouble("General", "EngineerCaptureLevel", 1.0f);
	pRules->EngineerCaptureLevel_ = pRules->EngineerCaptureLevel;

	return 0x671E3B;
}