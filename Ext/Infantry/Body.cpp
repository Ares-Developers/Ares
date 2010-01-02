#include <InfantryClass.h>
#include <ScenarioClass.h>

#include <BuildingClass.h>
#include <CellClass.h>

#include "Body.h"

bool InfantryExt::ExtData::IsOccupant() {
	InfantryClass* thisTrooper = this->AttachedToObject;

	// under the assumption that occupants' position is correctly updated and not stuck on the pre-occupation one
	if(BuildingClass* buildingBelow = thisTrooper->GetCell()->GetBuilding()) {
		return (buildingBelow->Occupants.FindItemIndex(&thisTrooper) != -1); // cases where this could be false would be Nighthawks or Rocketeers above the building
	} else {
		return false; // if there is no building, he can't occupy one
	}
}
