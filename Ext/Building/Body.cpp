#include "Body.h"
#include "../BuildingType/Body.h"
#include <BuildingClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <CellSpread.h>
#include <GeneralStructures.h> // for CellStruct

const DWORD Extension<BuildingClass>::Canary = 0x87654321;
Container<BuildingExt> BuildingExt::ExtMap;

BuildingClass *Container<BuildingExt>::SavingObject = NULL;
IStream *Container<BuildingExt>::SavingStream = NULL;

// =============================
// member functions

void BuildingExt::ExtendFirewall(BuildingClass *pThis, CellStruct Center, HouseClass *Owner) {
	BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if(!pData->Firewall_Is) {
		return;
	}
	int range = pThis->Type->GuardRange / 256;
	CoordStruct XYZ;
	for(int direction = 0; direction <= 7; direction += 2) {
		CellStruct offset = CellSpread::GetNeighbourOffset(direction);
		int fillRange = 0;
		for(int curRange = 1; curRange <= range; ++curRange) {
			CellStruct CurrentPos = Center;
			CurrentPos.X += short(curRange * offset.X);
			CurrentPos.Y += short(curRange * offset.Y);

			if(!MapClass::Global()->CellExists(&CurrentPos)) {
				break;
			}

			CellClass *cell = MapClass::Global()->GetCellAt(&CurrentPos);

			if(BuildingClass *OtherEnd = cell->GetBuilding()) {
				if(OtherEnd->Owner == Owner && OtherEnd->Type == pThis->Type) {
					fillRange = curRange - 1;
					break;
				}
			}

			if(!cell->CanThisExistHere(pThis->Type->SpeedType, pThis->Type, Owner)) {
				break;
			}

		}

		++Unsorted::SomeMutex;
		for(int curRange = fillRange; curRange > 0; --curRange) {
			CellStruct CurrentPos = Center;
			CurrentPos.X += short(curRange * offset.X);
			CurrentPos.Y += short(curRange * offset.Y);

			if(CellClass *cell = MapClass::Global()->GetCellAt(&CurrentPos)) {
				if(BuildingClass *dummy = specific_cast<BuildingClass *>(pThis->Type->CreateObject(Owner))) {
					CellClass::Cell2Coord(&CurrentPos, &XYZ);
					if(!dummy->Put(&XYZ, 0)) {
						delete dummy;
					}
				}
			}
		}
		--Unsorted::SomeMutex;
	}
}

DWORD BuildingExt::GetFirewallFlags(BuildingClass *pThis) {
	CellClass *MyCell = MapClass::Global()->GetCellAt(pThis->get_Location());
	DWORD flags = 0;
	for(int direction = 0; direction < 8; direction += 2) {
		CellClass *Neighbour = MyCell->GetNeighbourCell(direction);
		if(BuildingClass *B = Neighbour->GetBuilding()) {
			BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(B->Type);
			if(pTypeData->Firewall_Is && B->Owner == pThis->Owner && !B->InLimbo && B->IsAlive) {
				flags |= 1 << (direction >> 1);
			}
		}
	}
	return flags;
}

void BuildingExt::UpdateDisplayTo(BuildingClass *pThis) {
	if(pThis->Type->Radar) {
		HouseClass *H = pThis->Owner;
		H->RadarVisibleTo.Clear();
		for(int i = 0; i < H->Buildings.Count; ++i) {
			BuildingClass *currentB = H->Buildings.GetItem(i);
			if(!currentB->InLimbo && currentB->Type->Radar) {
				H->RadarVisibleTo.data |= currentB->DisplayProductionTo.data;
			}
		}
		MapClass::Instance->sub_4F42F0(2);
	}
}

// #664: Advanced Rubble
//! This function switches the building into its Advanced Rubble state or back to normal.
/*!
	If no respective target-state object exists in the ExtData so far, the function will create it.
	It'll check beforehand whether the necessary Rubble.Intact or Rubble.Destroyed are actually set on the type.
	If the necessary one is not set, it'll log a debug message and abort.

	Rubble is set to full health on placing, since, no matter what we do in the backend, to the player, each pile of rubble is a new one.
	The reconstructed building, on the other hand, is set to 1% of it's health, since it was just no reconstructed, and not freshly built or repaired yet.

	Lastly, the function will set the current building as the alternate state on the other state, to ensure that, if the other state gets triggered back,
	it is connected to this state. (e.g. when a building is turned into rubble, the normal state will set itself as the normal state of the rubble,
	so when the rubble is reconstructed, it switches back to the correct normal state.)

	\param beingRepaired if set to true, the function will turn the building back into its normal state. If false or unset, the building will be turned into rubble.

	\author Renegade
	\date 16.12.09+
*/
void BuildingExt::ExtData::RubbleYell(bool beingRepaired) {
	BuildingClass* currentBuilding = this->AttachedToObject;
	BuildingTypeClass* currentBuildingType = currentBuilding->Type;
	BuildingTypeExt::ExtData *pTypeData = BuildingTypeExt::ExtMap.Find(currentBuildingType);

	currentBuilding->Remove(); // only takes it off the map

	if(beingRepaired) {
		if(!this->NormalState && !pTypeData->RubbleIntact) {
			Debug::Log("Warning! Advanced Rubble was supposed to be reconstructed but Ares could not obtain its normal state. Check if Rubble.Intact is set (correctly).");
			return;
		}

		// if we don't have a normal state building yet, create one.
		if(!this->NormalState) {
			this->NormalState = specific_cast<BuildingClass *>(pTypeData->RubbleIntact->CreateObject(currentBuilding->Owner));
		}

		this->NormalState->Health = static_cast<int>(this->NormalState->Type->Strength / 100); // see description above
		this->NormalState->IsAlive = true; // assuming this is in the sense of "is not destroyed"
		// Location should not be changed by removal
		this->NormalState->Put(&currentBuilding->Location, currentBuilding->Facing);

		// make sure we get back here if necessary
		BuildingExt::ExtData* NormalExt = BuildingExt::ExtMap.Find(this->NormalState);
		NormalExt->setRubble(currentBuilding);

	} else { // if we're not here to repair that thing, obviously, we're gonna crush it
		if(!this->RubbleState && !pTypeData->RubbleDestroyed) {
			Debug::Log("Warning! Building was supposed to be turned into Advanced Rubble but Ares could not obtain its rubble state. Check if Rubble.Destroyed is set (correctly).");
			return;
		}

		if(!this->RubbleState) {
			this->RubbleState = specific_cast<BuildingClass *>(pTypeData->RubbleDestroyed->CreateObject(currentBuilding->Owner));
		}
		this->RubbleState->Health = this->RubbleState->Type->Strength; // see description above
		// Location should not be changed by removal
		this->RubbleState->Put(&currentBuilding->Location, currentBuilding->Facing);

		// make sure we get back here if necessary
		BuildingExt::ExtData* RubbleExt = BuildingExt::ExtMap.Find(this->RubbleState);
		RubbleExt->setNormal(currentBuilding);
	}

	return;
}

