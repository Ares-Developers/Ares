#include "Body.h"
#include "../Rules/Body.h"
#include "../TechnoType/Body.h"
#include "../Building/Body.h"
#include "../BuildingType/Body.h"
#include "../Rules/Body.h"
#include "../Tiberium/Body.h"
#include "../WarheadType/Body.h"
#include "../WeaponType/Body.h"
#include "../../Misc/Debug.h"
#include "../../Misc/JammerClass.h"
#include "../../Misc/PoweredUnitClass.h"

#include <AircraftClass.h>
#include <GameOptionsClass.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <SpecificStructures.h>
#include <TiberiumClass.h>
#include <UnitClass.h>

#include <algorithm>

// bugfix #297: Crewed=yes jumpjets spawn parachuted infantry on destruction, not idle
DEFINE_HOOK(737F97, UnitClass_ReceiveDamage, 0)
{
	GET(UnitClass *, t, ESI);
	GET_STACK(TechnoClass *, Killer, 0x54);
	GET_STACK(bool, select, 0x13);
	GET_STACK(bool, ignoreDefenses, 0x58);
	TechnoExt::SpawnSurvivors(t, Killer, select, ignoreDefenses);

	R->EBX(-1); // #1489302 trucks and crates
	return 0x73838A;
}

// bugfix #297: Crewed=yes AircraftTypes spawn parachuting infantry on death
DEFINE_HOOK(41668B, AircraftClass_ReceiveDamage, 6)
{
	GET(AircraftClass *, a, ESI);
	GET_STACK(TechnoClass *, Killer, 0x28);
	GET_STACK(int, ignoreDefenses, 0x20);
	bool select = a->IsSelected && a->Owner->ControlledByPlayer();
	TechnoExt::SpawnSurvivors(a, Killer, select, ignoreDefenses != 0);

	// Crashable support for aircraft
	if(auto pExt = TechnoTypeExt::ExtMap.Find(a->GetTechnoType())) {
		if(!pExt->Crashable.Get(true)) {
			R->EAX(0);
			return 0x41669A;
		}
	}

	return 0;
}

// rotation when crashing made optional
DEFINE_HOOK(4DECAE, FootClass_Crash_Spin, 5)
{
	GET(FootClass*, pThis, ESI);
	auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	return pExt->CrashSpin ? 0u : 0x4DED4Bu;
}

// move to the next hva frame, even if this unit isn't moving
DEFINE_HOOK(4DA8B2, FootClass_Update_AnimRate, 6)
{
	GET(FootClass*, pThis, ESI);
	auto pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	enum { Undecided = 0u, NoChange = 0x4DAA01u, Advance = 0x4DA9FBu };

	// any of these prevents the animation to advance to the next frame
	if(pThis->IsBeingWarpedOut() || pThis->IsWarpingIn() || pThis->IsAttackedByLocomotor) {
		return NoChange;
	}

	// animate unit whenever in air
	if(pExt->AirRate && pThis->GetHeight() > 0) {
		return (Unsorted::CurrentFrame % pExt->AirRate) ? NoChange : Advance;
	}

	return Undecided;
}

DEFINE_HOOK(6F9E50, TechnoClass_Update, 5)
{
	GET(TechnoClass* const, pThis, ECX);

	auto const pType = pThis->GetTechnoType();
	auto const pData = TechnoExt::ExtMap.Find(pThis);
	auto const pTypeData = TechnoTypeExt::ExtMap.Find(pType);

	// #1208
	if(pTypeData->PassengerTurret) {
		// 18 = 1 8 = A H = Adolf Hitler. Clearly we can't allow it to come to that.
		auto const passengerNumber = Math::min(pThis->Passengers.NumPassengers, 17);
		pThis->CurrentTurretNumber = Math::min(passengerNumber, pType->TurretCount - 1);
	}

	// #617 powered units
	if(!pTypeData->PoweredBy.empty()) {
		if(!pData->PoweredUnit) {
			pData->PoweredUnit = std::make_unique<PoweredUnitClass>(pThis);
		}
		if(!pData->PoweredUnit->Update()) {
			TechnoExt::Destroy(pThis);
		}
	}

	AttachEffectClass::Update(pThis);

	return 0;
}

//! TechnoClass::Update is called every frame; returning 0 tells it to execute the original function's code as well.
//! EXCEPT if the target is under Temporal, use the 71A860 hook for that - Graion, 2013-06-13.
DEFINE_HOOK(6F9E76, TechnoClass_Update_CheckOperators, 6)
{
	GET(TechnoClass* const, pThis, ESI); // object this is called on

	auto const pData = TechnoExt::ExtMap.Find(pThis);

	// Related to operators/drivers, issue #342
	auto const pBuildingBelow = pThis->GetCell()->GetBuilding();
	auto const buildingBelowIsMe = pThis == pBuildingBelow;

	/* Conditions checked:
		pBuildingBelow will be NULL if no building was found
		This check ensures that Operator'd units don't Deactivate above structures such as War Factories, Repair Depots or Battle Bunkers.
		(Which is potentially abusable, but let's hope no one figures that out.)
	*/
	if(!pBuildingBelow || (buildingBelowIsMe && pBuildingBelow->IsPowerOnline())) {
		bool Override = false;
		if(auto const pFoot = abstract_cast<FootClass*>(pThis)) {
			if(!pBuildingBelow) {
				// immobile, though not disabled. like hover tanks after
				// a repair depot has been sold or warped away.
				Override = (pFoot->Locomotor->Is_Powered() == pThis->Deactivated);
			}
		}

		if(pData->IsOperated()) { // either does have an operator or doesn't need one, so...
			if(Override || (pThis->Deactivated && !pThis->IsUnderEMP() && pData->IsPowered())) { // ...if it's currently off, turn it on! (oooh baby)
				pThis->Reactivate();
				if(buildingBelowIsMe) {
					pThis->Owner->RecheckTechTree = true; // #885
				}
			}
		} else { // doesn't have an operator, so...
			if(!pThis->Deactivated) { // ...if it's not off yet, turn it off!
				pThis->Deactivate();
				if(buildingBelowIsMe) {
					pThis->Owner->RecheckTechTree = true; // #885
				}
			}
		}
	}

	// prevent disabled units from driving around.
	if(pThis->Deactivated) {
		if(auto const pUnit = abstract_cast<UnitClass*>(pThis)) {
			if(pUnit->Locomotor->Is_Moving() && pUnit->Destination && !pThis->LocomotorSource) {
				pUnit->SetDestination(nullptr, true);
				pUnit->StopMoving();
			}
		}

		// dropping Radar Jammers (#305) here for now; should check if another TechnoClass::Update hook might be better ~Ren
		if(pData->RadarJam) { // RadarJam should only be non-null if the object is an active radar jammer
			pData->RadarJam->UnjamAll();
		}
	} else {
		// dropping Radar Jammers (#305) here for now; should check if another TechnoClass::Update hook might be better ~Ren
		auto const pType = pThis->GetTechnoType();
		auto const pTypeData = TechnoTypeExt::ExtMap.Find(pType);

		if(pTypeData->RadarJamRadius) {
			if(!pData->RadarJam) {
				pData->RadarJam = std::make_unique<JammerClass>(pThis);
			}

			pData->RadarJam->Update();
		}
	}

	/* 	using 0x6F9E7C instead makes this function override the original game one's entirely -
		don't activate that unless you handle _everything_ originally handled by the game */
	return 0;
}

// Radar Jammers (#305) unjam all on owner change
DEFINE_HOOK(7014D5, TechnoClass_ChangeOwnership_RadarJammer, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if(auto const& pJammer = pExt->RadarJam) {
		pJammer->UnjamAll();
	}

	return 0;
}

// fix for vehicle paradrop alignment
DEFINE_HOOK(415CA6, AircraftClass_Paradrop, 6)
{
	GET(AircraftClass *, A, EDI);
	GET(FootClass *, P, ESI);
	if(P->WhatAmI() != AbstractType::Unit) {
		return 0;
	}
	CoordStruct SrcXYZ = A->GetCoords();
	LEA_STACK(CoordStruct *, XYZ, 0x20);
	XYZ->X = (SrcXYZ.X & ~0xFF) + 0x80;
	XYZ->Y = (SrcXYZ.Y & ~0xFF) + 0x80;
	XYZ->Z = SrcXYZ.Z - 1;
	R->ECX<CoordStruct *>(XYZ);
	return 0x415DE3;
}

// #1232: fix for dropping units out of flying Carryalls
DEFINE_HOOK(415DF6, AircraftClass_Paradrop_Carryall, 6)
{
	GET(FootClass *, pTechno, ESI);
	pTechno->IsOnCarryall = false;
	return 0;
}

