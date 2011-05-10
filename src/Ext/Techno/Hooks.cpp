#include "Body.h"
#include "../TechnoType/Body.h"
#include "../Building/Body.h"
#include "../BuildingType/Body.h"
#include "../../Misc/Debug.h"
#include "../../Misc/JammerClass.h"
#include "../../Misc/PoweredUnitClass.h"

#include <SpecificStructures.h>

// bugfix #297: Crewed=yes jumpjets spawn parachuted infantry on destruction, not idle
DEFINE_HOOK(737F97, UnitClass_ReceiveDamage, 0)
{
	GET(UnitClass *, t, ESI);
	GET_STACK(TechnoClass *, Killer, 0x54);
	GET_STACK(bool, select, 0x13);
	GET_STACK(bool, ignoreDefenses, 0x58);
	TechnoExt::SpawnSurvivors(t, Killer, select, ignoreDefenses);

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

	return 0;
}

DEFINE_HOOK(6F9E50, TechnoClass_Update, 5)
{
	GET(TechnoClass *, Source, ECX);

	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(Source);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Source->GetTechnoType());

	if(pData->CloakSkipTimer.IsDone()) {
		pData->CloakSkipTimer.Stop();
		Source->Cloakable = Source->GetTechnoType()->Cloakable;
	} else if(pData->CloakSkipTimer.GetTimeLeft() > 0) {
		Source->Cloakable = 0;
	}

	// #1208
	if(pTypeData) {
		if(pTypeData->PassengerTurret.Get()) {
			// 18 = 1 8 = A H = Adolf Hitler. Clearly we can't allow it to come to that.
			int passengerNumber = (Source->Passengers.NumPassengers <= 17) ? Source->Passengers.NumPassengers : 17;
			int maxTurret = Source->GetTechnoType()->TurretCount - 1;
			Source->CurrentTurretNumber = (passengerNumber <= maxTurret) ? passengerNumber : maxTurret;
		}
	}
	
	// #617 powered units
	if( pTypeData->PoweredBy.Count ) {
		if(!pData->PoweredUnit) {
			pData->PoweredUnit = new PoweredUnitClass(Source, pTypeData);
		}
		pData->PoweredUnit->Update();
	}

	return 0;
}

//! TechnoClass::Update is called every frame; returning 0 tells it to execute the original function's code as well.
DEFINE_HOOK(6F9E76, TechnoClass_Update_CheckOperators, 6)
{
	GET(TechnoClass *, pThis, ESI); // object this is called on
	//TechnoTypeClass *Type = pThis->GetTechnoType();
	//TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Type);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(pThis);

	// Related to operators/drivers, issue #342
	BuildingClass * pTheBuildingBelow = pThis->GetCell()->GetBuilding();

	/* Conditions checked:
		- Is there no building below us
		OR
		- Is this the building on this cell AND is it online

		pTheBuildingBelow will be NULL if no building was found
		This check ensures that Operator'd units don't Deactivate above structures such as War Factories, Repair Depots or Battle Bunkers.
		(Which is potentially abusable, but let's hope no one figures that out.)
	*/
	if(!pTheBuildingBelow || ((pTheBuildingBelow == pThis) && (pTheBuildingBelow->IsPowerOnline()))) {
		bool Override = false;
		if(FootClass *pFoot = generic_cast<FootClass*>(pThis)) {
			if(!pTheBuildingBelow) {
				// immobile, though not disabled. like hover tanks after
				// a repair depot has been sold or warped away.
				Override = (pFoot->Locomotor->Is_Powered() == pThis->Deactivated);
			}
		}

		if(pData->IsOperated()) { // either does have an operator or doesn't need one, so...
			if( (pThis->Deactivated && pData->IsPowered() && !pThis->IsUnderEMP()) || Override ) { // ...if it's currently off, turn it on! (oooh baby)
				pThis->Reactivate();
				pThis->Owner->ShouldRecheckTechTree = true; // #885
			}
		} else { // doesn't have an operator, so...
			if(!pThis->Deactivated) { // ...if it's not off yet, turn it off!
				pThis->Deactivate();
				pThis->Owner->ShouldRecheckTechTree = true; // #885
			}
		}
	}

	// prevent disabled units from driving around.
	if(pThis->Deactivated) {
		if(UnitClass* pUnit = specific_cast<UnitClass*>(pThis)) {
			if(pUnit->Locomotor->Is_Moving() && pUnit->Destination && !pThis->LocomotorSource) {
				pUnit->SetDestination(NULL, true);
				pUnit->StopMoving();
			}
		}

		// dropping Radar Jammers (#305) here for now; should check if another TechnoClass::Update hook might be better ~Ren
		if(pData->RadarJam) { // RadarJam should only be non-null if the object is an active radar jammer
			pData->RadarJam->UnjamAll();
		}
	} else {
		// dropping Radar Jammers (#305) here for now; should check if another TechnoClass::Update hook might be better ~Ren
		if(!!pTypeData->RadarJamRadius) {
			if(!pData->RadarJam) {
				pData->RadarJam = new JammerClass(pThis, pData);
			}

			pData->RadarJam->Update();
		}
	}

	/* 	using 0x6F9E7C instead makes this function override the original game one's entirely -
		don't activate that unless you handle _everything_ originally handled by the game */
	return 0;
}

