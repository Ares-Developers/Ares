#include "Body.h"
#include "../../Misc/EMPulse.h"

DEFINE_HOOK(5240BD, CyborgParsingMyArse, 7) {
	// Otherwise the TechnoType's Cyborg tag is not parsed.
	return 0x5240C4;
}

DEFINE_HOOK(6FAF0D, TechnoClass_Update_EMPLock, 6) {
	GET(TechnoClass *, pThis, ESI);

	// original code.
	if (pThis->EMPLockRemaining) {
		--pThis->EMPLockRemaining;
		if (!pThis->EMPLockRemaining) {
			// the forced vacation just ended. we use our own
			// function here that is quicker in retrieving the
			// EMP animation and does more stuff.
			EMPulse::DisableEMPEffect(pThis);
		}
	}

	return 0x6FAFFD;
}
