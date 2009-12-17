#include "Body.h"
#include "../BuildingType/Body.h"
#include <SpecificStructures.h>
#include <ScenarioClass.h>
#include <InfantryClass.h>
#include <cmath>

/* #633 - spy building infiltration */
DEFINE_HOOK(4571E0, BuildingClass_Infiltrate, 5)
{
	GET(BuildingClass *, EnteredBuilding, ECX);
	GET_STACK(HouseClass *, EnteredBy, 0x4);

	bool apply_normal_infiltration_logic = 1;

	//! RA1-Style Spying, as requested in issue #633
	//! This sets the respective bit to inform the game that a particular house has spied this building.
	//! Knowing that, the game will reveal the current production in this building to the players who have spied it.
	//! In practice, this means: If a player who has spied a factory clicks on that factory, he will see the cameo of whatever is being built in the factory.
	EnteredBuilding->DisplayProductionTo.Add(EnteredBy);

	// also note that the slow radar reparse call (MapClass::sub_4F42D0()) is not made here, meaning if you enter a radar, there will be a discrepancy between what you see on the map/tact map and what the game thinks you see

	// wrapper around the entire function, so return 0 or handle _every_ single thing
	return (apply_normal_infiltration_logic) ? 0 : 0x45759F;
}

// check before drawing the tooltip
DEFINE_HOOK(43E7EF, BuildingClass_DrawVisible_P1, 5)
{
	GET(BuildingClass *, B, ESI);
	return B->DisplayProductionTo.Contains(HouseClass::Player) ? 0x43E80E : 0x43E832;
}

// check before drawing production cameo
DEFINE_HOOK(43E832, BuildingClass_DrawVisible_P2, 6)
{
	GET(BuildingClass *, B, ESI);
	return B->DisplayProductionTo.Contains(HouseClass::Player) ? 0x43E856 : 0x43E8EC;
}

// fix palette for spied factory production cameo drawing
DEFINE_HOOK(43E8D1, BuildingClass_DrawVisible_P3, 8)
{
	GET(TechnoTypeClass *, Type, EAX);
	R->SetEx_EAX<SHPStruct *>(Type->Cameo);
	R->SetEx_EDX<ConvertClass *>(FileSystem::CAMEO_PAL);
	return 0x43E8DF;
}

// if this is a radar, change the owner's house bitfields responsible for radar reveals
DEFINE_HOOK(44161C, BuildingClass_Destroy_OldSpy1, 6)
{
	GET(BuildingClass *, B, ESI);
	B->DisplayProductionTo.Clear();
	BuildingExt::UpdateDisplayTo(B);
	return 0x4416A2;
}

// if this is a radar, change the owner's house bitfields responsible for radar reveals
DEFINE_HOOK(448312, BuildingClass_ChangeOwnership_OldSpy1, a)
{
	GET(HouseClass *, newOwner, EBX);
	GET(BuildingClass *, B, ESI);

	if(B->DisplayProductionTo.Contains(newOwner)) {
		B->DisplayProductionTo.Remove(newOwner);
		BuildingExt::UpdateDisplayTo(B);
	}
	return 0x4483A0;
}

// if this is a radar, drop the new owner from the bitfield
DEFINE_HOOK(448D95, BuildingClass_ChangeOwnership_OldSpy2, 8)
{
	GET(HouseClass *, newOwner, EDI);
	GET(BuildingClass *, B, ESI);

	if(B->DisplayProductionTo.Contains(newOwner)) {
		B->DisplayProductionTo.Remove(newOwner);
	}

	return 0x448DB9;
}

DEFINE_HOOK(44F7A0, BuildingClass_UpdateDisplayTo, 0)
{
	GET(BuildingClass *, B, ECX);
	BuildingExt::UpdateDisplayTo(B);
	return 0x44F813;
}

DEFINE_HOOK(509303, HouseClass_AllyWith_unused, 0)
{
	GET(HouseClass *, pThis, ESI);
	GET(HouseClass *, pThat, EAX);

	pThis->RadarVisibleTo.Add(pThat);
	return 0x509319;
}

DEFINE_HOOK(56757F, MapClass_RevealArea0_DisplayTo, 0)
{
	GET(HouseClass *, pThis, ESI);
	GET(HouseClass *, pThat, EAX);

	return pThis->RadarVisibleTo.Contains(pThat)
	 ? 0x567597
	 : 0x56759D
	;
}

DEFINE_HOOK(567AC1, MapClass_RevealArea1_DisplayTo, 0)
{
	GET(HouseClass *, pThis, EBX);
	GET(HouseClass *, pThat, EAX);

	return pThis->RadarVisibleTo.Contains(pThat)
	 ? 0x567AD9
	 : 0x567ADF
	;
}


