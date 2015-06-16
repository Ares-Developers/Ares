#include <InfantryClass.h>
#include <BuildingClass.h>
#include <SpecificStructures.h>
#include "../Building/Body.h"
#include "../BuildingType/Body.h"
#include "../Techno/Body.h"
#include "Body.h"
#include "../Rules/Body.h"
#include "../../Misc/Actions.h"
#include <HouseClass.h>
#include <InputManagerClass.h>
#include <VoxClass.h>

// #664: Advanced Rubble - reconstruction part: Check
DEFINE_HOOK(51E63A, InfantryClass_GetCursorOverObject_EngineerOverFriendlyBuilding, 6) {
	GET(BuildingClass *, pTarget, ESI);
	GET(InfantryClass *, pThis, EDI);

	if(BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pTarget->Type)) {
		if((pData->RubbleIntact || pData->RubbleIntactRemove) && pTarget->Owner->IsAlliedWith(pThis)) {
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

	if(TargetTypeExtData->RubbleIntact || TargetTypeExtData->RubbleIntactRemove) {
		do_normal_repair = false;
		bool wasSelected = pThis->IsSelected;
		pThis->Remove();
		if(!TargetExtData->RubbleYell(true)) {
			++Unsorted::IKnowWhatImDoing;
			Target->Put(Target->Location, Target->Facing.current().value8());
			--Unsorted::IKnowWhatImDoing;
			VoxClass::Play("EVA_CannotDeployHere");
		}
		CellStruct Cell = pThis->GetMapCoords();
		Target->KickOutUnit(pThis, Cell);
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

	if(auto pGarrison = pData->GarrisonedIn) {
		if(!pGarrison->Occupants.Remove(pThis)) {
			Debug::Log("Infantry %s was garrisoned in building %s, but building didn't find it. WTF?", pThis->Type->ID, pGarrison->Type->ID);
		}
	}

	pData->GarrisonedIn = nullptr;

	return 0;
}

DEFINE_HOOK(51DFFD, InfantryClass_Put, 5)
{
	GET(InfantryClass *, pThis, EDI);
	TechnoExt::ExtData* pData = TechnoExt::ExtMap.Find(pThis);
	pData->GarrisonedIn = nullptr;

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
	return (I->Transporter || I->Type->MovementZone != MovementZone::AmphibiousDestroyer)
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
	Action ret = InfantryExt::GetEngineerEnterEnemyBuildingAction(pBld);

	// use a dedicated cursor
	if(ret == Action::Damage) {
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
	Action action = InfantryExt::GetEngineerEnterEnemyBuildingAction(pBld);
	if(action == Action::Damage) {
		int Damage = static_cast<int>(ceil(pBld->Type->Strength * RulesExt::Global()->EngineerDamage));
		pBld->ReceiveDamage(&Damage, 0, RulesClass::Global()->C4Warhead, pEngi, true, false, nullptr);
		return 0x51A010;
	} else {
		return 0x519EAA;
	}
}

// #1008047: the C4 did not work correctly in YR, because some ability checks were missing
DEFINE_HOOK(51C325, InfantryClass_IsCellOccupied_C4Ability, 6)
{
	GET(InfantryClass*, pThis, EBP);

	return (pThis->Type->C4 || pThis->HasAbility(Ability::C4)) ? 0x51C37D : 0x51C335;
}

DEFINE_HOOK(51A4D2, InfantryClass_UpdatePosition_C4Ability, 6)
{
	GET(InfantryClass*, pThis, ESI);

	return (!pThis->Type->C4 && !pThis->HasAbility(Ability::C4)) ? 0x51A7F4 : 0x51A4E6;
}

// do not prone in water
DEFINE_HOOK(5201CC, InfantryClass_UpdatePanic_ProneWater, 6)
{
	GET(InfantryClass*, pThis, ESI);
	auto landType = pThis->GetCell()->LandType;
	return (landType != LandType::Beach && landType != LandType::Water) ? 0 : 0x5201DC;
}

// #1283638: ivans cannot enter grinders; they get an attack cursor. if the
// grinder is rigged with a bomb, ivans can enter. this fix lets ivans enter
// allied grinders. pressing the force fire key brings back the old behavior.
DEFINE_HOOK(51EB48, InfantryClass_GetCursorOverObject_IvanGrinder, A)
{
	GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pTarget, ESI);

	if(auto pTargetBld = abstract_cast<BuildingClass*>(pTarget)) {
		if(pTargetBld->Type->Grinding && pThis->Owner->IsAlliedWith(pTargetBld)) {
			if(!InputManagerClass::Instance->IsForceFireKeyPressed()) {
				static const byte return_grind[] = {
					0x5F, 0x5E, 0x5D, // pop edi, esi and ebp
					0xB8, 0x0B, 0x00, 0x00, 0x00, // eax = Action::Repair (not Action::Eaten)
					0x5B, 0x83, 0xC4, 0x28, // esp += 0x28
					0xC2, 0x08, 0x00 // retn 8
				};

				return reinterpret_cast<DWORD>(return_grind);
			}
		}
	}

	return 0;
}
