#include "Body.h"
#include "../../Misc/EMPulse.h"

DEFINE_HOOK(0x5240BD, CyborgParsingMyArse, 0x7) {
	// Otherwise the TechnoType's Cyborg tag is not parsed.
	return 0x5240C4;
}

DEFINE_HOOK(0x6FAF0D, TechnoClass_Update_EMPLock, 0x6) {
	GET(TechnoClass *, pThis, ESI);

	// original code.
	if (pThis->EMPLockRemaining) {
		--pThis->EMPLockRemaining;
		if (!pThis->EMPLockRemaining) {
			// the forced vacation just ended. we use our own
			// function here that is quicker in retrieving the
			// EMP animation and does more stuff.
			EMPulse::DisableEMPEffect(pThis);
		} else {
			// deactivate units that were unloading afterwards
			if (!pThis->Deactivated && EMPulse::IsDeactivationAdvisable(pThis)) {
				pThis->Deactivate();

				// update the current mission
				if (TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(pThis)) {
					pData->EMPLastMission = pThis->CurrentMission;
				}
			}
		}
	}

	return 0x6FAFFD;
}

// copy the remaining EMP duration to the unit when undeploying a building.
DEFINE_HOOK(0x44A04C, BuildingClass_Unload_CopyEMPDuration, 0x6) {
	GET(TechnoClass *, pBuilding, EBP);
	GET(TechnoClass *, pUnit, EBX);

	// reuse the EMP duration of the deployed/undeployed Techno.
	if(pUnit && pBuilding) {
		pUnit->EMPLockRemaining = pBuilding->EMPLockRemaining;
		EMPulse::UpdateSparkleAnim(pUnit);
	}

	return 0;
}

DEFINE_HOOK(0x51E3B0, InfantryClass_GetCursorOverObject_EMP, 0x7) {
	GET(InfantryClass *, pInfantry, ECX);
	GET_STACK(TechnoClass *, pTarget, 0x4);

	// infantry should really not be able to deploy then EMP'd.
	if((pInfantry == pTarget) && pInfantry->Type->Deployer && pInfantry->IsUnderEMP()) {
		R->EAX(act_NoDeploy);
		return 0x51F187;
	}

	return 0;
}

DEFINE_HOOK(0x73D219, UnitClass_Draw_OreGatherAnim, 0x6) {
	GET(TechnoClass*, pTechno, ECX);

	// disabled ore gatherers should not appear working.
	if(pTechno->IsWarpingIn() || pTechno->IsUnderEMP()) {
		return 0x73D28E;
	}

	return 0x73D223;
}

DEFINE_HOOK(0x5F7933, TechnoTypeClass_FindFactory_ExcludeDisabled, 0x6) {
	GET(BuildingClass*, pBld, ESI);

	// add the EMP check to the limbo check
	if(pBld->InLimbo || pBld->IsUnderEMP()) {
		return 0x5F7A57;
	}

	return 0x5F7941;
}

DEFINE_HOOK(0x4456E5, BuildingClass_UpdateConstructionOptions_ExcludeDisabled, 0x6) {
	GET(BuildingClass*, pBld, ECX);

	// add the EMP check to the limbo check
	if(pBld->InLimbo || pBld->IsUnderEMP()) {
		return 0x44583E;
	}

	return 0x4456F3;
}