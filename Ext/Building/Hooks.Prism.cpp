#include "Body.h"
#include "../BuildingType/Body.h"
#include <BulletClass.h>
#include <LaserDrawClass.h>

DEFINE_HOOK(44B2FE, BuildingClass_Mi_Attack_IsPrism, 6)
{
	GET(BuildingClass *, B, ESI);
	GET(int, idxWeapon, EBP); //which weapon was chosen to attack the target with

	enum { IsPrism = 0x44B310, IsNotPrism = 0x44B630, IsCustomPrism = 0x44B6D6};

	BuildingTypeClass *pMasterType = B->Type;
	BuildingTypeExt::ExtData *pMasterTypeData = BuildingTypeExt::ExtMap.Find(pMasterType);

	if (pMasterTypeData->PrismForwarding.Enabled == BuildingTypeExt::cPrismForwarding::YES
		|| pMasterTypeData->PrismForwarding.Enabled == BuildingTypeExt::cPrismForwarding::ATTACK) {

		if (B->PrismStage == pcs_Idle) {
			B->PrismStage = pcs_Master;
			B->DelayBeforeFiring = B->Type->DelayedFireDelay;
			B->PrismTargetCoords.X = 0;
			B->PrismTargetCoords.Y = B->PrismTargetCoords.Z = 0;
			B->DestroyNthAnim(BuildingAnimSlot::Active);
			B->PlayNthAnim(BuildingAnimSlot::Special);

			int LongestChain = 0;

			//set up slaves
			int NetworkSize = 0;
			int stage = 1;

			//when it reaches zero we can't acquire any more slaves
			while (BuildingTypeExt::cPrismForwarding::AcquireSlaves_MultiStage(B, B, stage++, 0, &NetworkSize, &LongestChain) != 0) {}

			//now we have all the towers we know the longest chain, and can set all the towers' charge delays
			BuildingTypeExt::cPrismForwarding::SetPrismChargeDelay(B);

		} else if (B->PrismStage == pcs_Slave) {
			//a slave tower is changing into a master tower at the last second
			B->PrismStage = pcs_Master;
			BuildingExt::ExtData *pMasterData = BuildingExt::ExtMap.Find(B);
			pMasterData->PrismForwarding.SupportTarget = NULL;

		}

		return IsCustomPrism; //always custom, the new code is a complete rewrite of the old code

	}

	return IsNotPrism;
}

DEFINE_HOOK(447FAE, BuildingClass_GetObjectActivityState, 6)
{
	GET(BuildingClass *, B, ESI);
	enum { BusyCharging = 0x447FB8, NotBusyCharging = 0x447FC3};

	if(B->DelayBeforeFiring > 0) {
		//if this is a slave prism tower, then it might still be able to become a master tower at this time
		BuildingTypeClass *pType = B->Type;
		BuildingTypeExt::ExtData *pTypeData = BuildingTypeExt::ExtMap.Find(pType);
		if (pTypeData->PrismForwarding.Enabled == BuildingTypeExt::cPrismForwarding::YES
				|| pTypeData->PrismForwarding.Enabled == BuildingTypeExt::cPrismForwarding::ATTACK) {
			//is a prism tower
			if (B->PrismStage == pcs_Slave && pTypeData->PrismForwarding.BreakSupport) {
				return NotBusyCharging;
			}
		}
		return BusyCharging;
	}
	return NotBusyCharging;
}