DEFINE_HOOK(6F407D, TechnoClass_Init_1, 6)
{
	GET(TechnoClass* const, pThis, ESI);
	auto const pType = pThis->GetTechnoType();
	auto const pData = TechnoExt::ExtMap.Find(pThis);

	CaptureManagerClass* pCapturer = nullptr;
	ParasiteClass* pParasite = nullptr;
	TemporalClass* pTemporal = nullptr;

	auto const pFoot = abstract_cast<FootClass*>(pThis);

	auto const CheckWeapon = [=, &pCapturer, &pParasite, &pTemporal](
		WeaponTypeClass* pWeapon, int idxWeapon, const char* pTagName)
	{
		constexpr auto const Note = "Constructing an instance of [%s]:\r\n"
			"%s %s (slot %d) has no %s!";

		if(!pWeapon->Projectile) {
			Debug::FatalErrorAndExit(
				Note, pType->ID, pTagName, pWeapon->ID, idxWeapon,
				"Projectile");
		}

		auto const pWarhead = pWeapon->Warhead;

		if(!pWarhead) {
			Debug::FatalErrorAndExit(
				Note, pType->ID, pTagName, pWeapon->ID, idxWeapon, "Warhead");
		}

		if(pWarhead->MindControl && !pCapturer) {
			pCapturer = GameCreate<CaptureManagerClass>(
				pThis, pWeapon->Damage, pWeapon->InfiniteMindControl);
		}

		if(pWarhead->Temporal && !pTemporal) {
			pTemporal = GameCreate<TemporalClass>(pThis);
			pTemporal->WarpPerStep = pWeapon->Damage;
			pData->idxSlot_Warp = static_cast<BYTE>(idxWeapon);
		}

		if(pWarhead->Parasite && pFoot && !pParasite) {
			pParasite = GameCreate<ParasiteClass>(pFoot);
			pData->idxSlot_Parasite = static_cast<BYTE>(idxWeapon);
		}
	};

	// iterate all weapons and their elite counterparts
	for(auto i = 0; i < TechnoTypeClass::MaxWeapons; ++i) {
		if(auto const pWeapon = pType->Weapon[i].WeaponType) {
			CheckWeapon(pWeapon, i, "Weapon");
		}
		if(auto const pWeapon = pType->EliteWeapon[i].WeaponType) {
			CheckWeapon(pWeapon, i, "EliteWeapon");
		}
	}

	pThis->CaptureManager = pCapturer;
	pThis->TemporalImUsing = pTemporal;
	if(pFoot) {
		pFoot->ParasiteImUsing = pParasite;
	}

	auto const pHouseType = pThis->Owner->Type;
	pData->OriginalHouseType = pHouseType->FindParentCountry();
	if(!pData->OriginalHouseType) {
		pData->OriginalHouseType = pHouseType;
	}

	// if override is in effect, do not create initial payload.
	// this object might have been deployed, undeployed, ...
	if(Unsorted::IKnowWhatImDoing && Unsorted::CurrentFrame) {
		pData->PayloadCreated = true;
	}

	return 0x6F4102;
}

DEFINE_HOOK(6F4103, TechnoClass_Init_2, 6)
{
	return 0x6F41C0;
}

DEFINE_HOOK(446EE2, BuildingClass_Place_InitialPayload, 6)
{
	GET(BuildingClass* const, pThis, EBP);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->CreateInitialPayload();

	return 0;
}

DEFINE_HOOK(4D718C, FootClass_Put_InitialPayload, 6)
{
	GET(FootClass* const, pThis, ESI);

	if(pThis->WhatAmI() != AbstractType::Infantry) {
		auto const pExt = TechnoExt::ExtMap.Find(pThis);
		pExt->CreateInitialPayload();
	}

	return 0;
}

// temporal per-slot
DEFINE_HOOK(71A84E, TemporalClass_UpdateA, 5)
{
	GET(TemporalClass* const, pThis, ESI);

	// it's not guaranteed that there is a target
	if(auto const pTarget = pThis->Target) {
		auto const pExt = TechnoExt::ExtMap.Find(pTarget);
		// Temporal should disable RadarJammers
		pExt->RadarJam = nullptr;

		//AttachEffect handling under Temporal
		if(!pExt->AttachEffects_RecreateAnims) {
			for(auto& Item : pExt->AttachedEffects) {
				if(Item.Type->TemporalHidesAnim) {
					Item.KillAnim();
				}
			}
			pExt->AttachEffects_RecreateAnims = true;
		}
	}

	pThis->WarpRemaining -= pThis->GetWarpPerStep(0);

	R->EAX(pThis->WarpRemaining);
	return 0x71A88D;
}

// temporal per-slot
DEFINE_HOOK(71AB30, TemporalClass_GetHelperDamage, 5)
{
	GET(TemporalClass *, Temp, ESI);
	TechnoClass *T = Temp->Owner;
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(T);
	WeaponStruct *W = T->GetWeapon(pData->idxSlot_Warp);
	WarheadTypeExt::Temporal_WH = W->WeaponType->Warhead;
	R->EAX<WeaponStruct *>(W);
	return 0x71AB47;
}

// parasite per-slot
DEFINE_HOOK(62A020, ParasiteClass_Update, A)
{
	GET(TechnoClass *, T, ECX);
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(T);
	WeaponStruct *W = T->GetWeapon(pData->idxSlot_Parasite);
	R->EAX<WeaponStruct *>(W);
	return 0x62A02A;
}

DEFINE_HOOK(62A7B1, Parasite_ExitUnit, 9)
{
	GET(TechnoClass *, T, ECX);
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(T);
	WeaponStruct *W = T->GetWeapon(pData->idxSlot_Parasite);
	R->EAX<WeaponStruct *>(W);
	return 0x62A7BA;
}

DEFINE_HOOK(629804, ParasiteClass_UpdateSquiddy, 9)
{
	GET(TechnoClass *, T, ECX);
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(T);
	WeaponStruct *W = T->GetWeapon(pData->idxSlot_Parasite);
	R->EAX<WeaponStruct *>(W);
	return 0x62980D;
}

/*
DEFINE_HOOK(6F3330, TechnoClass_SelectWeapon, 5)
{
	GET(TechnoClass *, pThis, ECX);
	GET_STACK(TechnoClass *, pTarg, 0x4);

	DWORD Selected = TechnoClassExt::SelectWeaponAgainst(pThis, pTarg);
	R->EAX(Selected);
	return 0x6F3813;
}

int TechnoClassExt::SelectWeaponAgainst(TechnoClass *pThis, TechnoClass *pTarget)
{
	int Index = 0;

	WeaponStruct* W1 = pThis->GetWeapon(0);
	WeaponTypeClass* W1T = W1->WeaponType;
	WeaponStruct* W2 = pThis->GetWeapon(1);
	WeaponTypeClass* W2T = W2->WeaponType;

	TechnoTypeClass *pThisT = pThis->GetTechnoType();
//	TechnoTypeClass *pTargetT = pTarget->GetTechnoType();

	if(pThisT->HasMultipleTurrets() && !pThisT->get_IsGattling()) {
		return pThis->get_CurrentTurret();
	}

	if(pThis->CanOccupyFire()) {
		return 0;
	}

	if(pThis->get_InOpenToppedTransport()) {
		Index = pThisT->get_OpenTransportWeapon();
		if(Index != -1) {
			return Index;
		}
	}

	if(pThisT->get_IsGattling()) {
		int CurrentStage = pThis->get_CurrentGattlingStage() * 2;
		if(pTarget->get_AbstractFlags() & ABSFLAGS_ISTECHNO && pTarget->IsInAir()) {
			if(W2T && W2T->get_Projectile()->get_AA()) {
				return CurrentStage + 1;
			}
		}
		return CurrentStage;
	}

	if(pThis->WhatAmI() == abs_Building && ((BuildingClass *)pThis)->get_IsOverpowered()) {
		return 1;
	}

	if(pTarget->WhatAmI() == abs_Aircraft && ((AircraftClass *)pTarget)->get_IsCrashing()) {
		return 1;
	}

	// haaaaaaaate
	if(pTarget->WhatAmI() == abs_Cell) {
		CellClass *pTargetCell = (CellClass *)pTarget;
		if(

			(pTargetCell->get_LandType() != lt_Water && pTargetCell->IsOnFloor())
			|| ((pTargetCell->get_Flags() & cf_Bridge) && pThisT->get_Naval())

			&& (!pTargetCell->IsInAir() && pThisT->get_LandTargeting() == 2)

		)
		{
			return 1;
		}
	}

	eLandType ltTgt = pTarget->GetCell()->get_LandType();
	bool InWater = !pTarget->get_OnBridge() && !pTarget->IsInAir() && (ltTgt == 2 || ltTgt == 6);

	if(InWater) {
		Index = pThis->SelectNavalTargeting(pTarget);
		if(Index != -1) {
			return Index;
		} else {
			return 0; // what?
		}
	}

	if(!pTarget->IsInAir() && pThisT->get_LandTargeting() == 2) {
		return 1;
	}

	int WCount = pThisT->get_WeaponCount();
	if(WCount < 1) {
		return 0;
	}

	std::vector<WeaponTypeClassExt::WeaponWeight> Weights(WCount);
//	Weights.reserve(WCount);

	for(short i = 0; i < WCount; ++i) {
		WeaponTypeClass* W = pThis->GetWeapon(Index)->WeaponType;
		Weights[i].index = i;
		if(W) {
			CoordStruct xyz1 = *pThis->get_Location();
			CoordStruct xyz2 = *pTarget->get_Location();
			float distance = abs(xyz1.DistanceFrom(xyz2));
			bool CloseEnough = distance <= W->get_Range() && distance >= W->get_MinimumRange();
			Weights[i].DPF = TechnoClassExt::EvalVersesAgainst(pThis, pTarget, W);
			Weights[i].InRange = CloseEnough;
		} else {
			Weights[i].DPF = 0.0;
			Weights[i].InRange = 0;
		}
	}
	std::stable_sort(Weights.begin(), Weights.end());
	std::reverse(Weights.begin(), Weights.end());
	return Weights[0].index;
}

float TechnoClassExt::EvalVersesAgainst(TechnoClass *pThis, TechnoClass *pTarget, WeaponTypeClass* W)
{
	WarheadTypeClass *WH = W->get_Warhead();
	WarheadTypeClassExt::WarheadTypeClassData *pData = WarheadTypeClassExt::Ext_p[WH];
	float Verses = pData->Verses[pTarget->GetType()->get_Armor()];
	return W->get_Damage() * Verses / W->get_ROF();
}

bool TechnoClassExt::EvalWeaponAgainst(TechnoClass *pThis, TechnoClass *pTarget, WeaponTypeClass* W)
{
	if(!W || W->get_NeverUse()) { return 0; }

	WarheadTypeClass *WH = W->get_Warhead();
	if(!WH) { return 0; }

//	TechnoTypeClass *pThisT = pThis->GetTechnoType();
	TechnoTypeClass *pTargetT = pTarget->GetTechnoType();

	if(WH->get_Airstrike()) {
		if(pTarget->WhatAmI() != abs_Building) {
			return 0;
		}
		BuildingTypeClass *pBT = ((BuildingClass *)pTarget)->get_Type();
		// not my design, leave me alone
		return pBT->get_CanC4() && (!pBT->get_ResourceDestination() || !pBT->get_ResourceGatherer());
	}

	if(WH->get_IsLocomotor()) {
		return (pTarget->get_AbstractFlags() & ABSFLAGS_ISFOOT) != 0;
	}

	if(W->get_DrainWeapon()) {
		return pTargetT->get_Drainable() && !pThis->get_DrainTarget() && !pThis->get_Owner()->IsAlliedWith(pTarget);
	}

	if(W->get_AreaFire()) {
		return pThis->GetCurrentMission() == mission_Unload;
	}

	if(pTarget->WhatAmI() == abs_Building && ((BuildingClass *)pTarget)->get_Type()->get_Overpowerable()) {
		return WH->get_ElectricAssault() && pThis->get_Owner()->CanOverpower(pTarget);
	}

	if(pTarget->IsInAir() && !W->get_Projectile()->get_AA()) {
		return 0;
	}

	if(pTarget->IsOnFloor() && !W->get_Projectile()->get_AG()) {
		return 0;
	}

	return 1;
}
*/

