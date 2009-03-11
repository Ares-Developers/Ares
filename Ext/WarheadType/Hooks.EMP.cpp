#include "Body.h"

DEFINE_HOOK(5240BD, CyborgParsingMyArse, 7)
{
	return 0x5240C4;
}

//Feature #200: EMP Warheads
DEFINE_HOOK(4C5824, EMPulseClass_Initialize1, 5)
{
	GET(int, Duration, EDX);
	GET(TechnoClass *, curVictim, ESI);
	Duration += curVictim->EMPLockRemaining;
	R->set_EDX(Duration);
	return 0;
}

DEFINE_HOOK(4C5718, EMPulseClass_Initialize2, 6)
{
	GET(int, Duration, EAX);
	GET(TechnoClass *, curVictim, ESI);
	Duration += curVictim->EMPLockRemaining;
	R->set_EAX(Duration);
	return 0;
}

DEFINE_HOOK(4C575E, EMPulseClass_CyborgCheck, 7)
{
	GET(TechnoClass *, curVictim, ESI);
	TechnoTypeClass* pType = (TechnoTypeClass*)curVictim->GetType();
	return pType->Cyborg_ ? 0x4C577A : 0;
}