/* #221 - Trenches, subissue #663: Forward damage to occupants in UC buildings and Battle Bunkers  */
// building receives damage // this was moved to BulletExt::ExtData::DamageOccupants() which is executed in WarheadType hook BulletClass_Fire
A_FINE_HOOK(44235E, BuildingClass_ReceiveDamage_Trenches, 6)
{
	GET(BuildingClass *, Building, ESI);
	LEA_STACK(args_ReceiveDamage *, Arguments, 0xA0);
	//BuildingTypeClass *BuildingType = Building->Type;
	BuildingTypeExt::ExtData *BuildingAresData = BuildingTypeExt::ExtMap.Find(Building->Type);

	if(Building->Occupants.Count && BuildingAresData->UCPassThrough) { // only work when UCPassThrough is set, as per community vote in thread #1392
		if(false || ((ScenarioClass::Instance->Random.RandomRanged(0, 99) / 100) < BuildingAresData->UCPassThrough)) { //\todo replace false with !SubjectToTrenches once we can access that
			int poorBastard = ScenarioClass::Instance->Random.RandomRanged(0, Building->Occupants.Count - 1); // which Occupant is getting it?
			if(BuildingAresData->UCFatalRate && ((ScenarioClass::Instance->Random.RandomRanged(0, 99) / 100) < BuildingAresData->UCFatalRate)) {
				// fatal hit
				Building->Occupants[poorBastard]->Destroyed(Arguments->Attacker); // ReceiveDamage lists 4th argument as "pAttacker", and given that I call ReceiveDamage on this unit, it's kind of obvious who the target is...maybe wrongly labeled in args_ReceiveDamage?
			} else {
				// just a flesh wound
				*Arguments->Damage = static_cast<int> (ceil(*Arguments->Damage * BuildingAresData->UCDamageMultiplier));
				Building->Occupants[poorBastard]->ReceiveDamage(Arguments->Damage, Arguments->DistanceToEpicenter, Arguments->WH, Arguments->Attacker, Arguments->IgnoreDefenses, Arguments->PreventsPassengerEscape, Arguments->SourceHouse);
			}

			return 0x442373;
		}
	}

	// continue normally
	return 0;

	/*!
		\return 0: proceed to Building->ReceiveDamage
		\return 0x442373: the building will not call technoclass::receivedamage or anything else , as if the hit hadn't happened at all
		*/

	/* yes, I'm gonna jinx it and say: This should never be reached.
		don't jinx my compiler kthx
		Debug::Log("Warning: Safety case reached in BuildingClass_ReceiveDamage_Trenches.\nTarget: %p\nSource House: %p\nWarhead: %p\nDamage: %i\nExecuting default ReceiveDamage() handler...", Arguments->target, Arguments->SourceHouse, Arguments->WH, *Arguments->Damage);
		return 0;
	*/
}

/* 	#218 - specific occupiers // comes in 0.2
	#665 - raidable buildings */
DEFINE_HOOK(457D58, BuildingClass_CanBeOccupied_SpecificOccupiers, 6)
{
	GET(BuildingClass *, pThis, ESI);
	GET(InfantryClass *, pInf, EDI);
	BuildingClassExt* pBuildExt = BuildingClassExt::ExtMap.Find(pThis);
	bool can_occupy = false;

	if(pInf->Occupier) {
		bool isFull = (pThis->GetOccupantCount() == pThis->BuildingType->MaxNumberOccupants);
		bool isIneligible = (isFull || pThis->IsRedHP() || pInf->IsMindControlled());

		if(!isFull && !isIneligible) {
			if(pThis->Owner != pInf->Owner) {
				can_occupy = (pThis->Owner->Type->MultiplayPassive || pBuildExt->BunkerRaidable);
			} else {
				can_occupy = true;
			}
		}
	}

/*
// original code replaced by this hook
	if(pInf->Occupier) {
		if ( pThis->Owner != pInf->Owner && !pThis->Owner->Country->MultiplayPassive
			|| pThis->GetOccupantCount() == pThis->BuildingType->MaxNumberOccupants
			|| pThis->IsRedHP()
			|| pInf->IsMindControlled() )
			return 0;
	}
	return 1;
*/

	return can_occupy ? 0x457DD5 : 0x457DA3;
}


// #664: Advanced Rubble - turning into rubble part
DEFINE_HOOK(44266B, BuildingClass_ReceiveDamage_AfterPreDeathSequence, 6)
{
	GET(BuildingClass *, pThis, ESI);
	BuildingExt::ExtData* BuildingAresData = BuildingExt::ExtMap.Find(pThis);
	BuildingTypeExt::ExtData* destrBuildTE = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if(pThis->C4Timer.IsDone()) {
		// If this object has a rubble building set, turn, otherwise die
		if(destrBuildTE->RubbleDestroyed) {
			BuildingAresData->RubbleYell();
		} else {
			pThis->UnInit();
			pThis->AfterDestruction();
		}
	}

	/* original code
	if(pThis->C4Timer.IsDone()) {
		pThis->UnInit();
		pThis->AfterDestruction();
	}*/
	return 0x442905;
}

// #666: Trench Traversal - check if traversal is possible & cursor display
DEFINE_HOOK(44725F, BuildingClass_GetCursorOverObject_TargetABuilding, 5)
{
	GET(BuildingClass *, pThis, ECX);
	GET(TechnoClass *, T, EBP);
	// not decided on UI handling yet

	if(T->WhatAmI() == abs_Building) {
		BuildingClass* targetBuilding = game_cast<BuildingClass *>(T);
		BuildingExt::ExtData* curBuildExt = BuildingExt::ExtMap.Find(pThis);

		if(curBuildExt->canTraverseTo(targetBuilding)) {
			//! \todo show entry cursor, hooked up to traversal logic
		}
	}

	return 0;
}

// #664: Advanced Rubble - prevent rubble from being sold, ever
A_FINE_HOOK(4494D2, BuildingClass_IsSellable, 6)
{
	GET(BuildingClass *, B, ESI);
	switch(decision) {
		case Yes:
			return 0x449532;
		case No:
			return 0x449536;
		case DecideNormally:
		default:
			return 0;
	}
}
