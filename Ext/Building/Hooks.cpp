#include "Body.h"
#include "../BuildingType/Body.h"
#include "../Techno/Body.h"
#include "../../Misc/Network.h"

#include <SpecificStructures.h>
#include <ScenarioClass.h>
#include <InfantryClass.h>
#include <CellClass.h>

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
	R->EAX<SHPStruct *>(Type->Cameo);
	R->EDX<ConvertClass *>(FileSystem::CAMEO_PAL);
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

/* 	#218 - specific occupiers // comes in 0.2
	#665 - raidable buildings */
DEFINE_HOOK(457D58, BuildingClass_CanBeOccupied_SpecificOccupiers, 6)
{
	GET(BuildingClass *, pThis, ESI);
	GET(InfantryClass *, pInf, EDI);
	BuildingTypeExt::ExtData* pBuildTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
	bool can_occupy = false;

	if(pInf->Type->Occupier) {
		bool isFull = (pThis->GetOccupantCount() == pThis->Type->MaxNumberOccupants);
		bool isEmpty = (pThis->GetOccupantCount() == 0); // yes, yes, !pThis->GetOccupantCount() - leave it this way for semantics :P
		bool isIneligible = (pThis->IsRedHP() || pInf->IsMindControlled());

		if(!isFull && !isIneligible) {
			if(pThis->Owner != pInf->Owner) {
				can_occupy = (pThis->Owner->Type->MultiplayPassive || (pBuildTypeExt->BunkerRaidable && isEmpty)); // The building switches owner after the first occupant enters, so this check should not interfere with the player who captured it, only prevent others from entering it while it's occupied. (Bug #699)
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
// moved to before the survivors get unlimboed, per sanity's requirements
//A_FINE_HOOK(44266B, BuildingClass_ReceiveDamage_AfterPreDeathSequence, 6)
DEFINE_HOOK(441F12, BuildingClass_Destroy_RubbleYell, 6)
{
	GET(BuildingClass *, pThis, ESI);
	BuildingExt::ExtData* BuildingAresData = BuildingExt::ExtMap.Find(pThis);
	BuildingTypeExt::ExtData* destrBuildTE = BuildingTypeExt::ExtMap.Find(pThis->Type);

	Debug::Log("Destroying trench\n");
	if(!pThis->C4Timer.Ignorable()) {
		Debug::Log("C4 Done\n");
		// If this object has a rubble building set, turn, otherwise die
		if(destrBuildTE->RubbleDestroyed) {
			Debug::Log("Rubble destroyed\n");
			BuildingAresData->RubbleYell();
		} else {
			Debug::Log("UnInit\n");
			pThis->UnInit();
			pThis->AfterDestruction();
		}
	}

	/* original code
	if(pThis->C4Timer.IsDone()) {
		pThis->UnInit();
		pThis->AfterDestruction();
	}*/
	return 0;
}

// #666: Trench Traversal - check if traversal is possible & cursor display
DEFINE_HOOK(44725F, BuildingClass_GetCursorOverObject_TargetABuilding, 5)
{
	GET(BuildingClass *, pThis, ESI);
	GET(TechnoClass *, T, EBP);
	// not decided on UI handling yet

	if(T->WhatAmI() == abs_Building) {
		BuildingClass* targetBuilding = specific_cast<BuildingClass *>(T);
		BuildingExt::ExtData* curBuildExt = BuildingExt::ExtMap.Find(pThis);

		if(curBuildExt->canTraverseTo(targetBuilding)) {
			//show entry cursor, hooked up to traversal logic in Misc/Network.cpp -> AresNetEvent::Handlers::RespondToTrenchRedirectClick
			R->EAX<eAction>(act_Enter);
			return 0x447273;
		}
	}

	return 0;
}

DEFINE_HOOK(443414, BuildingClass_ClickedMission, 6)
{
	GET(eAction, Action, EAX);
	GET(BuildingClass *, pThis, ECX);

	GET_STACK(ObjectClass *, pTarget, 0x8);

	if(Action == act_Enter) {
		if(BuildingClass *pTargetBuilding = specific_cast<BuildingClass *>(pTarget)) {
			CoordStruct XYZ;
			pTargetBuilding->GetCoords(&XYZ);
			CellStruct tgt = { short(XYZ.X / 256), short(XYZ.Y / 256) };
			AresNetEvent::Handlers::RaiseTrenchRedirectClick(pThis, &tgt);
			R->EAX(1);
			return 0x44344D;
		}
	}

	return 0;
}

// #665: Raidable Buildings - prevent raided buildings from being sold while raided
DEFINE_HOOK(4494D2, BuildingClass_IsSellable, 6)
{
	GET(BuildingClass *, B, ESI);
	BuildingExt::ExtData* curBuildExt = BuildingExt::ExtMap.Find(B);

	enum SellValues {FORCE_SELLABLE, FORCE_UNSELLABLE, DECIDE_NORMALLY} sellTreatment;
	sellTreatment = DECIDE_NORMALLY; // default

	if(curBuildExt->isCurrentlyRaided) {
		sellTreatment = FORCE_UNSELLABLE; // enemy shouldn't be able to sell "borrowed" buildings
	}

	switch(sellTreatment) {
		case FORCE_SELLABLE:
			return 0x449532;
		case FORCE_UNSELLABLE:
			return 0x449536;
		case DECIDE_NORMALLY:
		default:
			return 0;
	}
}

/* Requested in issue #695
	Instructions from D:
	Flow:
	Occupier is Remove()'d from the map,
	added to the Occupants list,
	<-- hook happens here
	ThreatToCell is updated,
	"EVA_StructureGarrisoned" is played if applicable.
*/
DEFINE_HOOK(52297F, InfantryClass_GarrisonBuilding_OccupierEntered, 5)
{
	GET(InfantryClass *, pInf, ESI);
	GET(BuildingClass *, pBld, EBP);
	BuildingExt::ExtData* buildingExtData = BuildingExt::ExtMap.Find(pBld);
	TechnoExt::ExtData* infExtData = TechnoExt::ExtMap.Find(pInf);

	infExtData->GarrisonedIn = pBld;

	// if building and owner are from different players, and the building is not in raided state
	// change the building's owner and mark it as raided
	if((pBld->Owner != pInf->Owner) && !buildingExtData->isCurrentlyRaided) {
		buildingExtData->OwnerBeforeRaid = pBld->Owner;
		buildingExtData->isCurrentlyRaided = true;
		pBld->SetOwningHouse(pInf->Owner);
	}
	return 0;
}

/* Requested in issue #694
	D: The first hook fires each time one of the occupants is ejected through the Deploy function -
	the game doesn't have a builtin way to remove a single occupant, only all of them, so this is rigged inside that.*/
// This is CURRENTLY UNUSED - look at Misc/Network.cpp -> AresNetEvent::Handlers::RespondToTrenchRedirectClick
/*A_FINE_HOOK(4580A9, BuildingClass_UnloadOccupants_EachOccupantLeaves, 6)
{
	GET(BuildingClass *, pBld, ESI);
	GET(int, idxOccupant, EBP);

	TechnoExt::ExtData* infExtData = TechnoExt::ExtMap.Find(pBld->Occupants[idxOccupant]);
	infExtData->GarrisonedIn = NULL;

    / *
    - get current rally point target; if there is none, exit trench
    - check if target cell has a building
		- if so, check if it's the same building
			- if so, do nothing
			- if not, check building with canTraverseTo
				- if true, doTraverseTo
				- if false, do nothing
		- if not, exit trench
    * /

    if(0/ * spawned in a different building * /) {
        return 0x45819D;
    }
    // do the normal kickout thing
    return 0;
}*/

/* Requested in issue #694
	D: The second hook fires each time one of the occupants is killed (Assaulter). Note that it doesn't catch the damage forwarding fatal hit.
*/
DEFINE_HOOK(4586CA, BuildingClass_KillOccupiers_EachOccupierKilled, 6)
{
    GET(BuildingClass *, pBld, ESI);
    GET(int, idxOccupant, EDI);
    // I don't think anyone ever actually tested Assaulter=yes with raiding, putting this here 'cause it's likely needed
	BuildingExt::ExtData* buildingExtData = BuildingExt::ExtMap.Find(pBld);
    buildingExtData->evalRaidStatus();

    return 0;
}

/* Requested in issue #694
	D: The third hook fires after all the occupants have been ejected (by the first hook).
*/
DEFINE_HOOK(4581CD, BuildingClass_UnloadOccupants_AllOccupantsHaveLeft, 6)
{
    GET(BuildingClass *, pBld, ESI);
    BuildingExt::ExtData* buildingExtData = BuildingExt::ExtMap.Find(pBld);

    buildingExtData->evalRaidStatus();

    return 0;
}

/* Requested in issue #694
	D: The fourth hook fires after all the occupants have been killed by the second hook.
*/
DEFINE_HOOK(458729, BuildingClass_KillOccupiers_AllOccupantsKilled, 6)
{
    GET(BuildingClass *, pBld, ESI);
	BuildingExt::ExtData* buildingExtData = BuildingExt::ExtMap.Find(pBld);

	buildingExtData->evalRaidStatus();

    return 0;
}

// #666: Trench Traversal - check if traversal is possible & traverse, eject or do nothing, depending on the result
// This is CURRENTLY UNUSED - look at Misc/Network.cpp -> AresNetEvent::Handlers::RespondToTrenchRedirectClick
A_FINE_HOOK(457DF5, BuildingClass_UnloadOccupants_AboutToStartUnloading, 6)
{
	GET(BuildingClass *, currentBuilding, ESI);
	/*CellClass* rallyPoint =; // wherever the rally point points to
	if(BuildingClass* targetBuilding = rallyPoint->GetBuilding()) {
		BuildingExt::ExtData* currentBuildingExt = BuildingExt::ExtMap.Find(currentBuilding);
		if(currentBuildingExt->canTraverseTo(targetBuilding)) {
			currentBuildingExt->doTraverseTo(targetBuilding); // also calls evalRaidStatus()
		} else {
			// see if we can find a way to abort eviction here
		}
	}

	*/
	return 0;
}