DEFINE_HOOK(51F76D, InfantryClass_Unload, 5)
{
	GET(TechnoClass *, I, ESI);
	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(I->GetTechnoType());
	return pData->Is_Deso ? 0x51F77Du : 0x51F792u;
}

DEFINE_HOOK(51CE9A, InfantryClass_Idle, 5)
{
	GET(InfantryClass *, I, ESI);
	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(I->GetTechnoType());

	// don't play idle when paralyzed
	if(I->IsUnderEMP()) {
		R->BL(false);
		return 0x51CECD;
	}

	R->EDI(R->EAX()); // argh
	R->BL(pData->Is_Cow); // aaaargh! again!
	return pData->Is_Cow ? 0x51CEAEu : 0x51CECDu;
}

DEFINE_HOOK(747BBD, UnitTypeClass_LoadFromINI, 5)
{
	GET(UnitTypeClass *, U, ESI);

	U->AltImage = R->EAX<SHPStruct *>(); // jumping over, so replicated
	return U->Gunner
		? 0x747BD7u
		: 0x747E90u;
}

// godawful hack - Desolator deploy fire is triggered by ImmuneToRadiation !
DEFINE_HOOK(5215F9, InfantryClass_UpdateDeploy, 6)
{
	GET(TechnoClass *, I, ESI);
	return TechnoTypeExt::ExtMap.Find(I->GetTechnoType())->Is_Deso
		? 0x5216B6u
		: 0x52160Du;
}

// 52138C, 6
// godawful hack 2 - Desolator deploy fire is triggered by ImmuneToRadiation !
// DON'T USE
EXPORT_FUNC(InfantryClass_UpdateDeploy2)
{
/*
	GET(TechnoClass *, I, ESI);
	TechnoTypeClassExt::TechnoTypeClassData *pData = TechnoTypeClassExt::Ext_p[I->GetTechnoType()];
	return pData->Is_Deso_Radiation ? 0x52139A : 0x5214B9;
	WRONG: needs more code to reimplement weapon shooting without rad checks
*/
	return 0;
}

// stops movement sound from being played while unit is being pulled by a magnetron (see terror drone)
DEFINE_HOOK(7101CF, FootClass_ImbueLocomotor, 7)
{
	GET(FootClass *, F, ESI);
	F->Audio7.ShutUp();
	return 0;
}

DEFINE_HOOK(4DAA68, FootClass_Update_MoveSound, 6)
{
	GET(FootClass *, F, ESI);
	if(F->unknown_bool_53C) {
		return 0x4DAAEE;
	}
	if(F->LocomotorSource) {
		F->Audio7.ShutUp();
		return 0x4DAAEE;
	}
	return 0x4DAA70;
}

/* #397 - AffectsEnemies */
DEFINE_HOOK(701C97, TechnoClass_ReceiveDamage_AffectsEnemies, 6)
{
	GET(WarheadTypeClass *, pThis, EBP);
	GET(TechnoClass *, Victim, ESI);
	LEA_STACK(args_ReceiveDamage *, Arguments, 0xC8);

	// get the owner of the attacker. if there's none, use source house
	auto pSourceHouse = Arguments->Attacker ? Arguments->Attacker->Owner : Arguments->SourceHouse;

	// default for ownerless damage i.e. crates/fire particles
	bool CanAffect = true;

	// check if allied to target, then apply either AffectsAllies or AffectsEnemies
	if(pSourceHouse) {
		auto pExt = WarheadTypeExt::ExtMap.Find(pThis);
		CanAffect = Victim->Owner->IsAlliedWith(pSourceHouse) ? pThis->AffectsAllies : pExt->AffectsEnemies;

		/* Ren, 08.01.2011:
			<not applicable any more \>
			
			The question of how this works came up because the current treatment of Neutral is technically wrong:
			Semantically, AffectsEnemies=no only means "you cannot attack enemies", but our code renders it as "you cannot attack non-allies";
			this obviously means that AffectsEnemies=no includes being unable to attack Neutral, despite the fact that Neutral is, well, neutral - not our enemy.
			
			In the specific situation this came up in, the current behavior was desired and no one else complained so far,
			so I'm not proposing a change at this point. 
			In fact, since this flag is AffectsAllies's evil twin, it is very most likely that practically *all* users of 
			AffectsEnemies assume this behavior; he who sets AffectsEnemies=no likely does so with the intention of limiting damage to allied troops.
			I just wanted this behavior and the logic behind it to be documented for the future.
			
			Note that, in the specific case of AffectsEnemies=no, AffectsAllies=no, this will rear its ugly head as a bug: Neutral should be affected, but won't be.
			*/

		/* AlexB, 2013-08-19:
			"You're either with us, or against us" -- Old saying in Tennessee, ... or was it Texas?

			The game has no clear concept of neutrality. If something like that is going to be added, it could be
			in the form of a Nullable<bool> AffectsNeutral, and a CanAffect = AffectsNeutral.Get(CanAffect) if the
			source house is neutral (however this is going to be inferred).
		*/
	}

	return CanAffect ? 0x701CD7u : 0x701CC2u;
}

// select the most appropriate firing voice and also account
// for undefined flags, so you actually won't lose functionality
// when a unit becomes elite.
DEFINE_HOOK(7090A8, TechnoClass_SelectFiringVoice, 0) {
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, ECX);

	TechnoTypeClass* pType = pThis->GetTechnoType();
	TechnoTypeExt::ExtData* pData = TechnoTypeExt::ExtMap.Find(pType);

	int idxVoice = -1;

	int idxWeapon = pThis->SelectWeapon(pTarget);
	WeaponTypeClass* pWeapon = pThis->GetWeapon(idxWeapon)->WeaponType;

	// repair
	if(pWeapon && pWeapon->Damage < 0) {
		idxVoice = pData->VoiceRepair;
		if(idxVoice < 0) {
			if(!_strcmpi(pType->ID, "FV")) {
				idxVoice = RulesClass::Instance->VoiceIFVRepair;
			}
		}
	}

	// don't mix them up, but fall back to rookie voice if there
	// is no elite voice.
	if(idxVoice < 0) {
		if(idxWeapon) {
			// secondary	
			if(pThis->Veterancy.IsElite()) {
				idxVoice = pType->VoiceSecondaryEliteWeaponAttack;
			}

			if(idxVoice < 0) {
				idxVoice = pType->VoiceSecondaryWeaponAttack;
			}
		} else {
			// primary
			if(pThis->Veterancy.IsElite()) {
				idxVoice = pType->VoicePrimaryEliteWeaponAttack;
			}

			if(idxVoice < 0) {
				idxVoice = pType->VoicePrimaryWeaponAttack;
			}
		}
	}

	// generic attack voice
	if(idxVoice < 0 && pType->VoiceAttack.Count) {
		unsigned int idxRandom = Randomizer::Global()->Random();
		idxVoice = pType->VoiceAttack.GetItem(idxRandom % pType->VoiceAttack.Count);
	}

	// play voice
	if(idxVoice > -1) {
		pThis->QueueVoice(idxVoice);
	}

	return 0x7091C5;
}