// fix for vehicle paradrop alignment
DEFINE_HOOK(415CA6, AircraftClass_Paradrop, 6)
{
	GET(AircraftClass *, A, EDI);
	GET(FootClass *, P, ESI);
	if(P->WhatAmI() != abs_Unit) {
		return 0;
	}
	CoordStruct SrcXYZ;
	A->GetCoords(&SrcXYZ);
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
	GET(TechnoClass *, T, ESI);
	TechnoTypeClass * Type = T->GetTechnoType();
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(T);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Type);

	CaptureManagerClass *Capturer = NULL;
	ParasiteClass *Parasite = NULL;
	TemporalClass *Temporal = NULL;

	FootClass *F = generic_cast<FootClass *>(T);
	bool IsFoot = (F != NULL);

//	for(int i = 0; i < pTypeData->Weapons.get_Count(); ++i) {
//		WeaponStruct *W = &pTypeData->Weapons[i];
	TechnoTypeClass *TT = T->GetTechnoType();
	for(int i = 0; i < 18; ++i) {
		WeaponTypeClass *W1 = TT->get_Weapon(i);
		WeaponTypeClass *W2 = TT->get_EliteWeapon(i);
		if(!W1 && !W2) {
			continue;
		}
		WarheadTypeClass *WH1 = W1 ? W1->Warhead : NULL;
		WarheadTypeClass *WH2 = W2 ? W2->Warhead : NULL;

		if((W1 && !WH1) || (W2 && !WH2)) {
			Debug::FatalErrorAndExit(
				"Constructing an instance of [%s]:\r\n%sWeapon %s (slot %d) has no Warhead!",
					Type->ID,
					WH1 ? "Elite " : "",
					(WH1 ? W2 : W1)->ID,
					i);
		}

		if(WH1 && WH1->MindControl && Capturer == NULL) {
			GAME_ALLOC(CaptureManagerClass, Capturer, T, W1->Damage, W1->InfiniteMindControl);
		} else if(WH2 && WH2->MindControl && Capturer == NULL) {
			GAME_ALLOC(CaptureManagerClass, Capturer, T, W2->Damage, W2->InfiniteMindControl);
		}

		if(WH1 && WH1->Temporal && Temporal == NULL) {
			GAME_ALLOC(TemporalClass, Temporal, T);
			Temporal->WarpPerStep = W1->Damage;
			pData->idxSlot_Warp = (BYTE)i;
		} else if(WH2 && WH2->Temporal && Temporal == NULL) {
			GAME_ALLOC(TemporalClass, Temporal, T);
			Temporal->WarpPerStep = W2->Damage;
			pData->idxSlot_Warp = (BYTE)i;
		}

		if((WH1 && WH1->Parasite || WH2 && WH2->Parasite) && IsFoot && Parasite == NULL) {
			GAME_ALLOC(ParasiteClass, Parasite, F);
			pData->idxSlot_Parasite = (BYTE)i;
		}
	}

	T->CaptureManager = Capturer;
	T->TemporalImUsing = Temporal;
	if(IsFoot) {
		F->ParasiteImUsing = Parasite;
	}

	return 0x6F4102;
}

DEFINE_HOOK(6F4103, TechnoClass_Init_2, 6)
{
	return 0x6F41C0;
}

// temporal per-slot
DEFINE_HOOK(71A860, TemporalClass_UpdateA, 6)
{
	GET(TemporalClass *, Temp, ESI);
	TechnoClass *T = Temp->Owner;
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(T);
	WeaponStruct *W = T->GetWeapon(pData->idxSlot_Warp);
	R->EAX<WeaponStruct *>(W);
	return 0x71A876;
}

