#include <InfantryClass.h>
#include <ScenarioClass.h>

#include <BuildingClass.h>
#include <CellClass.h>
#include <HouseClass.h>
#include <SessionClass.h>

#include "Body.h"

#include "../Rules/Body.h"
#include <GameModeOptionsClass.h>

template<> const DWORD Extension<InfantryClass>::Canary = 0xE1E2E3E4;
InfantryExt::ExtContainer InfantryExt::ExtMap;

bool InfantryExt::ExtData::IsOccupant() {
	InfantryClass* thisTrooper = this->OwnerObject();

	// under the assumption that occupants' position is correctly updated and not stuck on the pre-occupation one
	if(BuildingClass* buildingBelow = thisTrooper->GetCell()->GetBuilding()) {
		// cases where this could be false would be Nighthawks or Rocketeers above the building
		return (buildingBelow->Occupants.FindItemIndex(thisTrooper) != -1);
	} else {
		return false; // if there is no building, he can't occupy one
	}
}

//! Gets the action an engineer will take when entering an enemy building.
/*!
	This function accounts for the multi-engineer feature.

	\param pBld The Building the engineer enters.

	\author AlexB
	\date 2012-07-22
*/
Action InfantryExt::GetEngineerEnterEnemyBuildingAction(
	BuildingClass* const pBld)
{
	// only skirmish allows to disable it, so we only check there. for all other
	// modes, it's always on. single player campaigns also use special multi
	// engineer behavior.
	auto const gameMode = SessionClass::Instance->GameMode;
	auto allowDamage = gameMode != GameMode::Skirmish 
		|| GameModeOptionsClass::Instance->MultiEngineer;

	if(gameMode == GameMode::Campaign) {
		// single player missions are currently hardcoded to "don't do damage".
		allowDamage = false; // TODO: replace this by a new rules tag.
	}

	// damage if multi engineer is enabled and target isn't that low on health.
	if(allowDamage) {

		// check to always capture tech structures. a structure counts
		// as tech if its initial owner is a multiplayer-passive country.
		auto const isTech = pBld->InitialOwner
			? pBld->InitialOwner->IsNeutral() : false;

		auto const pRulesExt = RulesExt::Global();
		if(!isTech || !pRulesExt->EngineerAlwaysCaptureTech) {
			// no civil structure. apply new logic.
			auto const capLevel = RulesClass::Global()->EngineerCaptureLevel;
			if(pBld->GetHealthPercentage() > capLevel) {
				return (pRulesExt->EngineerDamage > 0.0)
					? Action::Damage : Action::NoEnter;
			}
		}
	}

	// default.
	return Action::Capture;
}

// =============================
// load / save

template <typename T>
void InfantryExt::ExtData::Serialize(T& Stm) {
	//Stm;
}

void InfantryExt::ExtData::LoadFromStream(AresStreamReader &Stm) {
	Extension<InfantryClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void InfantryExt::ExtData::SaveToStream(AresStreamWriter &Stm) {
	Extension<InfantryClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

InfantryExt::ExtContainer::ExtContainer() : Container("InfantryClass") {
}

InfantryExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
DEFINE_HOOK(517CB0, InfantryClass_CTOR, 5)
{
	GET(InfantryClass*, pItem, ESI);

	InfantryExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(517F83, InfantryClass_DTOR, 6)
{
	GET(InfantryClass*, pItem, ESI);

	InfantryExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(521B00, InfantryClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(521960, InfantryClass_SaveLoad_Prefix, 6)
{
	GET_STACK(InfantryClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	InfantryExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(521AEC, InfantryClass_Load_Suffix, 6)
{
	InfantryExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(521B14, InfantryClass_Save_Suffix, 3)
{
	InfantryExt::ExtMap.SaveStatic();
	return 0;
}
#endif
