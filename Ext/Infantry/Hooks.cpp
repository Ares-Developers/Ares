#include <InfantryClass.h>
#include <BuildingClass.h>
#include <SpecificStructures.h>
#include "../Building/Body.h"
#include "../BuildingType/Body.h"
#include "../Techno/Body.h"

// #664: Advanced Rubble - reconstruction part: Check
/* -- TODO: UI handler
A_FINE_HOOK(51E635, InfantryClass_GetCursorOverObject_EngineerOverFriendlyBuilding, 5)
{
	GET(BuildingClass *, Target, ESI);
	GET(InfantryClass *, pThis, EDI);
	FPUControl fp(R->get_EAX()); // (Target->GetHealthPercentage() == RulesClass::Instance->ConditionIdeal)
	switch(control) {
		case Rebuild:
			// fall through - not decided about UI handling yet
		case DecideNormally:
		default:
		return fp.isEqual()
		 ? 0x51E63A
		 : 0x51E659
		;
	}
}
*/

// #664: Advanced Rubble - reconstruction part: Reconstruction
DEFINE_HOOK(519FAF, InfantryClass_UpdatePosition_EngineerRepairsFriendly, 6)
{
	GET(InfantryClass *, pThis, ESI);
	GET(BuildingClass *, Target, EDI);

	BuildingExt::ExtData* TargetExtData = BuildingExt::ExtMap.Find(Target);
	BuildingTypeExt::ExtData* TargetTypeExtData = BuildingTypeExt::ExtMap.Find(Target->Type);
	bool do_normal_repair = true;

	if(TargetTypeExtData->RubbleIntact) {
		do_normal_repair = false;
		bool wasSelected = pThis->IsSelected;
		pThis->Remove();
		TargetExtData->RubbleYell(true);
		CellStruct Cell;
		pThis->GetMapCoords(&Cell);
		Target->KickOutUnit(pThis, &Cell);
		if(wasSelected) {
			pThis->Select();
		}
	}

	return do_normal_repair ? 0 : 0x51A65D; //0x51A010 eats the Engineer, 0x51A65D hopefully does not
}

DEFINE_HOOK(51DF38, InfantryClass_Remove, A)
{
	GET(InfantryClass *, pThis, ESI);
	TechnoExt::ExtData* pData = TechnoExt::ExtMap.Find(pThis);

	if(BuildingClass *Garrison = pData->GarrisonedIn) {
		signed int idx = Garrison->Occupants.FindItemIndex(&pThis);
		if(idx == -1) {
			Debug::Log("Infantry %s was garrisoned in building %s, but building didn't find it. WTF?", pThis->Type->ID, Garrison->Type->ID);
		} else {
			Garrison->Occupants.RemoveItem(idx);
		}
	}

	return 0;
}

/*
A_FINE_HOOK(518434, InfantryClass_ReceiveDamage_SkipDeathAnim, 7)
{
	GET(InfantryClass *, pThis, ESI);
	GET_STACK(ObjectClass *, pAttacker, 0xE0);
	bool skipInfDeathAnim = false;
	return skipDeathAnim ? 0x5185F1 : 0;
}
*/
