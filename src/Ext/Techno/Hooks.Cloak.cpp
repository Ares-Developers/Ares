#include "Body.h"

// replace the cloak checking functions to include checks for new features
DEFINE_HOOK(6FB757, TechnoClass_UpdateCloak, 8)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	bool cloakable = !pExt->CloakDisallowed(false) && pExt->IsReallyCloakable();

	return cloakable ? 0x6FB7FD : 0x6FB75F;
}

DEFINE_HOOK(6FBDC0, TechnoClass_ShouldBeCloaked, 5)
{
	GET(TechnoClass*, pThis, ECX);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	bool ret = pExt->CloakAllowed(true) && pExt->IsReallyCloakable();

	R->EAX(ret ? 1 : 0);
	return 0x6FBF93;
}

DEFINE_HOOK(6FBC90, TechnoClass_ShouldNotBeCloaked, 5)
{
	GET(TechnoClass*, pThis, ECX);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	bool ret = (!pThis->Cloakable && pExt->CloakDisallowed(true)) || !pExt->IsReallyCloakable();

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
	R->ECX(pExt->DecloakSound.Get(RulesClass::Instance->CloakSound));
	return 0x703761;
}