// Support per unit modification of Iron Curtain effect duration
DEFINE_HOOK(70E2B0, TechnoClass_IronCurtain, 5) {
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, duration, STACK_OFFS(0x0, -0x4));
	//GET_STACK(HouseClass*, source, STACK_OFFS(0x0, -0x8));
	GET_STACK(bool, force, STACK_OFFS(0x0, -0xC));

	// if it's no force shield then it's the iron curtain.
	auto pData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	double modifier = force ? pData->ForceShield_Modifier : pData->IronCurtain_Modifier;
	duration = static_cast<int>(duration * modifier);

	pThis->IronCurtainTimer.Start(duration);
	pThis->IronTintStage = 0;
	pThis->ForceShielded = force ? TRUE : FALSE;

	R->EAX(DamageState::Unaffected);
	return 0x70E2FD;
}

// update the vehicle thief's destination. needed to follow a
// target without the requirement to also enable Thief=yes.
DEFINE_HOOK(5202F9, InfantryClass_UpdateVehicleThief_Check, 6)
{
	GET(InfantryClass*, pThis, ESI);

	// good old WW checks for Thief. idiots.
	if(!pThis->Type->VehicleThief) {
		// also allow for drivers, because vehicles may still drive around. usually they are not.
		TechnoTypeExt::ExtData* pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
		if(!pTypeExt->CanDrive) {
			return 0x5206A1;
		}
	}

	return 0x52030D;
}

// the hijacker is close to the target. capture.
DEFINE_HOOK(5203F7, InfantryClass_UpdateVehicleThief_Hijack, 5)
{
	enum {GoOn = 0x5206A1, Stop = 0x520473};

	GET(InfantryClass*, pThis, ESI);
	GET(FootClass*, pTarget, EDI);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	bool finalize = pExt->PerformActionHijack(pTarget);
	if(finalize) {
		// manually deinitialize this infantry
		pThis->UnInit();
	}
	return finalize ? Stop : GoOn;
}

DEFINE_HOOK(51E7BF, InfantryClass_GetCursorOverObject_CanCapture, 6)
{
	GET(InfantryClass *, pSelected, EDI);
	GET(ObjectClass *, pTarget, ESI);

	enum { 
		Capture = 0x51E84B,  // the game will return an Enter cursor no questions asked
		DontCapture = 0x51E85A, // the game will assume this is not a VehicleThief and will check for other cursors normally
		Select = 0x51E7EF, // select target instead of ordering this
		DontMindMe = 0, // the game will check if this is a VehicleThief
	} DoWhat = DontMindMe;

	if(TechnoClass* pTechno = generic_cast<TechnoClass*>(pTarget)) {
		if(pTechno->GetTechnoType()->IsTrain) {
			DoWhat = Select;
		} else {
			TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pSelected);
			TechnoTypeExt::ExtData* pTypeExt = TechnoTypeExt::ExtMap.Find(pSelected->Type);
			if(pSelected->Type->VehicleThief || pTypeExt->CanDrive) {
				DoWhat = (pExt->GetActionHijack(pTechno) != AresAction::None ? Capture : DontCapture);
			}
		}
	}

	return DoWhat;
}

// change all the special things infantry do, like vehicle thief, infiltration,
// bridge repair, enter transports or bio reactors, ...
DEFINE_HOOK(519675, InfantryClass_UpdatePosition_BeforeInfantrySpecific, A)
{
	// called after FootClass:UpdatePosition has been called and before
	// all specific infantry handling takes place.
	enum { 
		Return = 0x51AA01, // skip the original logic
		Destroy = 0x51A010, // uninits this infantry and returns
		Handle = 0 // resume the original function
	} DoWhat = Handle;

	GET(InfantryClass*, pThis, ESI);

	if(pThis) {
		// steal vehicles / reclaim KillDriver'd units using CanDrive
		if(pThis->CurrentMission == Mission::Capture) {
			if(TechnoClass* pDest = generic_cast<TechnoClass*>(pThis->Destination)) {
				// this is the possible target we stand on
				CellClass* pCell = pThis->GetCell();
				TechnoClass* pTarget = pCell->GetUnit(pThis->OnBridge);
				if(!pTarget) {
					pTarget = pCell->GetAircraft(pThis->OnBridge);
					if(!pTarget) {
						pTarget = pCell->GetBuilding();
						if(pTarget && !pTarget->IsStrange()) {
							pTarget = nullptr;
						}
					}
				}

				// reached its destination?
				if(pTarget && pTarget == pDest) {
					// reached the target. capture.
					TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
					bool finalize = pExt->PerformActionHijack(pTarget);
					DoWhat = finalize ? Destroy : Return;
				}
			}
		}
	}

	return DoWhat;
}

DEFINE_HOOK(471C96, CaptureManagerClass_CanCapture, A)
{
	// this is a complete rewrite, because it might be easier to change
	// this in a central place than spread all over the source code.
	enum { 
		Allowed = 0x471D2E, // this can be captured
		Disallowed = 0x471D35 // can't be captured
	};

	GET(CaptureManagerClass*, pThis, ECX);
	GET(TechnoClass*, pTarget, ESI);
	TechnoClass* pCapturer = pThis->Owner;

	// target exists and doesn't belong to capturing player
	if(!pTarget || pTarget->Owner == pCapturer->Owner) {
		return Disallowed;
	}

	// generally not capturable
	if(pTarget->GetTechnoType()->ImmuneToPsionics) {
		return Disallowed;
	}

	// disallow capturing bunkered units
	if(pTarget->BunkerLinkedItem && pTarget->BunkerLinkedItem->WhatAmI() == AbstractType::Unit) {
		return Disallowed;
	}

	// TODO: extend this for mind-control priorities
	if(pTarget->IsMindControlled() || pTarget->MindControlledByHouse) {
		return Disallowed;
	}

	// free slot? (move on if infinite or single slot which will be freed if used)
	if(!pThis->InfiniteMindControl && pThis->MaxControlNodes != 1 && pThis->ControlNodes.Count >= pThis->MaxControlNodes) {
		return Disallowed;
	}

	// currently disallowed
	auto mission = pTarget->CurrentMission;
	if(pTarget->IsIronCurtained() || mission == Mission::Selling || mission == Mission::Construction) {
		return Disallowed;
	}

    // driver killed. has no mind.
	TechnoExt::ExtData* pTargetExt = TechnoExt::ExtMap.Find(pTarget);
	if(pTargetExt->DriverKilled) {
		return Disallowed;
	}

	// passed all tests
	return Allowed;
}

DEFINE_HOOK(53C450, TechnoClass_CanBePermaMC, 5)
{
	// complete rewrite. used by psychic dominator, ai targeting, etc.
	GET(TechnoClass*, pThis, ECX);
	BYTE ret = 0;

	if(pThis && pThis->WhatAmI() != AbstractType::Building
		&& !pThis->IsIronCurtained() && !pThis->IsInAir()) {

		TechnoTypeClass* pType = pThis->GetTechnoType();
		if(!pType->ImmuneToPsionics && !pType->BalloonHover) {
			
			// KillDriver check
			TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
			if(!pExt->DriverKilled) {
				ret = 1;
			}
		}
	}

	R->AL(ret);
	return 0x53C4BA;
}

DEFINE_HOOK(73758A, UnitClass_ReceivedRadioCommand_QueryEnterAsPassenger_KillDriver, 6)
{
	// prevent units from getting the enter cursor on transports
	// with killed drivers.
	GET(TechnoClass*, pThis, ESI);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	return (pExt->DriverKilled ? 0x73761Fu : 0u);
}

DEFINE_HOOK(41946B, AircraftClass_ReceivedRadioCommand_QueryEnterAsPassenger_KillDriver, 6)
{
	// prevent units from getting the enter cursor on transports
	// with killed drivers.
	GET(TechnoClass*, pThis, ESI);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	return (pExt->DriverKilled ? 0x4190DDu : 0u);
}

DEFINE_HOOK(6F6A58, TechnoClass_DrawHealthBar_HidePips_KillDriver, 6)
{
	// prevent player from seeing pips on transports with killed drivers.
	GET(TechnoClass*, pThis, ESI);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	return (pExt->DriverKilled ? 0x6F6AB6u : 0u);
}

