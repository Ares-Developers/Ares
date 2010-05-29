#include <InfantryClass.h>
#include <ScenarioClass.h>

#include <BuildingClass.h>
#include <CellClass.h>

#include "Body.h"

Container<InfantryExt> InfantryExt::ExtMap;

bool InfantryExt::ExtData::IsOccupant() {
	InfantryClass* thisTrooper = this->AttachedToObject;

	// under the assumption that occupants' position is correctly updated and not stuck on the pre-occupation one
	if(BuildingClass* buildingBelow = thisTrooper->GetCell()->GetBuilding()) {
		// cases where this could be false would be Nighthawks or Rocketeers above the building
		return (buildingBelow->Occupants.FindItemIndex(&thisTrooper) != -1);
	} else {
		return false; // if there is no building, he can't occupy one
	}
}
