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