DEFINE_HOOK(7087EB, TechnoClass_ShouldRetaliate_KillDriver, 6)
{
	// prevent units with killed drivers from retaliating.
	GET(TechnoClass*, pThis, ESI);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	return (pExt->DriverKilled ? 0x708B17u : 0u);
}

DEFINE_HOOK(7091D6, TechnoClass_CanPassiveAquire_KillDriver, 6)
{
	// prevent units with killed drivers from looking for victims.
	GET(TechnoClass*, pThis, ESI);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	return (pExt->DriverKilled ? 0x70927Du : 0u);
}

DEFINE_HOOK(6F3283, TechnoClass_CanScatter_KillDriver, 8)
{
	// prevent units with killed drivers from scattering when attacked.
	GET(TechnoClass*, pThis, ESI);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	return (pExt->DriverKilled ? 0x6F32C5u : 0u);
}

DEFINE_HOOK(5198AD, InfantryClass_UpdatePosition_EnteredGrinder, 6)
{
	GET(InfantryClass *, Infantry, ESI);
	GET(BuildingClass *, Grinder, EBX);

	BuildingExt::ExtData *pData = BuildingExt::ExtMap.Find(Grinder);

	if(pData->ReverseEngineer(Infantry)) {
		if(Infantry->Owner->ControlledByPlayer()) {
			VoxClass::Play("EVA_ReverseEngineeredInfantry");
			VoxClass::Play("EVA_NewTechnologyAcquired");
		}
	}

	return 0;
}

DEFINE_HOOK(73A1BC, UnitClass_UpdatePosition_EnteredGrinder, 7)
{
	GET(UnitClass *, Vehicle, EBP);
	GET(BuildingClass *, Grinder, EBX);

	BuildingExt::ExtData *pData = BuildingExt::ExtMap.Find(Grinder);

	if(pData->ReverseEngineer(Vehicle)) {
		if(Vehicle->Owner->ControlledByPlayer()) {
			VoxClass::Play("EVA_ReverseEngineeredVehicle");
			VoxClass::Play("EVA_NewTechnologyAcquired");
		}
	}

	// #368: refund hijackers
	if(Vehicle->HijackerInfantryType != -1) {
		if(InfantryTypeClass *Hijacker = InfantryTypeClass::Array->GetItem(Vehicle->HijackerInfantryType)) {
			int refund = Hijacker->GetRefund(Vehicle->Owner, 0);
			Grinder->Owner->GiveMoney(refund);
		}
	}

	return 0;
}

DEFINE_HOOK(6F6AC9, TechnoClass_Remove, 6) {
	GET(TechnoClass *, pThis, ESI);
	TechnoExt::ExtData* TechnoExt = TechnoExt::ExtMap.Find(pThis);

	// if the removed object is a radar jammer, unjam all jammed radars
	TechnoExt->RadarJam = nullptr;

	// #617 powered units
	TechnoExt->PoweredUnit = nullptr;

	//#1573, #1623, #255 attached effects
	auto& Effects = TechnoExt->AttachedEffects;
	if(!Effects.empty()) {
		//auto const pID = pThis->GetTechnoType()->ID;
		for(auto& Item : Effects) {
			//Debug::Log("[AttachEffect] Removing %d. item from %s\n",
			//	&Item - Effects.data(), pID);
			Item.KillAnim();
		}

		auto const it = std::remove_if(
			Effects.begin(), Effects.end(),
			[](AttachEffectClass const& Item)
		{
			return static_cast<bool>(Item.Type->DiscardOnEntry);
		});

		if(it != Effects.end()) {
			Effects.erase(it, Effects.end());
			TechnoExt->RecalculateStats();
		}

		TechnoExt->AttachEffects_RecreateAnims = true;
	}

	return pThis->InLimbo ? 0x6F6C93u : 0x6F6AD5u;
}

DEFINE_HOOK(74642C, UnitClass_ReceiveGunner, 6)
{
	GET(UnitClass *, Unit, ESI);
	auto pData = TechnoExt::ExtMap.Find(Unit);
	pData->MyOriginalTemporal = Unit->TemporalImUsing;
	Unit->TemporalImUsing = nullptr;
	return 0;
}

DEFINE_HOOK(74653C, UnitClass_RemoveGunner, 0)
{
	GET(UnitClass *, Unit, EDI);
	auto pData = TechnoExt::ExtMap.Find(Unit);
	Unit->TemporalImUsing = pData->MyOriginalTemporal;
	pData->MyOriginalTemporal = nullptr;
	return 0x746546;
}


DEFINE_HOOK(741206, UnitClass_GetFireError, 6)
{
	GET(UnitClass *, Unit, ESI);
	auto Type = Unit->Type;
	if(!Type->TurretCount || Type->IsGattling) {
		return 0x741229;
	}
	auto idxW = Unit->SelectWeapon(nullptr);
	auto W = Unit->GetWeapon(idxW);
	return (W->WeaponType && W->WeaponType->Warhead->Temporal)
		? 0x741210u
		: 0x741229u
	;
}

// bug #1290: carryall size limit
DEFINE_HOOK(417D75, AircraftClass_GetCursorOverObject_CanTote, 5)
{
	GET(AircraftClass *, pCarryall, ESI);
	GET(UnitClass *, pTarget, EDI);

	auto pCarryallData = TechnoTypeExt::ExtMap.Find(pCarryall->Type);

	return (pCarryallData->CarryallCanLift(pTarget))
		? 0u
		: 0x417DF6u
	;
}

DEFINE_HOOK(416E37, AircraftClass_Mi_MoveCarryall_CanTote, 5)
{
	GET(AircraftClass *, pCarryall, ESI);
	GET(UnitClass *, pTarget, EDI);

	auto pCarryallData = TechnoTypeExt::ExtMap.Find(pCarryall->Type);

	return (pCarryallData->CarryallCanLift(pTarget))
		? 0u
		: 0x416EC9u
	;
}

DEFINE_HOOK(416C4D, AircraftClass_Carryall_Unload_DestroyCargo, 5)
{
	GET(AircraftClass* , pCarryall, EDI);
	GET(UnitClass *, pCargo, ESI);

	int Damage = pCargo->Health;
	pCargo->ReceiveDamage(&Damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);

	Damage = pCarryall->Health;
	pCarryall->ReceiveDamage(&Damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, true, nullptr);

	return 0x416C53;
}

DEFINE_HOOK(416C94, AircraftClass_Carryall_Unload_UpdateCargo, 6)
{
	GET(UnitClass *, pCargo, ESI);

	pCargo->UpdatePosition(2);

	if(pCargo->Deactivated && pCargo->Locomotor->Is_Powered()) {
		pCargo->Locomotor->Power_Off();
	}

	return 0;
}

// support Occupier and VehicleThief on one type. if this is not done
// the Occupier handling will leave a dangling Destination pointer.
DEFINE_HOOK(4D9A83, FootClass_PointerGotInvalid_OccupierVehicleThief, 6)
{
	GET(InfantryClass*, pInfantry, ESI);
	GET(InfantryTypeClass*, pType, EAX);

	if(pType->VehicleThief) {
		if(abstract_cast<FootClass*>(pInfantry->Destination)) {
			return 0x4D9AB9;
		}
	}

	return 0;
}

// issue #895788: cells' high occupation flags are marked only if they
// actually contains a bridge while unmarking depends solely on object
// height above ground. this mismatch causes the cell to become blocked.
DEFINE_HOOK(7441B6, UnitClass_MarkOccupationBits, 6)
{
	GET(UnitClass*, pThis, ECX);
	GET(CoordStruct*, pCrd, ESI);

	CellClass* pCell = MapClass::Instance->GetCellAt(*pCrd);
	int height = MapClass::Instance->GetCellFloorHeight(*pCrd) + CellClass::BridgeHeight;
	bool alt = (pCrd->Z >= height && pCell->ContainsBridge());

	// remember which occupation bit we set
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->AltOccupation = alt;

	if(alt) {
		pCell->AltOccupationFlags |= 0x20;
	} else {
		pCell->OccupationFlags |= 0x20;
	}

	return 0x744209;
}

DEFINE_HOOK(744216, UnitClass_UnmarkOccupationBits, 6)
{
	GET(UnitClass*, pThis, ECX);
	GET(CoordStruct*, pCrd, ESI);

	enum { obNormal = 1, obAlt = 2 };

	CellClass* pCell = MapClass::Instance->GetCellAt(*pCrd);
	int height = MapClass::Instance->GetCellFloorHeight(*pCrd) + CellClass::BridgeHeight;
	int alt = (pCrd->Z >= height) ? obAlt : obNormal;

	// also clear the last occupation bit, if set
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	if(!pExt->AltOccupation.empty()) {
		int lastAlt = pExt->AltOccupation ? obAlt : obNormal;
		alt |= lastAlt;
		pExt->AltOccupation.clear();
	}

	if(alt & obAlt) {
		pCell->AltOccupationFlags &= ~0x20;
	}

	if(alt & obNormal)
	{
		pCell->OccupationFlags &= ~0x20;
	}

	return 0x74425E;
}

