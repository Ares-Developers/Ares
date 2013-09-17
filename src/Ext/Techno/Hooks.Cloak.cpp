#include "Body.h"

#include "../Rules/Body.h"

// replace the cloak checking functions to include checks for new features
DEFINE_HOOK(6FB757, TechnoClass_UpdateCloak, 8)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	bool tryCloak = !pExt->CloakDisallowed(false);

	return tryCloak ? 0x6FB7FD : 0x6FB75F;
}

DEFINE_HOOK(6FBDC0, TechnoClass_ShouldBeCloaked, 5)
{
	GET(TechnoClass*, pThis, ECX);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	bool ret = pExt->CloakAllowed();

	R->EAX(ret ? 1 : 0);
	return 0x6FBF93;
}

DEFINE_HOOK(6FBC90, TechnoClass_ShouldNotBeCloaked, 5)
{
	GET(TechnoClass*, pThis, ECX);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	// the original code would not disallow cloaking as long as
	// pThis->Cloakable is set, but this prevents CloakStop from
	// working, because it overrides IsCloakable().
	bool ret = pExt->CloakDisallowed(true);

	R->EAX(ret ? 1 : 0);
	return 0x6FBDBC;
}

DEFINE_HOOK(70380A, TechnoClass_Cloak_CloakSound, 6)
{
	GET(TechnoClass*, pThis, ESI);
	auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	R->ECX(pExt->CloakSound.Get(RulesClass::Instance->CloakSound));
	return 0x703810;
}

DEFINE_HOOK(70375B, TechnoClass_Uncloak_DecloakSound, 6)
{
	GET(int, ptr, ESI);
	TechnoClass* pThis = (TechnoClass*)(ptr - 0x9C);
	auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	int default = RulesExt::Global()->DecloakSound.Get(RulesClass::Instance->CloakSound);
	R->ECX(pExt->DecloakSound.Get(default));
	return 0x703761;
}

// replace Is_Moving_Now, because it doesn't check the
// current speed in case the unit is turning.
DEFINE_HOOK(4DBDD4, FootClass_IsCloakable_CloakStop, 6)
{
	GET(FootClass*, pThis, ESI);
	R->AL(pThis->Locomotor->Is_Moving());
	return 0x4DBDE3;
}