// #666: IsTrench/Traversal
//! This function checks if occupants of the current building can, on principle, move on to the target building.
/*!
	Conditions checked are whether there are occupants in the current building, whether the target building is full,
	whether the current building even is a trench, and whether both buildings are of the same trench kind.

	The current system assumes 1x1 sized trench parts; it will probably work in all cases where the 0,0 (top) cells of
	buildings touch (e.g. a 3x3 building next to a 1x3 building), but not when the 0,0 cells are further apart.
	This may be changed at a later point in time, if there is demand for it.

	\param targetBuilding a pointer to the target building.
	\return returns true if traversal between both buildings is legal, otherwise false.
	\sa doTraverseTo()

	\author Renegade
	\date 16.12.09+
*/
bool BuildingExt::ExtData::canTraverseTo(BuildingClass* targetBuilding) {
	BuildingClass* currentBuilding = this->AttachedToObject;
	//BuildingTypeClass* currentBuildingType = game_cast<BuildingTypeClass *>(currentBuilding->GetTechnoType());
	BuildingTypeClass* targetBuildingType = targetBuilding->Type;

	if((targetBuilding->Occupants.Count >= targetBuildingType->MaxNumberOccupants) || !currentBuilding->Occupants.Count || !targetBuildingType->CanBeOccupied) {
		return false; // Can't traverse if there's no one to move, or the target is full, or we can't actually occupy the target
	}

	BuildingTypeExt::ExtData* currentBuildingTypeExt = BuildingTypeExt::ExtMap.Find(currentBuilding->Type);
	BuildingTypeExt::ExtData* targetBuildingTypeExt = BuildingTypeExt::ExtMap.Find(targetBuilding->Type);

	if((currentBuildingTypeExt->IsTrench > -1) && (currentBuildingTypeExt->IsTrench == targetBuildingTypeExt->IsTrench)) {
		// if we've come here, there's room, there are people to move, and the buildings are trenches and of the same kind
		// the only questioning remaining is whether they are next to each other.
		return targetBuilding->Location.DistanceFrom(currentBuilding->Location) <= 256.0; // if the target building is more than 256 leptons away, it's not on a straight neighboring cell

	} else {
		return false; // not the same trench kind or not a trench
	}

}

// #666: IsTrench/Traversal
//! This function moves as many occupants as possible from the current to the target building.
/*!
	This function will move occupants from the current building to the target building until either
	a) the target building is full, or
	b) the current building is empty.

	\warning The function assumes the legality of the traversal was checked beforehand with #canTraverseTo(), and will therefore not do any safety checks.

	\param targetBuilding a pointer to the target building.
	\sa canTraverseTo()

	\author Renegade
	\date 16.12.09+
*/
void BuildingExt::ExtData::doTraverseTo(BuildingClass* targetBuilding) {
	BuildingClass* currentBuilding = this->AttachedToObject;
	BuildingTypeClass* targetBuildingType = targetBuilding->Type;

	while(currentBuilding->Occupants.Count && (targetBuilding->Occupants.Count < targetBuildingType->MaxNumberOccupants)) { // depending on Westwood's handling, this could explode when Size > 1 units are involved...but don't tell the users that
		targetBuilding->Occupants.AddItem(currentBuilding->Occupants.GetItem(0));
		currentBuilding->Occupants.RemoveItem(0); // maybe switch Add/Remove if the game gets pissy about multiple of them walking around
	}
}

// =============================
// container hooks


DEFINE_HOOK(43BCBD, BuildingClass_CTOR, 6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(43BCF7, BuildingClass_DTOR, 6)
{
	GET(BuildingClass*, pItem, ECX);

	BuildingExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(453E20, BuildingClass_SaveLoad_Prefix, 5)
DEFINE_HOOK_AGAIN(454190, BuildingClass_SaveLoad_Prefix, 5)
{
	GET_STACK(BuildingExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<BuildingExt>::SavingObject = pItem;
	Container<BuildingExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(45417E, BuildingClass_Load_Suffix, 5)
{
	BuildingExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(454244, BuildingClass_Save_Suffix, 7)
{
	BuildingExt::ExtMap.SaveStatic();
	return 0;
}
