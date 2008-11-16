#include "TechnoExt.h"
#include "TechnoTypeExt.h"

EXT_P_DEFINE(TechnoClass);

EXT_CTOR(TechnoClass)
{
	if(!CONTAINS(Ext_p, pThis)) {
		ALLOC(ExtData, pData);

		pData->idxSlot_Wave = 0;
		pData->idxSlot_Beam = 0;
		pData->idxSlot_Warp = 0;

		pData->CloakSkipTimer.Stop();

		pData->Insignia_Image = NULL;

		Ext_p[pThis] = pData;
	}
}

EXT_DTOR(TechnoClass)
{
	if(CONTAINS(Ext_p, pThis)) {
		DEALLOC(Ext_p, pThis);
	}
}

EXT_LOAD(TechnoClass)
{
	if(CONTAINS(Ext_p, pThis)) {
		Create(pThis);

		ULONG out;
		pStm->Read(&Ext_p[pThis], sizeof(ExtData), &out);
	}
}

EXT_SAVE(TechnoClass)
{
	if(CONTAINS(Ext_p, pThis)) {
		ULONG out;
		pStm->Write(&Ext_p[pThis], sizeof(ExtData), &out);
	}
}

void TechnoClassExt::SpawnSurvivors(TechnoClass *pThis, TechnoClass *pKiller, bool Select)
{
	TechnoTypeClass *Type = pThis->GetTechnoType();

	HouseClass *pOwner = pThis->get_Owner();
	TechnoTypeClassExt::TechnoTypeClassData *pData = TechnoTypeClassExt::Ext_p[Type];
	RETZ_UNLESS(pData->Survivors_PilotChance || pData->Survivors_PassengerChance);

	CoordStruct loc = *pThis->get_Location();

	int chance = pData->Survivors_PilotChance;
	if(chance && Randomizer::Global()->RandomRanged(1, 100) <= chance) {
		InfantryTypeClass *PilotType = pData->Survivors_Pilots[pOwner->get_SideIndex()];
		if(PilotType) {
			InfantryClass *Pilot = new InfantryClass(PilotType, pOwner);

			Pilot->set_Health(PilotType->get_Strength() >> 1);
			Pilot->get_Veterancy()->Veterancy = pThis->get_Veterancy()->Veterancy;
			CoordStruct tmpLoc = loc;
			CellStruct tmpCoords = CellSpread::GetCell(Randomizer::Global()->RandomRanged(0, 7));
			tmpLoc.X += tmpCoords.X * 144;
			tmpLoc.Y += tmpCoords.Y * 144;

			if(!TechnoClassExt::ParadropSurvivor(Pilot, &tmpLoc, Select)) {
				Pilot->RegisterDestruction(pKiller); //(TechnoClass *)R->get_StackVar32(0x54));
				delete Pilot;
			}
		}
	}

	chance = pData->Survivors_PassengerChance;
	while(pThis->get_Passengers()->FirstPassenger) {
		FootClass *passenger;
		bool toDelete = 1;
		passenger = pThis->get_Passengers()->RemoveFirstPassenger();
		if(chance) {
			if(Randomizer::Global()->RandomRanged(1, 100) <= chance) {
				CoordStruct tmpLoc = loc;
				CellStruct tmpCoords = CellSpread::GetCell(Randomizer::Global()->RandomRanged(0, 7));
				tmpLoc.X += tmpCoords.X * 128;
				tmpLoc.Y += tmpCoords.Y * 128;
				toDelete = !TechnoClassExt::ParadropSurvivor(passenger, &tmpLoc, Select);
			}
		}
		if(toDelete) {
			passenger->RegisterDestruction(pKiller); //(TechnoClass *)R->get_StackVar32(0x54));
			passenger->UnInit();
		}
	}
}

bool TechnoClassExt::ParadropSurvivor(FootClass *Survivor, CoordStruct *loc, bool Select)
{
	bool success;
	int floorZ = MapClass::Global()->GetCellFloorHeight(loc);
	if(loc->Z > floorZ) {
		success = Survivor->SpawnParachuted(loc);
	} else {
		success = Survivor->Put(loc, Randomizer::Global()->RandomRanged(0, 7));
	}
	RET_UNLESS(success);
	Survivor->Scatter(0xB1CFE8, 1, 0);
	Survivor->QueueMission(Survivor->get_Owner()->ControlledByHuman() ? mission_Guard : mission_Hunt, 0);
	if(Select) {
		Survivor->Select();
	}
	return 1;
	// TODO: Tag
}

// bugfix #297: Crewed=yes jumpjets spawn parachuted infantry on destruction, not idle
// 7381A9, 6
EXPORT_FUNC(UnitClass_ReceiveDamage)
{
	GET(TechnoClass *, t, ESI);

	TechnoClassExt::SpawnSurvivors(t, (TechnoClass *)R->get_StackVar32(0x54), !!R->get_StackVar8(0x13));

	return 0x73838A;
}

// bugfix #297: Crewed=yes AircraftTypes spawn parachuting infantry on death
// 41668B, 6
EXPORT_FUNC(AircraftClass_ReceiveDamage)
{
	GET(AircraftClass *, a, ESI);

	TechnoClass *Killer = (TechnoClass *)R->get_StackVar32(0x2C);
	bool select = a->get_IsSelected() && a->get_Owner()->ControlledByPlayer();
	TechnoClassExt::SpawnSurvivors(a, Killer, select);

	return 0;
}

