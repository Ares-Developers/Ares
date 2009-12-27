#include <InfantryClass.h>
#include <BuildingClass.h>
#include <SpecificStructures.h>
#include "../Building/Body.h"
#include "../BuildingType/Body.h"

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
		pThis->Remove();
		TargetExtData->RubbleYell(true);
		if(pThis->Put(&pThis->Location, 0)) {
			CoordStruct XYZ;
			pThis->GetCoords(&XYZ);
			DWORD pLocation = (DWORD)&XYZ;
			pThis->Scatter(pLocation, 1, 0); // since we're not gonna eat the Engineer, it has to move away
		} else {
			pThis->UnInit();
		}
	}


	return do_normal_repair ? 0 : 0x51A65D; //0x51A010 eats the Engineer, 0x51A65D hopefully does not
}