DEFINE_HOOK(70DEBA, TechnoClass_UpdateGattling_Cycle, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, lastStageValue, EAX);
	GET_STACK(int, a2, 0x24);

	auto pType = pThis->GetTechnoType();

	if(pThis->GattlingValue < lastStageValue) {
		// just increase the value
		pThis->GattlingValue += a2 * pType->RateUp;
	} else {
		// if max or higher, reset cyclic gattlings
		auto pExt = TechnoTypeExt::ExtMap.Find(pType);

		if(pExt->GattlingCyclic) {
			pThis->GattlingValue = 0;
			pThis->CurrentGattlingStage = 0;
			pThis->Audio4.DTOR_1();
			pThis->unknown_bool_4B8 = false;
		}
	}

	// recreate hooked instruction
	R->Stack<int>(0x10, pThis->GattlingValue);

	return 0x70DEEB;
}

// make the space between gunner name segment and ifv
// name smart. it disappears if one of them is empty,
// eliminating leading and trailing spaces.
DEFINE_HOOK(746C55, UnitClass_GetUIName, 6)
{
	GET(UnitClass*, pThis, ESI);
	GET(wchar_t*, pGunnerName, EAX);

	auto pName = pThis->Type->UIName;

	auto pSpace = L"";
	if(pName && *pName && pGunnerName && *pGunnerName) {
		pSpace = L" ";
	}

	_snwprintf_s(pThis->ToolTipText, _TRUNCATE, L"%s%s%s", pGunnerName, pSpace, pName);

	R->EAX(pThis->ToolTipText);
	return 0x746C76;
}

// spawn tiberium when a unit dies. this is a minor part of the
// tiberium heal feature. the actual healing happens in FootClass_Update.
DEFINE_HOOK(702216, TechnoClass_ReceiveDamage_TiberiumHeal, 6)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	// TS did not check for HasAbility here, either
	if(pExt->TiberiumRemains.Get(pType->TiberiumHeal && RulesExt::Global()->Tiberium_HealEnabled)) {
		CoordStruct crd = pThis->GetCoords();
		CellClass* pCenter = MapClass::Instance->GetCellAt(crd);

		// increase the tiberium for the four neighbours and center.
		// center is retrieved by getting a neighbour cell index >= 8
		for(auto i = 0u; i < 5u; ++i) {
			CellClass* pCell = pCenter->GetNeighbourCell(2*i);
			int value = ScenarioClass::Instance->Random.RandomRanged(0, 2);
			pCell->IncreaseTiberium(0, value);
		}
	}

	return 0;
}

// damage the techno when it is moving over a cell containing tiberium
DEFINE_HOOK(4D85E4, FootClass_UpdatePosition_TiberiumDamage, 9)
{
	GET(FootClass*, pThis, ESI);

	int damage = 0;
	WarheadTypeClass* pWarhead = nullptr;
	int transmogrify = RulesClass::Instance->TiberiumTransmogrify;

	if(RulesExt::Global()->Tiberium_DamageEnabled && pThis->GetHeight() <= RulesClass::Instance->HoverHeight) {
		TechnoTypeClass* pType = pThis->GetTechnoType();
		TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pType);

		// default is: infantry can be damaged, others cannot
		bool enabled = (pThis->WhatAmI() != InfantryClass::AbsID);

		if(!pExt->TiberiumProof.Get(enabled) && !pThis->HasAbility(Ability::TiberiumProof)) {
			if(pThis->Health > 0) {
				CellClass* pCell = pThis->GetCell();
				int idxTiberium = pCell->GetContainedTiberiumIndex();
				if(auto pTiberium = TiberiumClass::Array->GetItemOrDefault(idxTiberium)) {
					auto pTibExt = TiberiumExt::ExtMap.Find(pTiberium);

					pWarhead = pTibExt->GetWarhead();
					damage = pTibExt->GetDamage();

					transmogrify = pExt->TiberiumTransmogrify.Get(transmogrify);
				}
			}
		}
	}

	if(damage && pWarhead) {
		CoordStruct crd = pThis->GetCoords();

		if(pThis->ReceiveDamage(&damage, 0, pWarhead, nullptr, false, false, nullptr) == DamageState::NowDead) {
			TechnoExt::SpawnVisceroid(crd, RulesClass::Instance->SmallVisceroid, transmogrify, false);
			return 0x4D8F29;
		}
	}

	return 0;
}

// spill the stored tiberium on destruction
DEFINE_HOOK(702200, TechnoClass_ReceiveDamage_SpillTiberium, 6)
{
	GET(TechnoClass*, pThis, ESI);

	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	if(pExt->TiberiumSpill) {
		double stored = pThis->Tiberium.GetTotalAmount();
		if(pThis->WhatAmI() != BuildingClass::AbsID
			&& stored > 0.0
			&& !ScenarioClass::Instance->SpecialFlags.HarvesterImmune)
		{
			// don't spill more than we can hold
			double max = 9.0;
			if(max > pType->Storage) {
				max = pType->Storage;
			}

			// assume about half full, recalc if possible
			int value = static_cast<int>(max / 2);
			if(pType->Storage > 0) {
				value = Game::F2I(stored / pType->Storage * max);
			}

			// get the spill center
			CoordStruct crd = pThis->GetCoords();
			CellClass* pCenter = MapClass::Instance->GetCellAt(crd);

			const unsigned int Neighbours[] = {9, 2, 7, 1, 4, 3, 0, 5, 6};
			for(auto neighbour : Neighbours) {
				// spill random amount
				int amount = ScenarioClass::Instance->Random.RandomRanged(0, 2);
				CellClass* pCell = pCenter->GetNeighbourCell(neighbour);
				pCell->IncreaseTiberium(0, amount);
				value -= amount;

				// stop if value is reached
				if(value <= 0) {
					break;
				}
			}
		}
	}

	return 0;
}

// blow up harvester units big time
DEFINE_HOOK(738749, UnitClass_Destroy_TiberiumExplosive, 6)
{
	GET(UnitClass*, pThis, ESI);

	if(RulesClass::Instance->TiberiumExplosive) {
		if(!ScenarioClass::Instance->SpecialFlags.HarvesterImmune) {
			if(pThis->Tiberium.GetTotalAmount() > 0.0f) {

				// multiply the amounts with their powers and sum them up
				int morePower = 0;
				for(int i=0; i<TiberiumClass::Array->Count; ++i) {
					TiberiumClass* pTiberium = TiberiumClass::Array->GetItem(i);
					double power = pThis->Tiberium.GetAmount(i) * pTiberium->Power;
					morePower += Game::F2I(power);
				}

				// go boom
				WarheadTypeClass* pWH = RulesExt::Global()->Tiberium_ExplosiveWarhead;
				if(morePower > 0 && pWH) {
					CoordStruct crd = pThis->GetCoords();
					MapClass::DamageArea(crd, morePower, pThis, pWH, false, nullptr);
				}
			}
		}
	}

	return 0x7387C4;
}

// merge two small visceroids into one large visceroid
DEFINE_HOOK(739F21, UnitClass_UpdatePosition_Visceroid, 6)
{
	GET(UnitClass*, pThis, EBP);

	// fleshbag erotic
	if(pThis->Type->SmallVisceroid) {
		if(UnitTypeClass* pLargeType = RulesClass::Instance->LargeVisceroid) {
			if(UnitClass* pDest = specific_cast<UnitClass*>(pThis->Destination)) {
				if(pDest->Type->SmallVisceroid) {

					// nice to meat you!
					auto crdMe = pThis->GetCoords();
					auto crdHim = pDest->GetCoords();

					auto cellMe = CellClass::Coord2Cell(crdMe);
					auto cellHim = CellClass::Coord2Cell(crdHim);

					// two become one
					if(cellMe == cellHim) {
						pDest->Type = pLargeType;
						pDest->Health = pLargeType->Strength;

						CellClass* pCell = MapClass::Instance->GetCellAt(pDest->LastMapCoords);
						pDest->UpdateThreatInCell(pCell);

						pThis->UnInit();
						return 0x73B0A5;
					}
				}
			}
		}
	}

	return 0;
}

// complete rewrite
DEFINE_HOOK(4D98C0, FootClass_Destroyed, A) {
	GET(FootClass*, pThis, ECX);
	//GET_STACK(AbstractClass*, pKiller, 0x4);
	auto pType = pThis->GetTechnoType();

	// exclude unimportant units, and only play for current player
	if(!pType->DontScore && !pType->Insignificant && !pType->Spawned
		&& pThis->Owner->ControlledByPlayer())
	{
		auto pExt = TechnoTypeExt::ExtMap.Find(pType);
		int idx = pExt->EVA_UnitLost;
		if(idx != -1) {
			CellStruct cell = pThis->GetMapCoords();
			RadarEventClass::Create(RadarEventType::UnitLost, cell);
			VoxClass::PlayIndex(idx, -1, -1);
		}
	}

	return 0x4D9918;
}