// temporal per-slot
DEFINE_HOOK(71AB30, TemporalClass_GetHelperDamage, 5)
{
	GET(TemporalClass *, Temp, ESI);
	TechnoClass *T = Temp->Owner;
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(T);
	WeaponStruct *W = T->GetWeapon(pData->idxSlot_Warp);
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


DEFINE_HOOK(6F3330, TechnoClass_SelectWeapon, 5)
{
	GET(TechnoClass *, pThis, ECX);
	GET_STACK(TechnoClass *, pTarg, 0x4);

//	DWORD Selected = TechnoClassExt::SelectWeaponAgainst(pThis, pTarg);
//	R->EAX(Selected);
//	return 0x6F3813;
	return 0;
}

/*
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
	return pData->Is_Deso ? 0x51F77D : 0x51F792;
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
	return pData->Is_Cow ? 0x51CEAE : 0x51CECD;
}

DEFINE_HOOK(747BBD, UnitTypeClass_LoadFromINI, 5)
{
	GET(UnitTypeClass *, U, ESI);

	U->AltImage = R->EAX<SHPStruct *>(); // jumping over, so replicated
	return U->Gunner
		? 0x747BD7
		: 0x747E90;
}

// godawful hack - Desolator deploy fire is triggered by ImmuneToRadiation !
DEFINE_HOOK(5215F9, InfantryClass_UpdateDeploy, 6)
{
	GET(TechnoClass *, I, ESI);
	return TechnoTypeExt::ExtMap.Find(I->GetTechnoType())->Is_Deso ? 0x5216B6 : 0x52160D;
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
	bool CanAffect = true;

	WarheadTypeExt::ExtData *WHTypeExt = WarheadTypeExt::ExtMap.Find(pThis);

	if(Arguments->Attacker) {
		/* Ren, 08.01.2011:
			Just documenting how/why this works, since I just stared at it for ten minutes trying to
			figure out why this makes sense:
			
			AffectsEnemies=yes is the default case (weapons usually affect the enemy), so if that's true,
			we can affect the target (this might later be changed by AffectsAllies).
			AffectsEnemies=no means we cannot attack enemies, i.e. we can only attack allies;
			ergo, CanAffect is only true if victim and target are allied.
			Thanks to lazy evaluation, the latter condition is only ever checked if AffectsEnemies is set to no, which is not the default.
			
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
		CanAffect = WHTypeExt->AffectsEnemies || Victim->Owner->IsAlliedWith(Arguments->Attacker->Owner);

		if(Arguments->Attacker->Owner != Arguments->SourceHouse) {
			Debug::Log("Info: During AffectsEnemies parsing, Attacker's Owner was %p [%s], but SourceHouse was %p [%s].",
				Arguments->Attacker->Owner,
				(Arguments->Attacker->Owner ? Arguments->Attacker->Owner->Type->ID : "null"),
				Arguments->SourceHouse,
				(Arguments->SourceHouse ? Arguments->SourceHouse->Type->ID : "null")
				);
			Debug::DumpStack(R, 0x180, 0xC0);
		}

	} else if(Arguments->SourceHouse) {
		// fallback, in case future ways of damage dealing don't include an attacker, e.g. stuff like GenericWarhead
		CanAffect = WHTypeExt->AffectsEnemies || Victim->Owner->IsAlliedWith(Arguments->SourceHouse);

	} else {
		//Debug::Log("Warning: Neither Attacker nor SourceHouse were set during AffectsEnemies parsing!");
		// this is often the case for ownerless damage i.e. crates/fire particles, no point in logging it
	}

	return CanAffect ? 0 : 0x701CC2;
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
DEFINE_HOOK(70E2D2, TechnoClass_IronCurtain_Modify, 6) {
	GET(TechnoClass*, pThis, ECX);
	GET(int, duration, EDX);
	GET_STACK(bool, force, 0x1C);

	// if it's no force shield then it's the iron curtain.
	if(!force) {
		if(TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())) {
			duration = (int)(duration * pData->IC_Modifier);
		}

		pThis->IronCurtainTimer.TimeLeft = duration;
		pThis->IronTintStage = 0;

		return 0x70E2DB;
	}

	return 0;
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
	if(TechnoExt->RadarJam) {
		TechnoExt->RadarJam->UnjamAll();
		delete TechnoExt->RadarJam;
		TechnoExt->RadarJam = NULL;
	}
	
	// #617 powered units
	if(TechnoExt->PoweredUnit)
	{
		delete TechnoExt->PoweredUnit;
		TechnoExt->PoweredUnit = NULL;
	}

	return 0;
}
