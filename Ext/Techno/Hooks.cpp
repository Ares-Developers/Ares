#include "Body.h"
#include "../TechnoType/Body.h"

#include "../../Misc/Debug.h"

// bugfix #297: Crewed=yes jumpjets spawn parachuted infantry on destruction, not idle
DEFINE_HOOK(7381AE, UnitClass_ReceiveDamage, 6)
{
	GET(TechnoClass *, t, ESI);
	GET_STACK(TechnoClass *, Killer, 0x54);
	GET_STACK(bool, select, 0x13);

	TechnoExt::SpawnSurvivors(t, Killer, select);

	return 0x73838A;
}

// bugfix #297: Crewed=yes AircraftTypes spawn parachuting infantry on death
DEFINE_HOOK(41668B, AircraftClass_ReceiveDamage, 6)
{
	GET(AircraftClass *, a, ESI);

	GET_STACK(TechnoClass *, Killer, 0x28);
	bool select = a->IsSelected && a->Owner->ControlledByPlayer();
	TechnoExt::SpawnSurvivors(a, Killer, select);

	return 0;
}

DEFINE_HOOK(6F9E50, TechnoClass_Update, 5)
{
	GET(TechnoClass *, Source, ECX);

	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(Source);

	if(pData->CloakSkipTimer.IsDone()) {
		pData->CloakSkipTimer.Stop();
		Source->Cloakable = Source->GetTechnoType()->Cloakable;
	} else if(pData->CloakSkipTimer.GetTimeLeft() > 0) {
		Source->Cloakable = 0;
	}
	return 0;
}


//! TechnoClass::Update is called every frame; returning 0 tells it to execute the original function's code as well.
DEFINE_HOOK(6F9E76, TechnoClass_Update_CheckOperators, 6)
{
	GET(TechnoClass *, pThis, ESI); // object this is called on
	TechnoTypeClass *Type = pThis->GetTechnoType();
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Type); // gah...multilayered YR++ness!

	// Related to operators/drivers, issue #342
	BuildingClass * pTheBuildingBelow = pThis->GetCell()->GetBuilding();

	/* Conditions checked:
	- Is there no building below us
	OR
	- Is this the building on this cell AND is it online

	pTheBuildingBelow will be NULL if no building was found
	*/
	if(!pTheBuildingBelow || ((pTheBuildingBelow == pThis) && (pTheBuildingBelow->IsPowerOnline()))) {
		if(pTypeData->Operator != NULL) {
			if(pThis->Passengers.NumPassengers) {
				bool foundAnOperator = false;

				// loop & condition come from D
				for(ObjectClass* O = pThis->Passengers.GetFirstPassenger(); O; O = O->NextObject) {
					if(ABS_IS_FOOT(O) && O->GetType() == pTypeData->Operator) {
						foundAnOperator = true;
						break;
					}
				}

				if(foundAnOperator) {
					// takes a specific operator and someone is present AND that someone is the operator, so it stays active/gets reactivated
					if(pThis->Deactivated) pThis->Reactivate();
				} else {
					// takes a specific operator and someone is present, but it's not the operator, so it stays deactivated/gets deactivated
					if(!pThis->Deactivated) pThis->Deactivate();
				}
			} else {
				// takes a specific operator but no one is present, so it stays deactivated/gets deactivated
				if(!pThis->Deactivated) pThis->Deactivate();
			}
		} else if(pTypeData->IsAPromiscuousWhoreAndLetsAnyoneRideIt) {
			if(pThis->Passengers.NumPassengers) {
				// takes anyone and someone is present, so it stays active/gets reactivated
				if(pThis->Deactivated) pThis->Reactivate();
			} else {
				// takes anyone but no one is present, so it stays deactivated/gets deactivated
				if(!pThis->Deactivated) pThis->Deactivate();
			}
		}
	}

	//return 0x6F9E7C; // using this value instead makes this function override the original game one's entirely - don't activate this unless you handle *everything* originally handled by the game
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
	CoordStruct* XYZ = (CoordStruct *)R->lea_StackVar(0x20);
	XYZ->X = SrcXYZ.X & ~0x80;
	XYZ->Y = SrcXYZ.Y & ~0x80;
	XYZ->Z = SrcXYZ.Z - 1;
	R->set_ECX((DWORD)XYZ);
	return 0x415DE3;
}