// 6F9E50, 5
EXPORT_FUNC(TechnoClass_Update)
{
	GET(TechnoClass *, Source, ECX);

	RET_UNLESS(CONTAINS(TechnoClassExt::Ext_p, Source));
	TechnoClassExt::TechnoClassData *pData = TechnoClassExt::Ext_p[Source];

	if(pData->CloakSkipTimer.IsDone()) {
		pData->CloakSkipTimer.Stop();
		Source->set_Cloakable(Source->GetTechnoType()->get_Cloakable());
	} else if(pData->CloakSkipTimer.GetTimeLeft() > 0) {
		Source->set_Cloakable(0);
	}
	return 0;
}

// 415CA6, 6
// fix for vehicle paradrop alignment
EXPORT_FUNC(AircraftClass_Paradrop)
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

// 6F407D, 6
EXPORT_FUNC(TechnoClass_Init)
{
	GET(TechnoClass *, T, ESI);
	TechnoClassExt::TechnoClassData *pData = TechnoClassExt::Ext_p[T];
	TechnoTypeClassExt::TechnoTypeClassData *pTypeData = TechnoTypeClassExt::Ext_p[T->GetTechnoType()];

	CaptureManagerClass *Capturer = NULL;
	ParasiteClass *Parasite = NULL;
	TemporalClass *Temporal = NULL;
	bool IsFoot = (T->get_AbstractFlags() & ABSFLAGS_ISFOOT) != 0;
	FootClass *F = (FootClass *)T;

	for(int i = 0; i < pTypeData->Weapons.get_Count(); ++i) {
		WeaponStruct *W = &pTypeData->Weapons[i];
		if(!W || !W->WeaponType) { continue; }
		WarheadTypeClass *WH = W->WeaponType->get_Warhead();
		if(WH->get_MindControl() && Capturer == NULL) {
			Capturer = new CaptureManagerClass(T, W->WeaponType->get_Damage(), W->WeaponType->get_InfiniteMindControl());
		} else if(WH->get_Temporal() && Temporal == NULL) {
			Temporal = new TemporalClass(T);
			pData->idxSlot_Warp = (BYTE)i;
		} else if(WH->get_Parasite() && IsFoot && Parasite == NULL) {
			Parasite = new ParasiteClass(F);
		}
	}

	T->set_CaptureManager(Capturer);
	T->set_TemporalImUsing(Temporal);
	if(IsFoot) {
		F->set_ParasiteImUsing(Parasite);
	}

	return 0x6F41C0;
}

// 71A860, 6
// temporal per-slot
EXPORT_FUNC(TemporalClass_UpdateA)
{
	GET(TemporalClass *, Temp, ESI);
	TechnoClass *T = Temp->get_Owner();
	TechnoClassExt::TechnoClassData *pData = TechnoClassExt::Ext_p[T];
	WeaponStruct *W = T->GetWeapon(pData->idxSlot_Warp);
	R->set_EAX((DWORD)W);
	return 0x71A876;
}

// 71AB30, 6
// temporal per-slot
EXPORT_FUNC(TemporalClass_GetHelperDamage)
{
	GET(TemporalClass *, Temp, ESI);
	TechnoClass *T = Temp->get_Owner();
	TechnoClassExt::TechnoClassData *pData = TechnoClassExt::Ext_p[T];
	WeaponStruct *W = T->GetWeapon(pData->idxSlot_Warp);
	R->set_EAX((DWORD)W);
	return 0x71AB47;
}

DEFINE_HOOK(6F3330, TechnoClass_SelectWeapon, 5)
{
	TechnoClass * pThis = (TechnoClass *)R->get_ECX();
	TechnoClass * pTarg = (TechnoClass *)R->get_StackVar32(0x4);
	
	DWORD Selected = TechnoClassExt::SelectWeaponAgainst(pThis, pTarg);
	R->set_EAX(Selected);
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

DEFINE_HOOK(70A9BB, Insignia_1, 5)
{
	GET(TechnoClass *, T, ESI);
	TechnoClassExt::TechnoClassData *pData = TechnoClassExt::Ext_p[T];
	TechnoTypeClassExt::TechnoTypeClassData *pTypeData = TechnoTypeClassExt::Ext_p[T->GetTechnoType()];

	if(pTypeData->Insignia_V) {
		R->set_EBX(0);
		pData->Insignia_Image = pTypeData->Insignia_V;
	} else {
		R->set_EBX(0xE);
	}
	return 0x70A9C0;
}

DEFINE_HOOK(70A9CB, Insignia_2, 5)
{
	GET(TechnoClass *, T, ESI);
	TechnoClassExt::TechnoClassData *pData = TechnoClassExt::Ext_p[T];
	TechnoTypeClassExt::TechnoTypeClassData *pTypeData = TechnoTypeClassExt::Ext_p[T->GetTechnoType()];

	if(pTypeData->Insignia_E) {
		R->set_EBX(0);
		pData->Insignia_Image = pTypeData->Insignia_E;
	} else {
		R->set_EBX(0xF);
	}
	return 0x70A9D0;
}

DEFINE_HOOK(70A9E2, Insignia_3, 5)
{
	GET(TechnoClass *, T, ESI);
	TechnoClassExt::TechnoClassData *pData = TechnoClassExt::Ext_p[T];
	TechnoTypeClassExt::TechnoTypeClassData *pTypeData = TechnoTypeClassExt::Ext_p[T->GetTechnoType()];

	if(pTypeData->Insignia_R) {
		R->set_EBX(0);
		pData->Insignia_Image = pTypeData->Insignia_R;
	} else {
		R->set_EBX(0xFFFFFFFF);
	}
	return 0;
}

DEFINE_HOOK(70A9BB, Insignia_4, 5)
{
	GET(TechnoClass *, T, ESI);
	TechnoClassExt::TechnoClassData *pData = TechnoClassExt::Ext_p[T];

	if(pData->Insignia_Image) {
		R->set_EBP((DWORD)pData->Insignia_Image);
	}
	return 0;
}

