#include "Body.h"
#include "../TechnoType/Body.h"

#include <GameModeOptionsClass.h>
#include <BuildingClass.h>
#include <AircraftClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <ArrayClasses.h>
#include <Helpers/Template.h>

// =============================
// other hooks

DEFINE_HOOK(4F7870, HouseClass_PrereqValidator, 7)
{
	// int (TechnoTypeClass *item, bool BuildLimitOnly, bool includeQueued)
	/* return
		 1 - cameo shown
		 0 - cameo not shown
		-1 - cameo greyed out
	 */

	GET(HouseClass *, pHouse, ECX);
	GET_STACK(TechnoTypeClass *, pItem, 0x4);
	GET_STACK(bool, BuildLimitOnly, 0x8);
	GET_STACK(bool, IncludeQueued, 0xC);

	R->EAX(HouseExt::PrereqValidate(pHouse, pItem, BuildLimitOnly, IncludeQueued));
	return 0x4F8361;
}

// upgrades as prereqs, facepalm of epic proportions
// not needed anymore since the whole function's been replaced
/*
A_FINE_HOOK(4F7E49, HouseClass_CanBuildHowMany_Upgrades, 5)
{
		return R->get_EAX() < 3 ? 0x4F7E41 : 0x4F7E34;
}
*/

/*
 * Attention: This is a rewrite of the "is this house defeated yet?" check that should clear up the
 * "On rare occasions, a player may lose every single unit they have but will not be 'defeated'." issues
 * But! It does so by detecting if you have any objects that are not in limbo, and defeating you if you don't.
 * An exception was added for parasites - they will count as eligible even when in limbo...
 */

DEFINE_HOOK(4F8EBD, HouseClass_Update_HasBeenDefeated, 0)
{
	GET(HouseClass *, H, ESI);

	if(H->OwnedBuildings) {
		return 0x4F8F87;
	}

	struct EligibleObject {
		HouseClass *H;
		EligibleObject(HouseClass *pHouse) : H(pHouse) {};
		bool operator()(TechnoClass *T) {
			if(T->Owner != H) {
				return false;
			}
			if(!T->InLimbo) {
				return true;
			}
			if(FootClass *F = generic_cast<FootClass *>(T)) {
				return F->ParasiteImUsing != NULL;
			}
			return false;
		}
	} Eligible(H);

	if(GameModeOptionsClass::Instance->ShortGame) {
		for(int i = 0; i < RulesClass::Instance->BaseUnit.Count; ++i) {
			UnitTypeClass *U = RulesClass::Instance->BaseUnit[i];
			if(H->OwnedUnitTypes.GetItemCount(U->ArrayIndex)) {
				return 0x4F8F87;
			}
		}
	} else {
		if(H->OwnedUnitTypes1.Count) {
			for(int i = 0; i < UnitClass::Array->Count; ++i) {
				TechnoClass *T = UnitClass::Array->Items[i];
				if(Eligible(T)) {
					return 0x4F8F87;
				}
			}
		}

		if(H->OwnedInfantryTypes1.Count) {
			for(int i = 0; i < InfantryClass::Array->Count; ++i) {
				TechnoClass *T = InfantryClass::Array->Items[i];
				if(Eligible(T)) {
					return 0x4F8F87;
				}
			}
		}

		if(H->OwnedAircraftTypes1.Count) {
			for(int i = 0; i < AircraftClass::Array->Count; ++i) {
				TechnoClass *T = AircraftClass::Array->Items[i];
				if(Eligible(T)) {
					return 0x4F8F87;
				}
			}
		}

	}

	H->DestroyAll();
	H->AcceptDefeat();

	return 0x4F8F87;
}

DEFINE_HOOK(4F645F, HouseClass_CTOR_FixSideIndices, 5)
{
	GET(HouseClass *, pHouse, EBP);
	if(HouseTypeClass * pCountry = pHouse->Type) {
		if(strcmp(pCountry->ID, "Neutral") && strcmp(pCountry->ID, "Special")) {
			pHouse->SideIndex = pCountry->SideIndex;
		}
	}
	return 0x4F6490;
}
