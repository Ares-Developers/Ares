#include "Body.h"
#include "../BuildingType/Body.h"
#include "../Techno/Body.h"
#include "../TechnoType/Body.h"
#include "../../Misc/Network.h"

#include <SpecificStructures.h>
#include <ScenarioClass.h>
#include <InfantryClass.h>
#include <CellClass.h>
#include <HouseClass.h>

#include <cmath>

/* 	#218 - specific occupiers
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
		bool isIneligible = ((pThis->IsRedHP() && pThis->Type->TechLevel == -1) || pInf->IsMindControlled()); // 
		bool isNeutral = pThis->Owner->IsNeutral();
		bool isRaidable = (pBuildTypeExt->BunkerRaidable && isEmpty); // if it's not empty, it cannot be raided anymore, 'cause it already was
		bool sameOwner = (pThis->Owner == pInf->Owner);

		bool allowedOccupier = pBuildTypeExt->CanBeOccupiedBy(pInf);

		if(!isFull && !isIneligible && allowedOccupier) {
		/*	The building switches owners after the first occupant enters,
			so this check should not interfere with the player who captured it,
			only prevent others from entering it while it's occupied. (Bug #699) */
			can_occupy = sameOwner ? true : (isNeutral || isRaidable);
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

// semi-specific assaulters by customizable level
DEFINE_HOOK(457DB7, BuildingClass_CanBeOccupied_SpecificAssaulters, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	GET(InfantryClass* const, pInfantry, EDI);

	auto const isAssaultable = !pThis->Owner->IsAlliedWith(pInfantry)
		&& pThis->GetOccupantCount() > 0;

	if(isAssaultable) {
		auto const pBldExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

		// buildings with negative level are not assaultable
		if(pBldExt->AssaulterLevel >= 0) {
			auto const pInfExt = TechnoTypeExt::ExtMap.Find(pInfantry->Type);

			// assaultable if infantry has same level or more
			if(pBldExt->AssaulterLevel <= pInfExt->AssaulterLevel) {
				return 0x457DD5;
			}
		}
	}

	return 0x457DA3;
}

// #664: Advanced Rubble - turning into rubble part
// moved to before the survivors get unlimboed, per sanity's requirements
// TODO review
DEFINE_HOOK(441F12, BuildingClass_Destroy_RubbleYell, 6)
{
	GET(BuildingClass *, pThis, ESI);
	BuildingExt::ExtData* BuildingAresData = BuildingExt::ExtMap.Find(pThis);
	BuildingTypeExt::ExtData* destrBuildTE = BuildingTypeExt::ExtMap.Find(pThis->Type);

	// If this object has a rubble building set, turn
	if(destrBuildTE->RubbleDestroyed || destrBuildTE->RubbleDestroyedRemove) {
		++Unsorted::IKnowWhatImDoing;
		BuildingAresData->RubbleYell();
		--Unsorted::IKnowWhatImDoing;
	}

	return 0;
}

// remove all units from the rubble
DEFINE_HOOK(441F2C, BuildingClass_Destroy_KickOutOfRubble, 5) {
	GET(BuildingClass*, pBld, ESI);

	// find out whether this destroyed building would turn into rubble
	if(BuildingTypeExt::ExtData *pTData = BuildingTypeExt::ExtMap.Find(pBld->Type)) {
		if(pTData->RubbleDestroyed || pTData->RubbleDestroyedRemove) {
			if(BuildingExt::ExtData *pData = BuildingExt::ExtMap.Find(pBld)) {
				// this is not the rubble, but the old intact building.
				// since we force the same foundation this is no problem.
				pData->KickOutOfRubble();
			}
		}
	}

	return 0;
}

// #666: Trench Traversal - check if traversal is possible & cursor display
DEFINE_HOOK(44725F, BuildingClass_GetCursorOverObject_TargetABuilding, 5)
{
	GET(BuildingClass *, pThis, ESI);
	GET(TechnoClass *, T, EBP);
	// not decided on UI handling yet

	if(auto targetBuilding = abstract_cast<BuildingClass*>(T)) {
		BuildingExt::ExtData* curBuildExt = BuildingExt::ExtMap.Find(pThis);

		if(curBuildExt->canTraverseTo(targetBuilding)) {
			//show entry cursor, hooked up to traversal logic in Misc/Network.cpp -> AresNetEvent::Handlers::RespondToTrenchRedirectClick
			R->EAX(Action::Enter);
			return 0x447273;
		}
	}

	return 0;
}

DEFINE_HOOK(443414, BuildingClass_ClickedAction, 6)
{
	GET(enum class Action, Action, EAX);
	GET(BuildingClass *, pThis, ECX);

	GET_STACK(ObjectClass *, pTarget, 0x8);

	// part of deactivation logic
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if(pExt->IsDeactivated()) {
		R->EAX(1);
		return 0x44344D;
	}

	// trenches
	if(Action == Action::Enter) {
		if(BuildingClass *pTargetBuilding = specific_cast<BuildingClass *>(pTarget)) {
			CoordStruct XYZ = pTargetBuilding->GetCoords();
			CellStruct tgt = CellClass::Coord2Cell(XYZ);
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
	GET(BuildingClass *, pThis, ESI);
	auto curBuildExt = BuildingExt::ExtMap.Find(pThis);

	enum { Sellable = 0x449532, Unsellable = 0x449536, Undecided = 0 };

	// enemy shouldn't be able to sell "borrowed" buildings
	return curBuildExt->isCurrentlyRaided ? Unsellable : Undecided;
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
	// but only if that's even necessary - no need to raid urban combat buildings.
	// 27.11.2010 changed to include fix for #1305
	bool differentOwners = (pBld->Owner != pInf->Owner);
	bool ucBuilding = (pBld->Owner->IsNeutral() && (pBld->GetTechnoType()->TechLevel == -1));
	if(differentOwners && !ucBuilding && !buildingExtData->isCurrentlyRaided) {
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
	infExtData->GarrisonedIn = nullptr;

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
    //GET(int, idxOccupant, EDI);
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

/*
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


	return 0;
}
*/

/*	#698 - "Building captured" EVA announcement "bug"


	<DCoder> if you want the original handling (TechBuildingLost, BuildingCaptured, etc) to take place, return No,
	otherwise perform your own code and return Yes
	<Renegade> at what place in the chain is that executed?
	<DCoder> in the middle of where the building changes ownership from old player to new
*/
DEFINE_HOOK(448401, BuildingClass_ChangeOwnership_TrenchEVA, 6)
{
	GET(BuildingClass *, pBld, ESI);
	//GET(HouseClass *, pNewOwner, EBX);
	enum wasHandled { Yes = 0x44848F, No = 0} Handled = No;

	BuildingExt::ExtData* bldExt = BuildingExt::ExtMap.Find(pBld);

	if(bldExt->ignoreNextEVA) {
		Handled = Yes;
		bldExt->ignoreNextEVA = false;
	}

	return Handled;
}