//NB: PrismTargetCoords is not a coord struct, it's some kind of garbage whose first dword is the used weapon index and two others are undefined...
DEFINE_HOOK(4503F0, BuildingClass_Update_Prism, 9)
{
	int end = 0x4504E2;
	GET(BuildingClass *, pThis, ECX);
	if(int PrismStage = pThis->PrismStage) {
		BuildingExt::ExtData *pData = BuildingExt::ExtMap.Find(pThis);
		if (pThis->DelayBeforeFiring <= 0) {
			--pThis->DelayBeforeFiring;
			if(pThis->DelayBeforeFiring <= 0) {
				if(PrismStage == pcs_Slave) {
					if (BuildingClass *pTarget = pData->PrismForwarding.SupportTarget) {
						BuildingExt::ExtData *pTargetData = BuildingExt::ExtMap.Find(pTarget);
						BuildingTypeClass *pType = pThis->Type;
						BuildingTypeExt::ExtData *pTypeData = BuildingTypeExt::ExtMap.Find(pType);
						pTargetData->PrismForwarding.ModifierReserve += (pTypeData->PrismForwarding.SupportModifier + pData->PrismForwarding.ModifierReserve);
						pTargetData->PrismForwarding.DamageReserve += (pTypeData->PrismForwarding.DamageAdd  + pData->PrismForwarding.DamageReserve);
						pThis->FireLaser(pThis->PrismTargetCoords);

					}
				}
				if(PrismStage == pcs_Master) {
					if(pThis->Target) {
						if(pThis->GetFireError(pThis->Target, pThis->PrismTargetCoords.X, true) == FireError::OK) {
							if(BulletClass *LaserBeam = pThis->Fire(pThis->Target, pThis->PrismTargetCoords.X)) {
								BuildingTypeClass *pType = pThis->Type;
								BuildingTypeExt::ExtData *pTypeData = BuildingTypeExt::ExtMap.Find(pType);
								double DamageMult = pTypeData->PrismForwarding.SupportModifier + pData->PrismForwarding.ModifierReserve;
								LaserBeam->DamageMultiplier = ((DamageMult + 100) * 256) / 100; //apparently this is divided by 256 elsewhere
								LaserBeam->Health += pTypeData->PrismForwarding.DamageAdd  + pData->PrismForwarding.DamageReserve;
							}
						}
					}
				}
				//This tower's job is done. Go idle.
				pData->PrismForwarding.ModifierReserve = 0.0;
				pData->PrismForwarding.DamageReserve = 0;
				pData->PrismForwarding.Senders.Clear();
				pThis->SupportingPrisms = 0; //Ares doesn't actually use this, but maintaining it anyway (as direct feeds only)
				pData->PrismForwarding.SupportTarget = NULL;
				pThis->PrismStage = pcs_Idle;
			}
		} else {
			//still in delayed charge so not actually charging yet
			--pThis->DelayBeforeFiring;
			pThis->DestroyNthAnim(BuildingAnimSlot::Active);
			pThis->PlayNthAnim(BuildingAnimSlot::Special);
		}
	}
	return end;
}

DEFINE_HOOK(44ABD0, BuildingClass_FireLaser, 5)
{
	GET(BuildingClass *, B, ECX);
	LEA_STACK(CoordStruct *, pTargetXYZ, 0x4);

	BuildingTypeClass *pType = B->Type;
	BuildingTypeExt::ExtData *pTypeData = BuildingTypeExt::ExtMap.Find(pType);

	CoordStruct SourceXYZ, Base = {0, 0, 0};
	B->GetFLH(&SourceXYZ, 0, Base);

	ColorStruct blank(0, 0, 0);

	LaserDrawClass * LaserBeam;
	GAME_ALLOC(LaserDrawClass, LaserBeam, SourceXYZ, *pTargetXYZ, B->Owner->LaserColor, blank, blank, pTypeData->PrismForwarding.SupportDuration);

	if(LaserBeam) {
		LaserBeam->IsHouseColor = true;
		LaserBeam->field_1C = 3;
	}

	B->SupportingPrisms = 0;
	B->ReloadTimer.Start(pTypeData->PrismForwarding.SupportDelay);

	return 0x44ACE2;
}

//================================================================
//========BELOW: THE ORIGINAL CODE AS TRANSCRIBED BY DCODER=======
//================================================================

/*
 * whoo this is a complex logic:
 * when a building enters the Attack mission, it (after handling a special case of SAM=yes) does the following:
 * int error = this->GetFireError(target, secondary);
 * if(error == FireError::Facing) {
 *  // turn to face the right way, return and retry next frame
 * }
 * switch(error) {
 * 	case FireError::OK:
 * 	// if the building has upgrades, fire them and return
 * 	// if the building is not a Prism, set its DelayedFireDelay or fire its weapon if it doesn't have one. return
 * 	// otherwise do the stuff in the Attack_IsPrism hook below
 * 	return;
 *
 * 	case FireError::Rearm:
 *  return and retry 2 frames later;
 *
 *  // other stuff
 * }
 *
 * GetFireError returns FireError::Rearm if the asking building has DelayedFireDelay set
 * which is set once the prism has exhausted its search of potential slaves, or reached maximum slave amount
 * and gets decremented each frame inside UpdatePrism
 *
 *
 */

