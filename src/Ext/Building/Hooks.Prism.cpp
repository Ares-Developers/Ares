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
	GET(BuildingClass* const, pThis, ESI);
	//GET(int, idxWeapon, EBP); //which weapon was chosen to attack the target with
	R->EAX(pThis->Type);

	enum { IsPrism = 0x44B310, IsNotPrism = 0x44B630, IsCustomPrism = 0x44B6D6 };

	auto const pMasterType = pThis->Type;
	auto const pMasterTypeData = BuildingTypeExt::ExtMap.Find(pMasterType);

	if(pMasterTypeData->PrismForwarding.CanAttack()) {
		auto const pMasterData = BuildingExt::ExtMap.Find(pThis);

		if(pThis->PrismStage == PrismChargeState::Idle) {
			pThis->PrismStage = PrismChargeState::Master;
			pThis->DelayBeforeFiring = pThis->Type->DelayedFireDelay;

			pThis->PrismTargetCoords.X = 0;
			if(pMasterType->Weapon[1].WeaponType) {
				if(pThis->IsOverpowered) {
					pThis->PrismTargetCoords.X = 1;
				}
			}

			pThis->PrismTargetCoords.Y = pThis->PrismTargetCoords.Z = 0;
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

		} else if(pThis->PrismStage == PrismChargeState::Slave) {
			//a slave tower is changing into a master tower at the last second
			pThis->PrismStage = PrismChargeState::Master;
			pThis->PrismTargetCoords.X = 0;
			if(pMasterType->Weapon[1].WeaponType) {
				if(pThis->IsOverpowered) {
					pThis->PrismTargetCoords.X = 1;
				}
			}
			pThis->PrismTargetCoords.Y = pThis->PrismTargetCoords.Z = 0;
			pMasterData->PrismForwarding.ModifierReserve = 0.0;
			pMasterData->PrismForwarding.DamageReserve = 0;
			pMasterData->PrismForwarding.SetSupportTarget(nullptr);
		}

		return IsCustomPrism; //always custom, the new code is a complete rewrite of the old code
	}

	return IsNotPrism;
}

