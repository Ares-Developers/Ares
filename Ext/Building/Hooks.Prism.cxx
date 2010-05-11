#include "Body.h"
#include <BulletClass.h>

DEFINE_HOOK(44B2FE, BuildingClass_Mi_Attack_IsPrism, 6)
{
	GET(BuildingClass *, B, ESI);
	GET(int, idxWeapon, EBP); // which weapon was chosen to attack the target with

	// IsPrism - will find a single support Westwood style and apply the effects to it
	// IsCustomPrism - Ares code is used to find supports manually and TODO pseudocode all needed stuff
	enum { IsPrism = 0x44B310, IsNotPrism = 0x44B630, IsCustomPrism = 0x44B6D6};
	return IsNotPrism;
}


DEFINE_HOOK(447FAE, BuildingClass_GetObjectActivityState, 6)
{
	GET(BuildingClass *, B, ESI);
	enum { PrismCharging = 0x447FB8, PrismNotBusy = 0x447FC3};
	return PrismNotBusy; // self explanatory I think
}

// note: PrismTargetCoords is not a coord struct, it's some kind of garbage whose first dword is the used weapon index and two others are undefined...
// todo figure it out!
DEFINE_HOOK(4503F0, BuildingClass_Update_Prism, 9)
{
	int end = 0x4504E2;
	GET(BuildingClass *, pThis, ECX);
	if(int PrismStage = pThis->PrismStage) {
		--pThis->PrismReloadDelay;
		if(pThis->PrismReloadDelay <= 0) {
			if(PrismStage == pcs_Firing) {
				pThis->FireLaser(pThis->PrismTargetCoords);
				pThis->PrismStage = pcs_Idle;
				return end;
			}
			if(PrismStage == pcs_Charging) {
				if(pThis->Target) {
					if(pThis->GetFireError(pThis->Target, pThis->PrismTargetCoords.X, true) == FireError::OK) {
						if(BulletClass *LaserBeam = pThis->Fire(pThis->Target, pThis->PrismTargetCoords.X)) {
							if(int Supporters = pThis->SupportingPrisms) {
								LaserBeam->DamageMultiplier = ((RulesClass::Instance->PrismSupportModifier * Supporters + 100) * 256) / 100;
								pThis->SupportingPrisms = 0;
							}
						}
					}
				}
			}
			pThis->PrismStage = pcs_Idle;
		}
	}
	return end;
}
