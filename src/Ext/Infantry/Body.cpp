#include <InfantryClass.h>
#include <ScenarioClass.h>

#include <BuildingClass.h>
#include <CellClass.h>

#include "Body.h"

#include "../Rules/Body.h"
#include <GameModeOptionsClass.h>

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

//! Gets the action an engineer will take when entering an enemy building.
/*!
	This function accounts for the multi-engineer feature.

	\param pBld The Building the engineer enters.

	\author AlexB
	\date 2010-05-28
*/
eAction InfantryExt::GetEngineerEnterEnemyBuildingAction(BuildingClass *pBld) {
	// damage if multi engineer is enabled and target isn't that low on health.
	// no other mode than skirmish allows to disable it, so we only check whether
	// multi engineer is disabled there. for all other modes, it's always on.
	if(SessionClass::Instance->GameMode != GameMode::Skirmish || GameModeOptionsClass::Instance->MultiEngineer) {

		// check to always capture tech structures. a structure counts
		// as tech if it has the NeedsEngineer flag set.
		bool isTech = pBld->Type->NeedsEngineer;

		if(!RulesExt::Global()->EngineerAlwaysCaptureTech || !isTech) {
			// no civil structure. apply new logic.
			if(pBld->GetHealthPercentage() > RulesClass::Global()->EngineerCaptureLevel) {
				return (RulesExt::Global()->EngineerDamage > 0) ? act_Damage : act_NoEnter;
			}
		}
	}

	// default.
	return act_Capture;
}
