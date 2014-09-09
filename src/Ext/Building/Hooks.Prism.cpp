#include "Body.h"
#include "../BuildingType/Body.h"
#include "../Techno/Body.h"
#include "../WeaponType/Body.h"
#include <BulletClass.h>
#include <EBolt.h>
#include <HouseClass.h>
#include <LaserDrawClass.h>
#include <ScenarioClass.h>
#include <SpawnManagerClass.h>
#include <VocClass.h>

DEFINE_HOOK(44B2FE, BuildingClass_Mi_Attack_IsPrism, 6)
{
	GET(BuildingClass *, B, ESI);
	//GET(int, idxWeapon, EBP); //which weapon was chosen to attack the target with
	R->EAX<BuildingTypeClass *>(B->Type);

	enum { IsPrism = 0x44B310, IsNotPrism = 0x44B630, IsCustomPrism = 0x44B6D6};

	BuildingTypeClass *pMasterType = B->Type;
	BuildingTypeExt::ExtData *pMasterTypeData = BuildingTypeExt::ExtMap.Find(pMasterType);

	if (pMasterTypeData->PrismForwarding.Enabled == BuildingTypeExt::cPrismForwarding::YES
		|| pMasterTypeData->PrismForwarding.Enabled == BuildingTypeExt::cPrismForwarding::ATTACK) {

		BuildingExt::ExtData *pMasterData = BuildingExt::ExtMap.Find(B);

		if (B->PrismStage == PrismChargeState::Idle) {
			B->PrismStage = PrismChargeState::Master;
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
			while(pMasterData->PrismForwarding.AcquireSlaves_MultiStage(&pMasterData->PrismForwarding, stage++, 0, NetworkSize, LongestChain) != 0) {}

			//now we have all the towers we know the longest chain, and can set all the towers' charge delays
			pMasterData->PrismForwarding.SetChargeDelay(LongestChain);

		} else if (B->PrismStage == PrismChargeState::Slave) {
			//a slave tower is changing into a master tower at the last second
			B->PrismStage = PrismChargeState::Master;
			B->PrismTargetCoords.X = 0;
			if (pMasterType->get_Secondary()) {
				if (B->IsOverpowered) {
					B->PrismTargetCoords.X = 1;
				}
			}
			B->PrismTargetCoords.Y = B->PrismTargetCoords.Z = 0;
			pMasterData->PrismForwarding.ModifierReserve = 0.0;
			pMasterData->PrismForwarding.DamageReserve = 0;
			pMasterData->PrismForwarding.SetSupportTarget(nullptr);
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
			if (B->PrismStage == PrismChargeState::Slave && pTypeData->PrismForwarding.BreakSupport) {
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

	auto PrismStage = pThis->PrismStage;
	if(PrismStage != PrismChargeState::Idle) {
		BuildingExt::ExtData *pData = BuildingExt::ExtMap.Find(pThis);
		if (pData->PrismForwarding.PrismChargeDelay <= 0) {
			--pThis->DelayBeforeFiring;
			if(pThis->DelayBeforeFiring <= 0) {
				if(PrismStage == PrismChargeState::Slave) {
					if (auto pTarget = pData->PrismForwarding.SupportTarget) {
						auto pTargetData = pTarget->Owner;
						BuildingTypeClass *pType = pThis->Type;
						BuildingTypeExt::ExtData *pTypeData = BuildingTypeExt::ExtMap.Find(pType);
						//slave firing
						pTargetData->PrismForwarding.ModifierReserve +=
							(pTypeData->PrismForwarding.GetSupportModifier() + pData->PrismForwarding.ModifierReserve);
						pTargetData->PrismForwarding.DamageReserve +=
							(pTypeData->PrismForwarding.DamageAdd  + pData->PrismForwarding.DamageReserve);
						pThis->FireLaser(pThis->PrismTargetCoords);

					}
				}
				if(PrismStage == PrismChargeState::Master) {
					if(AbstractClass *Target = pThis->Target) {
						if(pThis->GetFireError(Target, pThis->PrismTargetCoords.X, true) == FireError::OK) {
							if(BulletClass *LaserBeam = pThis->Fire(Target, pThis->PrismTargetCoords.X)) {
								BuildingTypeClass *pType = pThis->Type;
								BuildingTypeExt::ExtData *pTypeData = BuildingTypeExt::ExtMap.Find(pType);

								//apparently this is divided by 256 elsewhere
								LaserBeam->DamageMultiplier = int((pData->PrismForwarding.ModifierReserve + 100) * 256) / 100;
								LaserBeam->Health += pTypeData->PrismForwarding.DamageAdd + pData->PrismForwarding.DamageReserve;
							}
						}
					}
				}
				//This tower's job is done. Go idle.
				pData->PrismForwarding.ModifierReserve = 0.0;
				pData->PrismForwarding.DamageReserve = 0;
				pData->PrismForwarding.RemoveAllSenders();
				pThis->SupportingPrisms = 0; //Ares sets this to the longest backward chain
				pData->PrismForwarding.SetSupportTarget(nullptr);
				pThis->PrismStage = PrismChargeState::Idle;
			}
		} else {
			//still in delayed charge so not actually charging yet
			--pData->PrismForwarding.PrismChargeDelay;
			if (pData->PrismForwarding.PrismChargeDelay <= 0) {
				//now it's time to start charging
				if (pThis->Type->GetBuildingAnim(BuildingAnimSlot::Special).Anim[0]) { //only if it actually has a special anim
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

	CoordStruct SourceXYZ = B->GetFLH(0, CoordStruct::Empty);

	ColorStruct blank(0, 0, 0);

	int idxSupport = -1;
	if (B->Veterancy.IsElite()) {
		idxSupport = pTypeData->PrismForwarding.EliteSupportWeaponIndex;
	} else {
		idxSupport = pTypeData->PrismForwarding.SupportWeaponIndex;
	}

	WeaponTypeClass * supportWeapon = nullptr;
	if (idxSupport != -1) {
		supportWeapon = pType->get_Weapon(idxSupport);
	}
	LaserDrawClass * LaserBeam = nullptr;
	if (supportWeapon) {
		WeaponTypeExt::ExtData *supportWeaponData = WeaponTypeExt::ExtMap.Find(supportWeapon);
		//IsLaser
		if (supportWeapon->IsLaser) {
			if (supportWeapon->IsHouseColor) {
				LaserBeam = GameCreate<LaserDrawClass>(SourceXYZ, *pTargetXYZ, B->Owner->LaserColor, blank, blank, supportWeapon->LaserDuration);
			} else {
				LaserBeam = GameCreate<LaserDrawClass>(SourceXYZ, *pTargetXYZ,
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
		if (supportWeapon->IsRadBeam) {
			RadBeam* supportRadBeam = RadBeam::Allocate(RadBeamType::RadBeam);
			if (supportRadBeam) {
				supportRadBeam->SetCoordsSource(SourceXYZ);
				supportRadBeam->SetCoordsTarget(*pTargetXYZ);
				if (supportWeaponData->Beam_IsHouseColor) {
					supportRadBeam->Color = B->Owner->LaserColor;
				} else {
					supportRadBeam->Color = supportWeaponData->GetBeamColor();
				}
				supportRadBeam->Period = supportWeaponData->Beam_Duration;
				supportRadBeam->Amplitude = supportWeaponData->Beam_Amplitude;
				//Debug::DumpObj((byte *)supportRadBeam, sizeof(RadBeam));
			}
		}
		//IsElectricBolt
		if (supportWeapon->IsElectricBolt) {
			if(auto supportEBolt = WeaponTypeExt::CreateBolt(supportWeaponData)) {
				//supportEBolt->Owner = B;
				TechnoExt::ExtData *pBuildingExt = TechnoExt::ExtMap.Find(B);
				pBuildingExt->MyBolt = supportEBolt;
				supportEBolt->WeaponSlot = idxSupport;
				supportEBolt->AlternateColor = supportWeapon->IsAlternateColor;
				supportEBolt->Fire(SourceXYZ, *pTargetXYZ, 0); //messing with 3rd arg seems to make bolts more jumpy, and parts of them disappear
			}
		}
		//Report
		if(supportWeapon->Report.Count > 0) {
			int ReportIndex = ScenarioClass::Instance->Random.RandomRanged(0, supportWeapon->Report.Count - 1);
			int SoundArrayIndex = supportWeapon->Report.GetItem(ReportIndex);
			if(SoundArrayIndex != -1) {
				VocClass::PlayAt(SoundArrayIndex, SourceXYZ, nullptr);
			}
		}
		//ROF
		B->ReloadTimer.Start(supportWeapon->ROF);
	} else {
		//just the default support beam
		LaserBeam = GameCreate<LaserDrawClass>(SourceXYZ, *pTargetXYZ, B->Owner->LaserColor, blank, blank, RulesClass::Instance->PrismSupportDuration);
		if(LaserBeam) {
			LaserBeam->IsHouseColor = true;
			LaserBeam->Thickness = 3;
		}
		B->ReloadTimer.Start(RulesClass::Instance->PrismSupportDelay);
	}

	//Intensity adjustment for LaserBeam
	if (LaserBeam) {
		if (pTypeData->PrismForwarding.Intensity > 0) {
			//BuildingExt::ExtData *pData = BuildingExt::ExtMap.Find(B);
			LaserBeam->Thickness += (pTypeData->PrismForwarding.Intensity * (B->SupportingPrisms - 1));
		}
	}

	B->SupportingPrisms = 0; //not sure why Westwood set this here. We're setting it in BuildingClass_Update_Prism

	return 0x44ACE2;
}

//these are all for cleaning up when a prism tower becomes unavailable

DEFINE_HOOK(4424EF, PrismForward_BuildingDestroyed, 6)
{
	GET(BuildingClass *, B, ESI);
	auto pData = BuildingExt::ExtMap.Find(B);
	pData->PrismForwarding.RemoveFromNetwork(true);
	return 0;
}

DEFINE_HOOK(447113, BuildingClass_Sell_PrismForward, 6)
{
	GET(BuildingClass *, B, ESI);

	// #754 - evict Hospital/Armory contents
	BuildingExt::KickOutHospitalArmory(B);

	auto pData = BuildingExt::ExtMap.Find(B);
	pData->PrismForwarding.RemoveFromNetwork(true);
	return 0;
}

DEFINE_HOOK(448277, BuildingClass_ChangeOwner_PrismForwardAndLeaveBomb, 5)
{
	GET(BuildingClass*, pThis, ESI);
	GET_STACK(HouseClass*, newOwner, 0x5C);

	enum { LeaveBomb = 0x448293 };

	// #754 - evict Hospital/Armory contents
	BuildingExt::KickOutHospitalArmory(pThis);

	auto oldOwner = pThis->Owner;

	if(newOwner != oldOwner) {
		auto pData = BuildingExt::ExtMap.Find(pThis);
		auto pTypeData = BuildingTypeExt::ExtMap.Find(pThis->Type);

		// the first and the last tower have to be allied to this
		if(pTypeData->PrismForwarding.ToAllies) {
			auto FirstTarget = pData->PrismForwarding.SupportTarget;

			if(!FirstTarget) {
				// no first target so either this is a master tower, an idle
				// tower, or not a prism tower at all no need to remove.
				return LeaveBomb;
			}

			// the tower the new owner strives to support has to be allied, otherwise abort.
			if(newOwner->IsAlliedWith(FirstTarget->GetOwner()->Owner)) {
				auto LastTarget = FirstTarget;
				while(LastTarget->SupportTarget) {
					LastTarget = LastTarget->SupportTarget;
				}

				// LastTarget is now the master (firing) tower
				if(newOwner->IsAlliedWith(LastTarget->GetOwner()->Owner)) {
					// alliances check out so this slave tower can keep on charging.
					return LeaveBomb;
				}
			}
		}

		// if we reach this point then the alliance checks have failed. use false
		// because animation should continue / slave is busy but won't now fire
		pData->PrismForwarding.RemoveFromNetwork(false);
	}

	return LeaveBomb;
}

DEFINE_HOOK(71AF76, TemporalClass_Fire_PrismForwardAndWarpable, 9) {
	GET(TechnoClass *, T, EDI);

	// bugfix #874 B: Temporal warheads affect Warpable=no units
	// it has been checked: this is warpable. free captured and destroy spawned units.
	if(T->SpawnManager) {
		T->SpawnManager->KillNodes();
	}

	if(T->CaptureManager) {
		T->CaptureManager->FreeAll();
	}

	// prism forward
	if (BuildingClass * B = specific_cast<BuildingClass *>(T)) {
		auto pData = BuildingExt::ExtMap.Find(B);
		pData->PrismForwarding.RemoveFromNetwork(true);
	}
	return 0;
}


DEFINE_HOOK(70FD9A, PrismForward_BuildingDrain, 6)
{
	GET(TechnoClass *, Drainer, ESI);
	GET(TechnoClass *, Drainee, EDI);
	if(Drainee->DrainingMe != Drainer) { // else we're already being drained, nothing to do
		if (BuildingClass * B = specific_cast<BuildingClass *>(Drainee)) {
			auto pData = BuildingExt::ExtMap.Find(B);
			pData->PrismForwarding.RemoveFromNetwork(true);
		}
	}
	return 0;
}

DEFINE_HOOK(454B3D, PrismForward_BuildingPowerDown, 6)
{
	GET(BuildingClass *, B, ESI);
	// this building just realised it needs to go offline
	// it unregistered itself from powered unit controls but hasn't done anything else yet
	auto pData = BuildingExt::ExtMap.Find(B);
	pData->PrismForwarding.RemoveFromNetwork(true);
	return 0;
}

DEFINE_HOOK(44EBF0, PrismForward_BuildingRemoved, 5)
{
	GET(BuildingClass *, B, ECX);
	auto pData = BuildingExt::ExtMap.Find(B);
	pData->PrismForwarding.RemoveFromNetwork(true);
	return 0;
}