/*DEFINE_HOOK(44B2FE, BuildingClass_Mi_Attack_IsPrism, 6)
{
	GET(BuildingClass *, B, ESI);
	GET(int, idxWeapon, EBP); // which weapon was chosen to attack the target with

	// IsPrism - will find a single support Westwood style and apply the effects to it
	// IsCustomPrism - Ares code is used to find supports manually and TODO pseudocode all needed stuff
	enum { IsPrism = 0x44B310, IsNotPrism = 0x44B630, IsCustomPrism = 0x44B6D6};

	if(B->Type == RulesClass::Instance->PrismType) {
		bool TimeToStartCharge = false;
		if(B->SupportingPrisms < RulesClass::Instance->PrismSupportMax) {
			TimeToStartCharge = true;
			// find a slave
			int nearestDistance = 0x7FFFFFFF;
			BuildingClass * nearestPrism = NULL;

			CoordStruct MyPosition, curPosition;

			B->GetPosition_2(&MyPosition);

			int Range = B->GetWeaponRange(1);

			for(int i = 0; i < B->Owner->Buildings.Count; ++i) {
				if(BuildingClass * curBld = B->Owner->Buildings[i]) {
					if(curBld->IsAlive && curBld->Type == RulesClass::Instance->PrismType) {
						if(curBld->ReloadTimer.Ignorable()) {
							if(curBld != B && !curBld->DelayBeforeFiring) {
								if(!curBld->IsBeingDrained() && curBld->GetCurrentMission() != mission_Attack) {
									curBld->GetPosition_2(&curPosition);
									int Distance = MyPosition.DistanceFrom(curPosition);
									if(Distance <= Range) {
										if(!nearestPrism || Distance < nearestDistance) {
											nearestPrism = curBld;
											nearestDistance = Distance;
										}
									}
								}
							}
						}
					}
				}
			}

			if(nearestPrism) {
				TimeToStartCharge = false;
				++B->SupportingPrisms;
				CoordStruct FLH, Base = {0, 0, 0};
				B->GetFLH(&FLH, 0, Base);
				nearestPrism->DelayBeforeFiring = nearestPrism->Type->DelayedFireDelay;
				nearestPrism->PrismStage = pcs_Slave;
				nearestPrism->PrismTargetCoords = FLH;
				nearestPrism->DestroyNthAnim(BuildingAnimSlot::Active);
				nearestPrism->PlayNthAnim(BuildingAnimSlot::Special);
			}
		}

		if(TimeToStartCharge) {
			// set up master - it seems this happens (and resets the Delay) each time the function is triggered which is every 1 frame...
			B->DelayBeforeFiring = B->Type->DelayedFireDelay;
			B->PrismStage = pcs_Master;
			B->PrismTargetCoords.X = 0;
			B->PrismTargetCoords.Y = B->PrismTargetCoords.Z = 0; // these are set to uninitialized vars really, but don't seem to be used for master
			B->DestroyNthAnim(BuildingAnimSlot::Active);
			B->PlayNthAnim(BuildingAnimSlot::Special);
		}
	}

	return IsNotPrism;
}*/


/*DEFINE_HOOK(447FAE, BuildingClass_GetObjectActivityState, 6)
{
	GET(BuildingClass *, B, ESI);
	enum { BusyCharging = 0x447FB8, NotBusyCharging = 0x447FC3};
	if(pThis->DelayBeforeFiring > 0) {
		return BusyCharging;
	}
	return NotBusyCharging;
}*/

// note: PrismTargetCoords is not a coord struct, it's some kind of garbage whose first dword is the used weapon index and two others are undefined...
// todo figure it out!
/*DEFINE_HOOK(4503F0, BuildingClass_Update_Prism, 9)
{
	int end = 0x4504E2;
	GET(BuildingClass *, pThis, ECX);
	if(int PrismStage = pThis->PrismStage) {
		--pThis->DelayBeforeFiring;
		if(pThis->DelayBeforeFiring <= 0) {
			if(PrismStage == pcs_Slave) {
				pThis->FireLaser(pThis->PrismTargetCoords);
				pThis->PrismStage = pcs_Idle;
				return end;
			}
			if(PrismStage == pcs_Master) {
				if(pThis->Target) {
					if(pThis->GetFireError(pThis->Target, pThis->PrismTargetCoords.X, true) == FireError::OK) {
						if() {
							if(int Supporters = pThis->SupportingPrisms) {
								BulletClass *LaserBeam = pThis->Fire(pThis->Target, pThis->PrismTargetCoords.X)
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
}*/

/*DEFINE_HOOK(44ABD0, BuildingClass_FireLaser, 5)
{
	GET(BuildingClass *, B, ECX);
	LEA_STACK(CoordStruct *, TargetXYZ, 0x4);

	CoordStruct SourceXYZ, Base = {0, 0, 0};
	B->GetFLH(&SourceXYZ, 0, Base);

	ColorStruct blank = {0, 0, 0};

	LaserDrawClass * LaserBeam;
	GAME_ALLOC(LaserDrawClass, LaserBeam, SourceXYZ, TargetXYZ, B->Owner->LaserColor, blank, blank, RulesClass::Instance->PrismSupportDuration);

	if(LaserBeam) {
		LaserBeam->IsHouseColor = true;
		LaserBeam->field_1C = 3;
	}

	B->SupportingPrisms = 0;
	B->ReloadTimer.Start(RulesClass::Instance->PrismSupportDelay);

	return 0x44ACE2;
}*/