DEFINE_HOOK(6F407D, TechnoClass_Init_1, 6)
{
	GET(TechnoClass *, T, ESI);
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(T);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());

	CaptureManagerClass *Capturer = NULL;
	ParasiteClass *Parasite = NULL;
	TemporalClass *Temporal = NULL;
	bool IsFoot = (T->AbstractFlags & ABSFLAGS_ISFOOT) != 0;
	FootClass *F = (FootClass *)T;

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
			_snprintf(Ares::readBuffer, Ares::readLength,
			"Constructing an instance of [%s]:\r\n%sWeapon %s (slot %d) has no Warhead!\n",
				T->get_ID(),
				WH1 ? "Elite " : "",
				(WH1 ? W2 : W1)->get_ID(),
				i);
			Debug::FatalError(Ares::readBuffer);
		}

		if(WH1 && WH1->MindControl && Capturer == NULL) {
			GAME_ALLOC(CaptureManagerClass, Capturer, T, W1->Damage, W1->InfiniteMindControl);
		} else if(WH2 && WH2->MindControl && Capturer == NULL) {
			GAME_ALLOC(CaptureManagerClass, Capturer, T, W2->Damage, W2->InfiniteMindControl);
		}

		if((WH1 && WH1->Temporal || WH2 && WH2->Temporal) && Temporal == NULL) {
			GAME_ALLOC(TemporalClass, Temporal, T);
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
	R->set_EAX((DWORD)W);
	return 0x71A876;
}

// temporal per-slot
DEFINE_HOOK(71AB30, TemporalClass_GetHelperDamage, 5)
{
	GET(TemporalClass *, Temp, ESI);
	TechnoClass *T = Temp->Owner;
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(T);
	WeaponStruct *W = T->GetWeapon(pData->idxSlot_Warp);
	R->set_EAX((DWORD)W);
	return 0x71AB47;
}

// parasite per-slot
DEFINE_HOOK(62A020, ParasiteClass_Update, A)
{
	GET(TechnoClass *, T, ECX);
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(T);
	WeaponStruct *W = T->GetWeapon(pData->idxSlot_Parasite);
	R->set_EAX((DWORD)W);
	return 0x62A02A;
}

DEFINE_HOOK(62A7B1, Parasite_ExitUnit, 9)
{
	GET(TechnoClass *, T, ECX);
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(T);
	WeaponStruct *W = T->GetWeapon(pData->idxSlot_Parasite);
	R->set_EAX((DWORD)W);
	return 0x62A7BA;
}

DEFINE_HOOK(629804, ParasiteClass_UpdateSquiddy, 9)
{
	GET(TechnoClass *, T, ECX);
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(T);
	WeaponStruct *W = T->GetWeapon(pData->idxSlot_Parasite);
	R->set_EAX((DWORD)W);
	return 0x62980D;
}


DEFINE_HOOK(6F3330, TechnoClass_SelectWeapon, 5)
{
	TechnoClass * pThis = (TechnoClass *)R->get_ECX();
	TechnoClass * pTarg = (TechnoClass *)R->get_StackVar32(0x4);

//	DWORD Selected = TechnoClassExt::SelectWeaponAgainst(pThis, pTarg);
//	R->set_EAX(Selected);
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

/* #604 - customizable parachutes */
DEFINE_HOOK(5F5ADD, Parachute_Animation, 6)
{
	GET(TechnoClass *, T, ESI);
	RET_UNLESS(ABS_IS_TECHNO(T));
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());
	if(pTypeData->Is_Bomb) {
		T->IsABomb = 1;
	}
	R->set_EDX((DWORD)pTypeData->Parachute_Anim.Get());
	return 0x5F5AE3;
}

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
	return pData->Is_Cow ? 0x51CEAE : 0x51CECD;
}

DEFINE_HOOK(747BBD, UnitTypeClass_LoadFromINI, 5)
{
	GET(UnitTypeClass *, U, ESI);

	U->set_AltImage((SHPStruct *)R->get_EAX()); // jumping over, so replicated
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

DEFINE_HOOK(73B672, UnitClass_DrawVXL, 6)
{
	GET(UnitClass *, U, EBP);
	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(U->GetTechnoType());
	if(pData->WaterAlt) {
		if(U->GetCell()->LandType == lt_Water) {
			R->set_EAX(0);
			return 0x73B68B;
		}
	}
	return 0;
}
