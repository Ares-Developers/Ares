#include <InfantryClass.h>
#include <BuildingClass.h>
#include <SpecificStructures.h>
#include "../Building/Body.h"
#include "../BuildingType/Body.h"
#include "../Techno/Body.h"
#include "Body.h"
#include "../Rules/Body.h"
#include "../../Misc/Actions.h"

// #664: Advanced Rubble - reconstruction part: Check
DEFINE_HOOK(51E63A, InfantryClass_GetCursorOverObject_EngineerOverFriendlyBuilding, 6) {
	GET(BuildingClass *, pTarget, ESI);
	GET(InfantryClass *, pThis, EDI);

	if(BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pTarget->Type)) {
		if(pData->RubbleIntact && pTarget->Owner->IsAlliedWith(pThis)) {
			return 0x51E659;
		}
	}

	return 0;
}

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
		if(!TargetExtData->RubbleYell(true)) {
			++Unsorted::IKnowWhatImDoing;
			Target->Put(&Target->Location, Target->Facing);
			--Unsorted::IKnowWhatImDoing;
			VoxClass::Play("EVA_CannotDeployHere");
		}
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
		signed int idx = Garrison->Occupants.FindItemIndex(pThis);
		if(idx == -1) {
			Debug::Log("Infantry %s was garrisoned in building %s, but building didn't find it. WTF?", pThis->Type->ID, Garrison->Type->ID);
		} else {
			Garrison->Occupants.RemoveItem(idx);
		}
	}

	pData->GarrisonedIn = NULL;

	return 0;
}

DEFINE_HOOK(51DFFD, InfantryClass_Put, 5)
{
	GET(InfantryClass *, pThis, EDI);
	TechnoExt::ExtData* pData = TechnoExt::ExtMap.Find(pThis);
	pData->GarrisonedIn = NULL;

	return 0;
}

DEFINE_HOOK(518434, InfantryClass_ReceiveDamage_SkipDeathAnim, 7)
{
	GET(InfantryClass *, pThis, ESI);
	//GET_STACK(ObjectClass *, pAttacker, 0xE0);
//	InfantryExt::ExtData* trooperAres = InfantryExt::ExtMap.Find(pThis);
//	bool skipInfDeathAnim = false; // leaving this default in case this is expanded in the future

	// there is not InfantryExt ExtMap yet!
	// too much space would get wasted since there is only four bytes worth of data we need to store per object
	// so those four bytes get stashed in Techno Map instead. they will get their own map if there's ever enough data to warrant it
	TechnoExt::ExtData* pData = TechnoExt::ExtMap.Find(pThis);

	return pData->GarrisonedIn ? 0x5185F1 : 0;
}

// should correct issue #743
DEFINE_HOOK(51D799, InfantryClass_PlayAnim_WaterSound, 7)
{
	GET(InfantryClass *, I, ESI);
	return (I->Transporter || I->Type->MovementZone != mz_AmphibiousDestroyer)
		? 0x51D8BF
		: 0x51D7A6
	;
}

DEFINE_HOOK(51E5BB, InfantryClass_GetCursorOverObject_MultiEngineerA, 7) {
	// skip old logic's way to determine the cursor
	return 0x51E5D9;
}

DEFINE_HOOK(51E5E1, InfantryClass_GetCursorOverObject_MultiEngineerB, 7) {
	GET(BuildingClass *, pBld, ECX);
	eAction ret = InfantryExt::GetEngineerEnterEnemyBuildingAction(pBld);

	// use a dedicated cursor
	if(ret == act_Damage) {
		Actions::Set(&RulesExt::Global()->EngineerDamageCursor);
	}

	// return our action
	R->EAX(ret);
	return 0;
}

DEFINE_HOOK(519D9C, InfantryClass_UpdatePosition_MultiEngineer, 5) {
	GET(InfantryClass *, pEngi, ESI);
	GET(BuildingClass *, pBld, EDI);

	// damage or capture
	eAction action = InfantryExt::GetEngineerEnterEnemyBuildingAction(pBld);
	if(action == act_Damage) {
		int Damage = (int)ceil(pBld->Type->Strength * RulesExt::Global()->EngineerDamage);
		pBld->ReceiveDamage(&Damage, 0, RulesClass::Global()->C4Warhead, pEngi, 1, 0, 0);
		return 0x51A010;
	} else {
		return 0x519EAA;
	}
}