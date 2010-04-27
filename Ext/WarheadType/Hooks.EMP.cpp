#include "Body.h"
#include "../../Misc/EMPulse.h"

DEFINE_HOOK(5240BD, CyborgParsingMyArse, 7) {
	// Otherwise the TechnoType's Cyborg tag is not parsed.
	return 0x5240C4;
}

DEFINE_HOOK(4C54E0, EMPulseClass_Initialize, 6) {
	GET(EMPulseClass *, pThis, ECX);
	GET_STACK(TechnoClass *, pGenerator, 0x4);

	// legacy way. don't use.
	Debug::DevLog(Debug::Warning, "EMPulseClass_Initialize stumbled upon an EMPClass instance of warhead %s. Use EMPulse instead.\n",
		(WarheadTypeExt::EMP_WH ? WarheadTypeExt::EMP_WH->ID : "<none>"));

	// completely new implementation of the EMPulse.
	EMPulse::CreateEMPulse(pThis, pGenerator);

	// skip old function entirely.
	return 0x4C58B6;
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
