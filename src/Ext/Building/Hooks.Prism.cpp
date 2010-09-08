#include "Body.h"
#include "../BuildingType/Body.h"
#include "../WeaponType/Body.h"
#include <BulletClass.h>
#include <LaserDrawClass.h>

DEFINE_HOOK(44B2FE, BuildingClass_Mi_Attack_IsPrism, 6)
{
	GET(BuildingClass *, B, ESI);
	GET(int, idxWeapon, EBP); //which weapon was chosen to attack the target with
	R->EAX<BuildingTypeClass *>(B->Type);

	enum { IsPrism = 0x44B310, IsNotPrism = 0x44B630, IsCustomPrism = 0x44B6D6};

	BuildingTypeClass *pMasterType = B->Type;
	BuildingTypeExt::ExtData *pMasterTypeData = BuildingTypeExt::ExtMap.Find(pMasterType);

	if (pMasterTypeData->PrismForwarding.Enabled == BuildingTypeExt::cPrismForwarding::YES
		|| pMasterTypeData->PrismForwarding.Enabled == BuildingTypeExt::cPrismForwarding::ATTACK) {

		BuildingExt::ExtData *pMasterData = BuildingExt::ExtMap.Find(B);

		if (B->PrismStage == pcs_Idle) {
			B->PrismStage = pcs_Master;
			B->DelayBeforeFiring = B->Type->DelayedFireDelay;

			B->PrismTargetCoords.X = 0;
			if (pMasterType->get_Secondary()) {
				if (B->IsOverpowered) {
					B->PrismTargetCoords.X = 1;
				}
			}

			B->PrismTargetCoords.Y = B->PrismTargetCoords.Z = 0;
			pMasterData->PrismForwarding.ModifierReserve = 0.0;
			pMasterData->PrismForwarding.DamageReserve = 0;

			int LongestChain = 0;

			//set up slaves
			int NetworkSize = 0;
			int stage = 0;

			//when it reaches zero we can't acquire any more slaves
			while (BuildingTypeExt::cPrismForwarding::AcquireSlaves_MultiStage(B, B, stage++, 0, &NetworkSize, &LongestChain) != 0) {}

			//now we have all the towers we know the longest chain, and can set all the towers' charge delays
			BuildingTypeExt::cPrismForwarding::SetChargeDelay(B, LongestChain);

		} else if (B->PrismStage == pcs_Slave) {
			Debug::Log("PrismForwarding: Converting Slave to Master\n");
			//a slave tower is changing into a master tower at the last second
			B->PrismStage = pcs_Master;
			B->PrismTargetCoords.X = 0;
			if (pMasterType->get_Secondary()) {
				if (B->IsOverpowered) {
					B->PrismTargetCoords.X = 1;
				}
			}
			B->PrismTargetCoords.Y = B->PrismTargetCoords.Z = 0;
			pMasterData->PrismForwarding.ModifierReserve = 0.0;
			pMasterData->PrismForwarding.DamageReserve = 0;
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

//NB: PrismTargetCoords is not just a coord struct, it's a union whose first dword is the used weapon index and two others are undefined...
DEFINE_HOOK(4503F0, BuildingClass_Update_Prism, 9)
{
	GET(BuildingClass *, pThis, ECX);
	if(int PrismStage = pThis->PrismStage) {
		BuildingExt::ExtData *pData = BuildingExt::ExtMap.Find(pThis);
		if (pData->PrismForwarding.PrismChargeDelay <= 0) {
			--pThis->DelayBeforeFiring;
			if(pThis->DelayBeforeFiring <= 0) {
				if(PrismStage == pcs_Slave) {
					if (BuildingClass *pTarget = pData->PrismForwarding.SupportTarget) {
						BuildingExt::ExtData *pTargetData = BuildingExt::ExtMap.Find(pTarget);
						BuildingTypeClass *pType = pThis->Type;
						BuildingTypeExt::ExtData *pTypeData = BuildingTypeExt::ExtMap.Find(pType);
						Debug::Log("[PrismForwarding] Slave firing. SM=%d MR=%lf\n",
							pTypeData->PrismForwarding.SupportModifier.Get(), pData->PrismForwarding.ModifierReserve);
						pTargetData->PrismForwarding.ModifierReserve +=
							(pTypeData->PrismForwarding.SupportModifier.Get() + pData->PrismForwarding.ModifierReserve);
						pTargetData->PrismForwarding.DamageReserve +=
							(pTypeData->PrismForwarding.DamageAdd.Get()  + pData->PrismForwarding.DamageReserve);
						pThis->FireLaser(pThis->PrismTargetCoords);

					}
				}
				if(PrismStage == pcs_Master) {
					if(ObjectClass *Target = pThis->Target) {
						if(pThis->GetFireError(Target, pThis->PrismTargetCoords.X, true) == FireError::OK) {
							if(BulletClass *LaserBeam = pThis->Fire(Target, pThis->PrismTargetCoords.X)) {
								BuildingTypeClass *pType = pThis->Type;
								BuildingTypeExt::ExtData *pTypeData = BuildingTypeExt::ExtMap.Find(pType);

								//apparently this is divided by 256 elsewhere
								LaserBeam->DamageMultiplier = ((pData->PrismForwarding.ModifierReserve + 100) * 256) / 100;
								LaserBeam->Health += pTypeData->PrismForwarding.DamageAdd.Get()  + pData->PrismForwarding.DamageReserve;
							}
						}
					}
				}
				//This tower's job is done. Go idle.
				pData->PrismForwarding.ModifierReserve = 0.0;
				pData->PrismForwarding.DamageReserve = 0;
				pData->PrismForwarding.Senders.Clear();
				pThis->SupportingPrisms = 0; //Ares sets this to the longest backward chain
				pData->PrismForwarding.SupportTarget = NULL;
				pThis->PrismStage = pcs_Idle;
			}
		} else {
			//still in delayed charge so not actually charging yet
			--pData->PrismForwarding.PrismChargeDelay;
			if (pData->PrismForwarding.PrismChargeDelay <= 0) {
				//now it's time to start charging
				if (pThis->Type->BuildingAnim[BuildingAnimSlot::Special].Anim[0]) { //only if it actually has a special anim
					pThis->DestroyNthAnim(BuildingAnimSlot::Active);
					pThis->PlayNthAnim(BuildingAnimSlot::Special);
				}
			}
		}
	}
	return 0x4504E2;
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

	int idxSupport = -1;
	if (B->Veterancy.IsElite()) {
		idxSupport = pTypeData->PrismForwarding.EliteSupportWeaponIndex;
	} else {
		idxSupport = pTypeData->PrismForwarding.SupportWeaponIndex;
	}

	WeaponTypeClass * supportWeapon = NULL;
	if (idxSupport != -1) {
		supportWeapon = pType->get_Weapon(idxSupport);
	}
	LaserDrawClass * LaserBeam;
	if (supportWeapon) {
		WeaponTypeExt::ExtData *supportWeaponData = WeaponTypeExt::ExtMap.Find(supportWeapon);
		//IsLaser
		if (supportWeapon->IsLaser) {
			if (supportWeapon->IsHouseColor) {
				GAME_ALLOC(LaserDrawClass, LaserBeam, SourceXYZ, *pTargetXYZ, B->Owner->LaserColor, blank, blank, supportWeapon->LaserDuration);
			} else {
				GAME_ALLOC(LaserDrawClass, LaserBeam, SourceXYZ, *pTargetXYZ,
					supportWeapon->LaserInnerColor, supportWeapon->LaserOuterColor, supportWeapon->LaserOuterSpread,
					supportWeapon->LaserDuration);
			}
			if(LaserBeam) {
				LaserBeam->IsHouseColor = supportWeapon->IsHouseColor;
				//basic thickness (intensity additions are added later)
				if (supportWeaponData->Laser_Thickness == -1) {
					LaserBeam->Thickness = 3; //original default
				} else {
					LaserBeam->Thickness = supportWeaponData->Laser_Thickness;
				}
			}
		}
		//IsRadBeam
		Debug::Log("[Prism Forwarding] Checking IsRadBeam...\n");
		if (supportWeapon->IsRadBeam) {
			Debug::Log("[Prism Forwarding] IsRadBeam!\n");
			RadBeam* supportRadBeam;
			;GAME_ALLOC(RadBeam, supportRadBeam, 0);
			supportRadBeam->Allocate(0);
			if (supportRadBeam) {
				Debug::Log("[Prism Forwarding] Allocated!\n");
				supportRadBeam->Owner = B;
				supportRadBeam->SetCoordsSource(&SourceXYZ);
				supportRadBeam->SetCoordsTarget(pTargetXYZ);
				if (supportWeaponData->Beam_IsHouseColor) {
					supportRadBeam->Color = B->Owner->LaserColor;
					Debug::Log("[Prism Forwarding] RadBeam housecolor\n");
				} else {
					supportRadBeam->Color = supportWeaponData->Beam_Color;
					Debug::Log("[Prism Forwarding] RadBeam not housecolor\n");
				}
				Debug::Log("[Prism Forwarding] beamcolor = R(%d)G(%d)B(%d)!\n", supportRadBeam->Color.R, supportRadBeam->Color.G, supportRadBeam->Color.B);
				supportRadBeam->Period = supportWeaponData->Beam_Duration;
				supportRadBeam->Amplitude = supportWeaponData->Beam_Amplitude;
			}
		}
		//IsElectricBolt
		Debug::Log("[Prism Forwarding] Checking IsElectricBolt...\n");
		if (supportWeapon->IsElectricBolt) {
			Debug::Log("[Prism Forwarding] IsElectricBolt!\n");
			EBolt* supportEBolt;
			GAME_ALLOC(EBolt, supportEBolt);
			if (supportEBolt) {
				Debug::Log("[Prism Forwarding] Alloc'd!\n");
				supportEBolt->Owner = B;
				supportEBolt->WeaponSlot = idxSupport;
				supportEBolt->AlternateColor = supportWeapon->IsAlternateColor;
				//what is Lifetime?
				//what about that unknown_18 that might be duration?
				supportEBolt->Fire(SourceXYZ, *pTargetXYZ, 15); //3rd arg is DWORD arg18 ???
			}
		}
		//Report
		if(supportWeapon->Report.Count > 0) {
			int ReportIndex = ScenarioClass::Instance->Random.RandomRanged(0, supportWeapon->Report.Count - 1);
			int SoundArrayIndex = supportWeapon->Report.GetItem(ReportIndex);
			if(SoundArrayIndex != -1) {
				VocClass::PlayAt(SoundArrayIndex, &SourceXYZ, NULL);
			}
		}
		//ROF
		B->ReloadTimer.Start(supportWeapon->ROF);
	
	} else {
		//just the default support beam
		GAME_ALLOC(LaserDrawClass, LaserBeam, SourceXYZ, *pTargetXYZ, B->Owner->LaserColor, blank, blank, RulesClass::Instance->PrismSupportDuration);
		if(LaserBeam) {
			LaserBeam->IsHouseColor = true;
			LaserBeam->Thickness = 3;
		}
		B->ReloadTimer.Start(RulesClass::Instance->PrismSupportDelay);
	}

	//Intensity adjustment for LaserBeam
	if (LaserBeam) {
		if (pTypeData->PrismForwarding.Intensity > 0) {
			BuildingExt::ExtData *pData = BuildingExt::ExtMap.Find(B);
			LaserBeam->Thickness += (pTypeData->PrismForwarding.Intensity * (B->SupportingPrisms -1));
		}
	}

	B->SupportingPrisms = 0; //not sure why Westwood set this here. We're setting it in BuildingClass_Update_Prism

	return 0x44ACE2;
}

//these are all for cleaning up when a prism tower becomes unavailable

DEFINE_HOOK(4424EF, PrismForward_BuildingDestroyed, 6)
{
	GET(BuildingClass *, B, ESI);
	BuildingTypeExt::cPrismForwarding::RemoveFromNetwork(B, true);
	return 0;
}

DEFINE_HOOK(447113, PrismForward_BuildingSold, 6)
{
	GET(BuildingClass *, B, ESI);
	BuildingTypeExt::cPrismForwarding::RemoveFromNetwork(B, true);
	return 0;
}

DEFINE_HOOK(448277, PrismForward_BuildingChangeOwner, 5)
{
	GET(BuildingClass *, B, ESI);
	GET_STACK(HouseClass *, newOwner, 0x5C);
	
	HouseClass * oldOwner = B->Owner;

	if (newOwner != oldOwner) {
		BuildingTypeClass *pType = B->Type;
		BuildingTypeExt::ExtData *pTypeData = BuildingTypeExt::ExtMap.Find(pType);

		if (pTypeData->PrismForwarding.ToAllies) {
			BuildingClass *LastTarget = B;
			BuildingClass *FirstTarget = NULL;
			while (LastTarget) {
				BuildingExt::ExtData *pData = BuildingExt::ExtMap.Find(LastTarget);
				BuildingClass *NextTarget = pData->PrismForwarding.SupportTarget;
				if (!FirstTarget) {
					if(!NextTarget) {
						//no first target so either this is a master tower, an idle tower, or not a prism tower at all
						//no need to remove
						return 0;
					}
					FirstTarget = NextTarget;
				}

				if (!NextTarget) {
					//LastTarget is now the master (firing) tower
					if (newOwner->IsAlliedWith(LastTarget->Owner) && newOwner->IsAlliedWith(FirstTarget->Owner)) {
						//alliances check out so this slave tower can keep on charging.
						return 0;
					}
				}
				LastTarget = NextTarget;
			}
		}
		//if we reach this point then the alliance checks have failed
		BuildingTypeExt::cPrismForwarding::RemoveFromNetwork(B, false); //false because animation should continue / slave is busy but won't now fire
		
	}

	return 0;
}

DEFINE_HOOK(71AF76, PrismForward_BuildingWarped, 9) {
	GET(TechnoClass *, T, EDI);
	if (BuildingClass * B = specific_cast<BuildingClass *>(T)) {
		BuildingTypeExt::cPrismForwarding::RemoveFromNetwork(B, true);
	}
	return 0;
}


DEFINE_HOOK(70FD9A, PrismForward_BuildingDrain, 6)
{
	GET(TechnoClass *, Drainer, ESI);
	GET(TechnoClass *, Drainee, EDI);
	if(Drainee->DrainingMe != Drainer) { // else we're already being drained, nothing to do
		if (BuildingClass * B = specific_cast<BuildingClass *>(Drainee)) {
			BuildingTypeExt::cPrismForwarding::RemoveFromNetwork(B, true);
		}
	}
	return 0;
}

DEFINE_HOOK(454B3D, PrismForward_BuildingPowerDown, 6)
{
	GET(BuildingClass *, B, ESI);
	// this building just realised it needs to go offline
	// it unregistered itself from powered unit controls but hasn't done anything else yet
	BuildingTypeExt::cPrismForwarding::RemoveFromNetwork(B, true);
	return 0;
}