// linking units for type selection
DEFINE_HOOK(732C30, TechnoClass_IDMatches, 5)
{
	GET(TechnoClass*, pThis, ECX);
	GET(DynamicVectorClass<const char*>*, pNames, EDX);

	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);
	const char* id = pExt->GetSelectionGroupID();

	bool match = false;

	// find any match
	for(auto i=pNames->begin(); i<pNames->end(); ++i) {
		if(!_strcmpi(*i, id)) {
			if(pThis->CanBeSelectedNow()) {
				match = true;
				break;
			}

			// buildings are exempt if they can't undeploy
			if(pThis->WhatAmI() == BuildingClass::AbsID && pType->UndeploysInto) {
				match = true;
				break;
			}
		}
	}

	R->EAX(match ? 1 : 0);
	return 0x732C97;
}

// #1283653: fix for jammed buildings and attackers in open topped transports
DEFINE_HOOK(702A38, TechnoClass_ReceiveDamage_OpenTopped, 7)
{
	REF_STACK(TechnoClass*, pAttacker, STACK_OFFS(0xC4, -0x10));

	// decide as if the transporter fired at this building
	if(pAttacker && pAttacker->InOpenToppedTransport && pAttacker->Transporter) {
		pAttacker = pAttacker->Transporter;
	}

	R->EDI(pAttacker);
	return 0x702A3F;
}

// #912875: respect the remove flag for invalidating SpawnManager owners
DEFINE_HOOK(707B19, TechnoClass_PointerGotInvalid_SpawnCloakOwner, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(void*, ptr, EBP);
	REF_STACK(bool, remove, STACK_OFFS(0x20, -0x8));

	if(auto pSM = pThis->SpawnManager) {
		// ignore disappearing owner
		if(remove || pSM->Owner != ptr) {
			R->ECX(pSM);
			return 0x707B23;
		}
	}

	return 0x707B29;
}

// flying aircraft carriers
// allow spawned units to spawn above ground
DEFINE_HOOK(414338, AircraftClass_Put_SpawnHigh, 6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(AircraftTypeClass*, pType, ECX);

	R->EAX(pType->MissileSpawn || pThis->SpawnOwner);
	return 0x41433E;
}

// aim for the cell for flying carriers
DEFINE_HOOK(6B783B, SpawnManagerClass_Update_SpawnHigh, 5)
{
	GET(SpawnManagerClass*, pThis, ESI);

	AbstractClass* pDest = pThis->Owner;
	if(pThis->Owner->GetHeight() > 0) {
		pDest = pThis->Owner->GetCell();
	}

	R->EAX(pDest);
	return 0;
}

// issue #279: per unit AirstrikeAttackVoice and AirstrikeAbortSound
DEFINE_HOOK(41D940, AirstrikeClass_Fire_AirstrikeAttackVoice, 5)
{
	GET(AirstrikeClass*, pAirstrike, EDI);

	// get default from rules
	int index = RulesClass::Instance->AirstrikeAttackVoice;

	// get from aircraft
	auto pAircraftExt = TechnoTypeExt::ExtMap.Find(pAirstrike->FirstObject->GetTechnoType());
	index = pAircraftExt->VoiceAirstrikeAttack.Get(index);

	// get from designator
	if(auto pOwner = pAirstrike->Owner) {
		auto pOwnerExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());
		index = pOwnerExt->VoiceAirstrikeAttack.Get(index);
	}

	VocClass::PlayAt(index, pAirstrike->FirstObject->Location, nullptr);
	return 0x41D970;
}

DEFINE_HOOK(41D5AE, AirstrikeClass_PointerGotInvalid_AirstrikeAbortSound, 9)
{
	GET(AirstrikeClass*, pAirstrike, ESI);

	// get default from rules
	int index = RulesClass::Instance->AirstrikeAbortSound;

	// get from aircraft
	auto pAircraftExt = TechnoTypeExt::ExtMap.Find(pAirstrike->FirstObject->GetTechnoType());
	index = pAircraftExt->VoiceAirstrikeAbort.Get(index);

	// get from designator
	if(auto pOwner = pAirstrike->Owner) {
		auto pOwnerExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());
		index = pOwnerExt->VoiceAirstrikeAbort.Get(index);
	}

	VocClass::PlayAt(index, pAirstrike->FirstObject->Location, nullptr);
	return 0x41D5E0;
}

DEFINE_HOOK(702CFE, TechnoClass_ReceiveDamage_PreventScatter, 6)
{
	GET(FootClass*, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0xC4, -0xC));

	auto pExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	// only allow to scatter if not prevented
	if(!pExt->PreventScatter) {
		pThis->Scatter(CoordStruct::Empty, true, false);
	}

	return 0x702D11;
}

DEFINE_HOOK(6F826E, TechnoClass_CanAutoTargetObject_CivilianEnemy, 5)
{
	GET(TechnoClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);
	GET(TechnoTypeClass*, pTargetType, EBP);

	enum { Undecided = 0, ConsiderEnemy = 0x6F8483, ConsiderCivilian = 0x6F83B1, Ignore = 0x6F894F };

	auto pExt = TechnoTypeExt::ExtMap.Find(pTargetType);

	// always consider this an enemy
	if(pExt->CivilianEnemy) {
		return ConsiderEnemy;
	}

	// if the potential target is attacking an allied object, consider it an enemy
	// to not allow civilians to overrun a player
	if(auto pTargetTarget = abstract_cast<TechnoClass*>(pTarget->Target)) {
		auto pOwner = pThis->Owner;
		if(pOwner->IsAlliedWith(pTargetTarget)) {
			auto pData = RulesExt::Global();

			bool repel = pOwner->ControlledByHuman() ? pData->AutoRepelPlayer : pData->AutoRepelAI;
			if(repel) {
				return ConsiderEnemy;
			}
		}
	}

	return Undecided;
}

// particle system related

// make damage sparks customizable, using game setting as default.
DEFINE_HOOK(6FACD9, TechnoClass_Update_DamageSparks, 6)
{
	GET(TechnoTypeClass*, pType, EBX);
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	bool sparks = pExt->DamageSparks.Get(pType->DamageSparks);

	R->EAX(sparks);
	return 0x6FACDF;
}

// smoke particle systems created when a techno is damaged
DEFINE_HOOK(702894, TechnoClass_ReceiveDamage_SmokeParticles, 6)
{
	GET(TechnoClass*, pThis, ESI);
	REF_STACK(DynamicVectorClass<ParticleSystemTypeClass const *>, Systems, 0x30);

	auto pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	auto it = pExt->ParticleSystems_DamageSmoke.GetElements(pType->DamageParticleSystems);
	auto allowAny = pExt->ParticleSystems_DamageSmoke.HasValue();

	for(auto pSystem : it) {
		if(allowAny || pSystem->BehavesLike == BehavesLike::Smoke) {
			Systems.AddItem(pSystem);
		}
	}

	return 0x702938;
}

// spark particle systems created at random intervals
DEFINE_HOOK(6FAD49, TechnoClass_Update_SparkParticles, 0) // breaks the loop
{
	GET(TechnoClass*, pThis, ESI);
	REF_STACK(DynamicVectorClass<ParticleSystemTypeClass const *>, Systems, 0x60);

	auto pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	auto it = pExt->ParticleSystems_DamageSparks.GetElements(pType->DamageParticleSystems);
	auto allowAny = pExt->ParticleSystems_DamageSparks.HasValue();

	for(auto pSystem : it) {
		if(allowAny || pSystem->BehavesLike == BehavesLike::Spark) {
			Systems.AddItem(pSystem);
		}
	}

	return 0x6FADB3;
}

// customizable berserk fire rate modification
DEFINE_HOOK(6FF28F, TechnoClass_Fire_BerserkROFMultiplier, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, ROF, EAX);

	if(pThis->Berzerk) {
		auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
		double multiplier = pExt->BerserkROFMultiplier.Get(RulesExt::Global()->BerserkROFMultiplier);
		ROF = static_cast<int>(ROF * multiplier);
	}

	R->EAX(ROF);
	return 0x6FF29E;
}

DEFINE_HOOK(6FE31C, TechnoClass_Fire_AllowDamage, 8)
{
	//GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	auto pExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	// whether conventional damage should be used
	bool applyDamage = pExt->ApplyDamage.Get(!pWeapon->IsSonic && !pWeapon->UseFireParticles);

	if(!applyDamage) {
		// clear damage
		R->EDI(0);
	}

	return applyDamage ? 0x6FE32Fu : 0x6FE3DFu;
}