DEFINE_HOOK(447FAE, BuildingClass_GetFireError_PrismForward, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	enum { BusyCharging = 0x447FB8, NotBusyCharging = 0x447FC3 };

	if(pThis->DelayBeforeFiring > 0) {
		//if this is a slave prism tower, then it might still be able to become a master tower at this time
		auto const pType = pThis->Type;
		auto const pTypeData = BuildingTypeExt::ExtMap.Find(pType);
		if(pTypeData->PrismForwarding.CanAttack()) {
			//is a prism tower
			if(pThis->PrismStage == PrismChargeState::Slave && pTypeData->PrismForwarding.BreakSupport) {
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
	GET(BuildingClass* const, pThis, ECX);
	auto const pType = pThis->Type;

	auto const PrismStage = pThis->PrismStage;
	if(PrismStage != PrismChargeState::Idle) {
		auto const pData = BuildingExt::ExtMap.Find(pThis);
		if(pData->PrismForwarding.PrismChargeDelay <= 0) {
			--pThis->DelayBeforeFiring;
			if(pThis->DelayBeforeFiring <= 0) {
				if(PrismStage == PrismChargeState::Slave) {
					if(auto pTarget = pData->PrismForwarding.SupportTarget) {
						auto const pTargetData = pTarget->Owner;
						auto const pTypeData = BuildingTypeExt::ExtMap.Find(pType);
						//slave firing
						pTargetData->PrismForwarding.ModifierReserve +=
							(pTypeData->PrismForwarding.GetSupportModifier() + pData->PrismForwarding.ModifierReserve);
						pTargetData->PrismForwarding.DamageReserve +=
							(pTypeData->PrismForwarding.DamageAdd + pData->PrismForwarding.DamageReserve);
						pThis->FireLaser(pThis->PrismTargetCoords);
					}
				}
				if(PrismStage == PrismChargeState::Master) {
					if(auto const Target = pThis->Target) {
						if(pThis->GetFireError(Target, pThis->PrismTargetCoords.X, true) == FireError::OK) {
							if(auto const LaserBeam = pThis->Fire(Target, pThis->PrismTargetCoords.X)) {
								auto const pTypeData = BuildingTypeExt::ExtMap.Find(pType);

								//apparently this is divided by 256 elsewhere
								LaserBeam->DamageMultiplier = static_cast<int>((pData->PrismForwarding.ModifierReserve + 100) * 256) / 100;
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
			if(pData->PrismForwarding.PrismChargeDelay <= 0) {
				//now it's time to start charging
				if(pType->GetBuildingAnim(BuildingAnimSlot::Special).Anim[0]) { //only if it actually has a special anim
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
	GET(BuildingClass* const, pThis, ECX);
	REF_STACK(CoordStruct const, targetXYZ, 0x4);

	auto const pType = pThis->Type;
	auto const pTypeData = BuildingTypeExt::ExtMap.Find(pType);

	auto const sourceXYZ = pThis->GetFLH(0, CoordStruct::Empty);

	ColorStruct const blank(0, 0, 0);

	int idxSupport = -1;
	if(pThis->Veterancy.IsElite()) {
		idxSupport = pTypeData->PrismForwarding.EliteSupportWeaponIndex;
	} else {
		idxSupport = pTypeData->PrismForwarding.SupportWeaponIndex;
	}

	auto const supportWeapon = (idxSupport != -1)
		? pType->Weapon[idxSupport].WeaponType : nullptr;

	LaserDrawClass* LaserBeam = nullptr;
	if(supportWeapon) {
		auto const supportWeaponData = WeaponTypeExt::ExtMap.Find(supportWeapon);
		//IsLaser
		if(supportWeapon->IsLaser) {
			if(supportWeapon->IsHouseColor) {
				LaserBeam = GameCreate<LaserDrawClass>(sourceXYZ, targetXYZ, pThis->Owner->LaserColor, blank, blank, supportWeapon->LaserDuration);
			} else {
				LaserBeam = GameCreate<LaserDrawClass>(sourceXYZ, targetXYZ,
					supportWeapon->LaserInnerColor, supportWeapon->LaserOuterColor, supportWeapon->LaserOuterSpread,
					supportWeapon->LaserDuration);
			}
			if(LaserBeam) {
				LaserBeam->IsHouseColor = supportWeapon->IsHouseColor;
				//basic thickness (intensity additions are added later)
				if(supportWeaponData->Laser_Thickness == -1) {
					LaserBeam->Thickness = 3; //original default
				} else {
					LaserBeam->Thickness = supportWeaponData->Laser_Thickness;
				}
			}
		}
		//IsRadBeam
		if(supportWeapon->IsRadBeam) {
			if(auto const supportRadBeam = RadBeam::Allocate(RadBeamType::RadBeam)) {
				supportRadBeam->SetCoordsSource(sourceXYZ);
				supportRadBeam->SetCoordsTarget(targetXYZ);
				if(supportWeaponData->Beam_IsHouseColor) {
					supportRadBeam->Color = pThis->Owner->LaserColor;
				} else {
					supportRadBeam->Color = supportWeaponData->GetBeamColor();
				}
				supportRadBeam->Period = supportWeaponData->Beam_Duration;
				supportRadBeam->Amplitude = supportWeaponData->Beam_Amplitude;
				//Debug::DumpObj((byte *)supportRadBeam, sizeof(RadBeam));
			}
		}
		//IsElectricBolt
		if(supportWeapon->IsElectricBolt) {
			if(auto const supportEBolt = WeaponTypeExt::CreateBolt(supportWeaponData)) {
				//supportEBolt->Owner = pThis;
				auto const pBuildingExt = TechnoExt::ExtMap.Find(pThis);
				pBuildingExt->MyBolt = supportEBolt;
				supportEBolt->WeaponSlot = idxSupport;
				supportEBolt->AlternateColor = supportWeapon->IsAlternateColor;
				supportEBolt->Fire(sourceXYZ, targetXYZ, 0); //messing with 3rd arg seems to make bolts more jumpy, and parts of them disappear
			}
		}
		//Report
		if(supportWeapon->Report.Count > 0) {
			auto const ReportIndex = ScenarioClass::Instance->Random.RandomRanged(0, supportWeapon->Report.Count - 1);
			auto const SoundArrayIndex = supportWeapon->Report.GetItem(ReportIndex);
			if(SoundArrayIndex != -1) {
				VocClass::PlayAt(SoundArrayIndex, sourceXYZ, nullptr);
			}
		}
		//ROF
		pThis->ReloadTimer.Start(supportWeapon->ROF);
	} else {
		//just the default support beam
		LaserBeam = GameCreate<LaserDrawClass>(sourceXYZ, targetXYZ, pThis->Owner->LaserColor, blank, blank, RulesClass::Instance->PrismSupportDuration);
		if(LaserBeam) {
			LaserBeam->IsHouseColor = true;
			LaserBeam->Thickness = 3;
		}
		pThis->ReloadTimer.Start(RulesClass::Instance->PrismSupportDelay);
	}

	//Intensity adjustment for LaserBeam
	if(LaserBeam) {
		if(pThis->SupportingPrisms) {
			if(pTypeData->PrismForwarding.Intensity < 0) {
				LaserBeam->Thickness -= pTypeData->PrismForwarding.Intensity; //add on absolute intensity
			} else if(pTypeData->PrismForwarding.Intensity > 0) {
				LaserBeam->Thickness += (pTypeData->PrismForwarding.Intensity * pThis->SupportingPrisms);
			}

			LaserBeam->IsSupported = (LaserBeam->Thickness > 5);
		}
	}

	pThis->SupportingPrisms = 0; //not sure why Westwood set this here. We're setting it in BuildingClass_Update_Prism

	return 0x44ACE2;
}

//these are all for cleaning up when a prism tower becomes unavailable

DEFINE_HOOK(4424EF, BuildingClass_ReceiveDamage_PrismForward, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	auto const pData = BuildingExt::ExtMap.Find(pThis);
	pData->PrismForwarding.RemoveFromNetwork(true);
	return 0;
}

DEFINE_HOOK(447113, BuildingClass_Sell_PrismForward, 6)
{
	GET(BuildingClass* const, pThis, ESI);

	// #754 - evict Hospital/Armory contents
	BuildingExt::KickOutHospitalArmory(pThis);

	auto const pData = BuildingExt::ExtMap.Find(pThis);
	pData->PrismForwarding.RemoveFromNetwork(true);
	return 0;
}

DEFINE_HOOK(448277, BuildingClass_ChangeOwner_PrismForwardAndLeaveBomb, 5)
{
	GET(BuildingClass* const, pThis, ESI);
	GET_STACK(HouseClass* const, newOwner, 0x5C);

	enum { LeaveBomb = 0x448293 };

	// #754 - evict Hospital/Armory contents
	BuildingExt::KickOutHospitalArmory(pThis);

	auto const pData = BuildingExt::ExtMap.Find(pThis);
	auto const pTypeData = BuildingTypeExt::ExtMap.Find(pThis->Type);

	// the first and the last tower have to be allied to this
	if(pTypeData->PrismForwarding.ToAllies) {
		auto const FirstTarget = pData->PrismForwarding.SupportTarget;

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

	// #305: remove all jammers. will be restored with the next update.
	pData->RegisteredJammers.clear();

	return LeaveBomb;
}

DEFINE_HOOK(71AF76, TemporalClass_Fire_PrismForwardAndWarpable, 9) {
	GET(TechnoClass* const, pThis, EDI);

	// bugfix #874 B: Temporal warheads affect Warpable=no units
	// it has been checked: this is warpable. free captured and destroy spawned units.
	if(pThis->SpawnManager) {
		pThis->SpawnManager->KillNodes();
	}

	if(pThis->CaptureManager) {
		pThis->CaptureManager->FreeAll();
	}

	// prism forward
	if(auto const pBld = abstract_cast<BuildingClass*>(pThis)) {
		auto const pData = BuildingExt::ExtMap.Find(pBld);
		pData->PrismForwarding.RemoveFromNetwork(true);
	}
	return 0;
}

DEFINE_HOOK(70FD9A, TechnoClass_Drain_PrismForward, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(TechnoClass* const, pDrainee, EDI);
	if(pDrainee->DrainingMe != pThis) { // else we're already being drained, nothing to do
		if(auto const pBld = abstract_cast<BuildingClass*>(pDrainee)) {
			auto const pData = BuildingExt::ExtMap.Find(pBld);
			pData->PrismForwarding.RemoveFromNetwork(true);
		}
	}
	return 0;
}

DEFINE_HOOK(454B3D, BuildingClass_UpdatePowered_PrismForward, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	// this building just realised it needs to go offline
	// it unregistered itself from powered unit controls but hasn't done anything else yet
	auto const pData = BuildingExt::ExtMap.Find(pThis);
	pData->PrismForwarding.RemoveFromNetwork(true);
	return 0;
}

DEFINE_HOOK(44EBF0, BuildingClass_Disappear_PrismForward, 5)
{
	GET(BuildingClass* const, pThis, ECX);
	auto const pData = BuildingExt::ExtMap.Find(pThis);
	pData->PrismForwarding.RemoveFromNetwork(true);
	return 0;
}
