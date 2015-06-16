#include "Body.h"

#include "../BuildingType/Body.h"
#include "../TechnoType/Body.h"

#include <BuildingClass.h>
#include <HouseClass.h>

DEFINE_HOOK(7004AD, TechnoClass_GetCursorOverObject_Saboteur, 6)
{
	// this is known to be InfantryClass, and Infiltrate is yes
	GET(InfantryClass const* const, pThis, ESI);
	GET(ObjectClass const* const, pObject, EDI);

	bool infiltratable = false;

	if(auto const pBldObject = abstract_cast<BuildingClass const*>(pObject)) {
		auto const pThisType = pThis->Type;
		auto const pObjectType = pBldObject->Type;

		auto const pThisTypeExt = TechnoTypeExt::ExtMap.Find(pThisType);
		auto const agent = pThisType->Agent;
		auto const saboteur = pThisTypeExt->Saboteur;

		infiltratable = (agent && pObjectType->Spyable)
			|| (saboteur && BuildingTypeExt::IsSabotagable(pObjectType))
			|| (!agent && !saboteur && pObjectType->Capturable)
			|| ((pThisType->C4 || pThis->HasAbility(Ability::C4)) && pObjectType->CanC4);
	}

	return infiltratable ? 0x700531u : 0x700536u;
}

DEFINE_HOOK(51EE6B, InfantryClass_GetCursorOverObject_Saboteur, 6)
{
	GET(InfantryClass const* const, pThis, EDI);
	GET(ObjectClass const* const, pObject, ESI);

	bool infiltratable = false;

	if(auto const pBldObject = abstract_cast<BuildingClass const*>(pObject)) {
		if(!pThis->Owner->IsAlliedWith(pBldObject)) {
			auto const pThisType = pThis->Type;
			auto const pObjectType = pBldObject->Type;

			auto const pThisTypeExt = TechnoTypeExt::ExtMap.Find(pThisType);
			auto const agent = pThisType->Agent;
			auto const saboteur = pThisTypeExt->Saboteur;

			infiltratable = (!agent && !saboteur && pObjectType->Capturable)
				|| (agent && pObjectType->Spyable)
				|| (saboteur && BuildingTypeExt::IsSabotagable(pObjectType));
		}
	}

	return infiltratable ? 0x51EEEDu : 0x51F04Eu;
}

DEFINE_HOOK(51B2CB, InfantryClass_SetTarget_Saboteur, 6)
{
	GET(InfantryClass* const, pThis, ESI);
	GET(ObjectClass* const, pTarget, EDI);

	if(auto const pBldObject = abstract_cast<BuildingClass const*>(pTarget)) {
		auto const pThisType = pThis->Type;
		auto const pTargetType = pBldObject->Type;

		auto const pThisTypeExt = TechnoTypeExt::ExtMap.Find(pThisType);

		auto allowed = false;

		if(pThisType->Agent) {
			allowed = pTargetType->Spyable;
		} else if(pThisTypeExt->Saboteur) {
			allowed = BuildingTypeExt::IsSabotagable(pTargetType);
		} else {
			allowed = pTargetType->Capturable;
		}

		if(allowed) {
			pThis->SetDestination(pTarget, true);
		}
	}

	return 0x51B33F;
}

DEFINE_HOOK(51A03E, InfantryClass_UpdatePosition_Saboteur, 6)
{
	GET(InfantryClass* const, pThis, ESI);
	GET(BuildingClass* const, pBuilding, EDI);

	auto const pThisType = pThis->Type;
	auto const pThisTypeExt = TechnoTypeExt::ExtMap.Find(pThisType);

	bool sabotage = false;
	if(pThisTypeExt->Saboteur && BuildingTypeExt::IsSabotagable(pBuilding->Type)) {
		if(pBuilding->IsIronCurtained() || pBuilding->IsBeingWarpedOut()
			|| pBuilding->GetCurrentMission() == Mission::Selling)
		{
			// building not sabotagable atm
			pThis->AbortMotion();
			pThis->Uncloak(false);
			pThis->ReloadTimer.Start(pThis->GetROF(1));

		} else if(pBuilding->C4Applied) {
			// scatter out
			pThis->ReloadTimer.Start(pThis->GetROF(1));

		} else {
			// sabotage
			pBuilding->C4Applied = true;
			pBuilding->C4AppliedBy = pThis;

			auto const delay = RulesClass::Instance->C4Delay;
			auto const duration = static_cast<int>(delay * 900);
			pBuilding->Flash(duration / 2);
			pBuilding->C4Timer.Start(duration);

			if(auto const pTag = pBuilding->AttachedTag) {
				pTag->RaiseEvent(
					TriggerEvent::EnteredBy, pThis, CellStruct::Empty, false,
					nullptr);
			}

			sabotage = true;
		}
	}

	return sabotage ? 0x51A010u : 0u;
}