// issue #1324: enemy repair wrench visible when it shouldn't
DEFINE_HOOK(6F525B, TechnoClass_DrawExtras_PowerOff, 5)
{
	GET(TechnoClass*, pTechno, EBP);
	GET_STACK(RectangleStruct*, pRect, 0xA0);

	if(auto pBld = abstract_cast<BuildingClass*>(pTechno)) {
		auto const pExt = BuildingExt::ExtMap.Find(pBld);

		// allies and observers can always see by default
		bool canSeeRepair = HouseClass::Player->IsAlliedWith(pBld->Owner)
			|| HouseClass::IsPlayerObserver();

		bool showRepair = FileSystem::WRENCH_SHP
			&& pBld->IsBeingRepaired
			// fixes the wrench playing over a temporally challenged building
			&& !pBld->IsBeingWarpedOut()
			&& !pBld->WarpingOut
			// never show to enemies when cloaked, and only if allowed
			&& (canSeeRepair || (pBld->CloakState == CloakState::Uncloaked && RulesExt::Global()->EnemyWrench));

		// display power off marker only for current player's buildings
		bool showPower = FileSystem::POWEROFF_SHP
			&& !pExt->TogglePower_HasPower
			// only for owned buildings, but observers got magic eyes
			&& (pBld->Owner->ControlledByPlayer() || HouseClass::IsPlayerObserver());

		// display any?
		if(showPower || showRepair) {
			auto cell = pBld->GetMapCoords();

			if(!MapClass::Instance->GetCellAt(cell)->IsShrouded()) {
				CoordStruct crd;
				pBld->GetPosition_2(&crd);

				Point2D point;
				TacticalClass::Instance->CoordsToClient(&crd, &point);

				// offset the markers
				Point2D ptRepair = point;
				if(showPower) {
					ptRepair.X -= 7;
					ptRepair.Y -= 7;
				}

				Point2D ptPower = point;
				if(showRepair) {
					ptPower.X += 18;
					ptPower.Y += 18;
				}

				// animation display speed
				// original frame calculation: ((currentframe%speed)*6)/(speed-1)
				int speed = GameOptionsClass::Instance->GetAnimSpeed(14) / 4;
				if(speed < 2) {
					speed = 2;
				}

				// draw the markers
				if(showRepair) {
					int frame = (FileSystem::WRENCH_SHP->Frames * (Unsorted::CurrentFrame % speed)) / speed;
					DSurface::Hidden_2->DrawSHP(FileSystem::MOUSE_PAL, FileSystem::WRENCH_SHP,
						frame, &ptRepair, pRect, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
				}

				if(showPower) {
					int frame = (FileSystem::POWEROFF_SHP->Frames * (Unsorted::CurrentFrame % speed)) / speed;
					DSurface::Hidden_2->DrawSHP(FileSystem::MOUSE_PAL, FileSystem::POWEROFF_SHP,
						frame, &ptPower, pRect, BlitterFlags(0xE00), 0, 0, 0, 1000, 0, 0, 0, 0, 0);
				}
			}
		}
	}

	return 0x6F5347;
}

DEFINE_HOOK(741613, UnitClass_ApproachTarget_OmniCrusher, 6)
{
	GET(UnitClass* const, pThis, ESI);

	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	auto const aggressive = pExt->OmniCrusher_Aggressive;

	return aggressive ? 0u : 0x741685u;
}

DEFINE_HOOK(7418AA, UnitClass_CrushCell_CrushDamage, 6)
{
	GET(UnitClass* const, pThis, EDI);
	GET(ObjectClass* const, pVictim, ESI);

	if(auto const pTechno = abstract_cast<TechnoClass*>(pVictim)) {
		auto pExt = TechnoTypeExt::ExtMap.Find(pVictim->GetTechnoType());

		auto const pWarhead = pExt->CrushDamageWarhead.Get(
			RulesClass::Instance->C4Warhead);

		auto damage = pExt->CrushDamage.Get(pTechno);

		if(pWarhead && damage) {
			pThis->ReceiveDamage(
				&damage, 0, pWarhead, nullptr, false, false, nullptr);
		}
	}

	return 0;
}

DEFINE_HOOK(4D9920, FootClass_SelectAutoTarget_Cloaked, 9)
{
	GET(FootClass* const, pThis, ECX);

	if(pThis->Owner->ControlledByHuman()
		&& pThis->GetCurrentMission() == Mission::Guard)
	{
		auto const pType = pThis->GetTechnoType();
		auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

		auto allowAquire = true;

		if(!pExt->CanPassiveAcquire_Guard) {
			// we are in guard mode
			allowAquire = false;
		} else if(!pExt->CanPassiveAcquire_Cloak) {
			// passive acquire is disallowed when guarding and cloakable
			if(pThis->IsCloakable() || pThis->HasAbility(Ability::Cloak)) {
				allowAquire = false;
			}
		}

		if(!allowAquire) {
			R->EAX(static_cast<TechnoClass*>(nullptr));
			return 0x4D995C;
		}
	}

	return 0;
}

DEFINE_HOOK(70BE80, TechnoClass_ShouldSelfHealOneStep, 5)
{
	GET(TechnoClass* const, pThis, ECX);
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	R->EAX(pExt->GetSelfHealAmount() > 0);
	return 0x70BF46;
}

DEFINE_HOOK(6FA743, TechnoClass_Update_SelfHeal, A)
{
	GET(TechnoClass* const, pThis, ESI);

	// prevent crashing and sinking technos from self-healing
	if(pThis->IsCrashing || pThis->IsSinking) {
		return 0x6FA941;
	}

	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	// this replaces the call to pThis->ShouldSelfHealOneStep()
	if(auto const amount = pExt->GetSelfHealAmount()) {
		pThis->Health += amount;

		R->ECX(pThis);
		return 0x6FA75A;
	}

	return 0x6FA793;
}

DEFINE_HOOK(7162B0, TechnoTypeClass_GetPipMax_MindControl, 6)
{
	GET(TechnoTypeClass const* const, pThis, ECX);

	auto const GetMindDamage = [](WeaponTypeClass const* const pWeapon) {
		return (pWeapon && pWeapon->Warhead->MindControl) ? pWeapon->Damage : 0;
	};

	auto count = GetMindDamage(pThis->Weapon[0].WeaponType);
	if(count <= 0) {
		count = GetMindDamage(pThis->Weapon[1].WeaponType);
	}

	R->EAX(count);
	return 0x7162BC;
}

DEFINE_HOOK(73769E, UnitClass_ReceivedRadioCommand_SpecificPassengers, 8)
{
	GET(UnitClass* const, pThis, ESI);
	GET(TechnoClass const* const, pSender, EDI);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	auto const pSenderType = pSender->GetTechnoType();

	auto const allowed = (pExt->PassengersWhitelist.empty() ||
			pExt->PassengersWhitelist.Contains(pSenderType))
		&& !pExt->PassengersBlacklist.Contains(pSenderType);

	return allowed ? 0u : 0x73780Fu;
}

DEFINE_HOOK(41949F, AircraftClass_ReceivedRadioCommand_SpecificPassengers, 6)
{
	GET(AircraftClass* const, pThis, ESI);
	GET_STACK(TechnoClass const* const, pSender, 0x14);

	enum { Allowed = 0x41945Fu, Disallowed = 0x41951Fu };

	auto const pType = pThis->GetTechnoType();

	if(pThis->Passengers.NumPassengers >= pType->Passengers) {
		return Disallowed;
	}

	auto const pSenderType = pSender->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	auto const allowed = (pExt->PassengersWhitelist.empty() ||
			pExt->PassengersWhitelist.Contains(pSenderType))
		&& !pExt->PassengersBlacklist.Contains(pSenderType);

	return allowed ? Allowed : Disallowed;
}

DEFINE_HOOK(740031, UnitClass_GetCursorOverObject_NoManualUnload, 6)
{
	GET(UnitClass const* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	return pExt->NoManualUnload ? 0x740115u : 0u;
}

DEFINE_HOOK(700EEC, TechnoClass_CanDeploySlashUnload_NoManualUnload, 6)
{
	// this techno is known to be a unit
	GET(UnitClass const* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	return pExt->NoManualUnload ? 0x700DCEu : 0u;
}

DEFINE_HOOK(700536, TechnoClass_GetCursorOverObject_NoManualFire, 6)
{
	GET(TechnoClass const* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	return pExt->NoManualFire ? 0x70056Cu : 0u;
}

DEFINE_HOOK(7008D4, TechnoClass_GetCursorOverCell_NoManualFire, 6)
{
	GET(TechnoClass const* const, pThis, ESI);

	auto const pType = pThis->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	return pExt->NoManualFire ? 0x700AB7u : 0u;
}

DEFINE_HOOK(51ED8E, InfantryClass_GetCursorOverObject_NoManualEnter, 6)
{
	//GET(InfantryClass const* const, pThis, ESI);
	GET(TechnoTypeClass const* const, pTargetType, EAX);

	auto const pExt = TechnoTypeExt::ExtMap.Find(pTargetType);
	bool enterable = pTargetType->Passengers > 0 && !pExt->NoManualEnter;

	return enterable ? 0x51ED9Cu : 0x51EE3Bu;
}

DEFINE_HOOK(74031A, UnitClass_GetCursorOverObject_NoManualEnter, 6)
{
	//GET(UnitClass const* const, pThis, ESI);
	GET(TechnoTypeClass const* const, pTargetType, EAX);

	auto const pExt = TechnoTypeExt::ExtMap.Find(pTargetType);
	bool enterable = pTargetType->Passengers > 0 && !pExt->NoManualEnter;

	return enterable ? 0x740324u : 0x74037Au;
}
