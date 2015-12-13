#include "Body.h"
#include "../Building/Body.h"
#include "../TechnoType/Body.h"
#include "../Rules/Body.h"
#include "../../Misc/Debug.h"

#include <AircraftClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <VocClass.h>

static bool IsDeactivated(TechnoClass * pThis) {
	return TechnoExt::ExtMap.Find(pThis)->IsDeactivated();
};

static Action GetAction(TechnoClass * pThis, ObjectClass *pThat = nullptr) {
	return TechnoExt::ExtMap.Find(pThis)->GetDeactivatedAction(pThat);
};

DEFINE_HOOK(447548, BuildingClass_GetCursorOverCell_Deactivated, 6)
{
	GET(BuildingClass *, pThis, ESI);
	if(IsDeactivated(pThis)) {
		R->EBX(GetAction(pThis));
		return 0x44776D;
	}
	return 0;
}

DEFINE_HOOK(447218, BuildingClass_GetCursorOverObject_Deactivated, 6)
{
	GET(BuildingClass *, pThis, ESI);
	GET_STACK(ObjectClass *, pThat, 0x1C);
	if(IsDeactivated(pThis)) {
		R->EAX(GetAction(pThis, pThat));
		return 0x447273;
	}
	return 0;
}

DEFINE_HOOK(7404B9, UnitClass_GetCursorOverCell_Deactivated, 6)
{
	GET(UnitClass *, pThis, ESI);
	if(IsDeactivated(pThis)) {
		R->EAX(GetAction(pThis));
		return 0x740805;
	}
	return 0;
}

DEFINE_HOOK(73FD5A, UnitClass_GetCursorOverObject_Deactivated, 5)
{
	GET(UnitClass *, pThis, ECX);
	GET_STACK(ObjectClass *, pThat, 0x20);
	if(IsDeactivated(pThis)) {
		R->EAX(GetAction(pThis, pThat));
		return 0x73FD72;
	}
	return 0;
}

DEFINE_HOOK(51F808, InfantryClass_GetCursorOverCell_Deactivated, 6)
{
	GET(InfantryClass *, pThis, EDI);
	if(IsDeactivated(pThis)) {
		R->EBX(GetAction(pThis));
		return 0x51FAE2;
	}
	return 0;
}

DEFINE_HOOK(51E440, InfantryClass_GetCursorOverObject_Deactivated, 8)
{
	GET(InfantryClass *, pThis, EDI);
	GET_STACK(ObjectClass *, pThat, 0x3C);
	if(IsDeactivated(pThis)) {
		R->EAX(GetAction(pThis, pThat));
		return 0x51E458;
	}
	return 0;
}

DEFINE_HOOK(417F83, AircraftClass_GetCursorOverCell_Deactivated, 6)
{
	GET(AircraftClass *, pThis, ESI);
	if(IsDeactivated(pThis)) {
		R->EAX(GetAction(pThis));
		return 0x417F94;
	}
	return 0;
}

DEFINE_HOOK(417CCB, AircraftClass_GetCursorOverObject_Deactivated, 5)
{
	GET(AircraftClass *, pThis, ECX);
	GET_STACK(ObjectClass *, pThat, 0x20);
	if(IsDeactivated(pThis)) {
		R->EAX(GetAction(pThis, pThat));
		return 0x417CDF;
	}
	return 0;
}

DEFINE_HOOK(4D74EC, FootClass_ClickedAction_Deactivated, 6)
{
	GET(FootClass *, pThis, ESI);
	return (IsDeactivated(pThis))
		? 0x4D77EC
		: 0
	;
}

// another hook is at 443414 and shares the EIP with a trench enter handler

DEFINE_HOOK(4D7D58, FootClass_140_Deactivated, 6)
{
	GET(FootClass *, pThis, ESI);
	return (IsDeactivated(pThis))
		? 0x4D7D62
		: 0
	;
}

DEFINE_HOOK(4436F7, BuildingClass_140_Deactivated, 5)
{
	GET(BuildingClass *, pThis, ECX);
	return (IsDeactivated(pThis))
		? 0x443729
		: 0
	;
}

DEFINE_HOOK(5200B3, InfantryClass_UpdatePanic, 6)
{
	GET(InfantryClass *, pThis, ESI);
	if(IsDeactivated(pThis)) {
		if(pThis->PanicDurationLeft > 0) {
			--pThis->PanicDurationLeft;
		}
		return 0x52025A;
	}
	return 0;
}

DEFINE_HOOK(51D0DD, InfantryClass_Scatter, 6)
{
	GET(InfantryClass *, pThis, ESI);
	return (IsDeactivated(pThis))
		? 0x51D6E6
		: 0
	;
}

// do not order deactivated units to move
DEFINE_HOOK(73DBF9, UnitClass_Mi_Unload_Decactivated, 5)
{
	GET(FootClass*, pUnloadee, EDI);
	LEA_STACK(CellStruct**, ppCell, 0x0);
	LEA_STACK(CellStruct*, pPosition, 0x1C);

	if(pUnloadee->Deactivated) {
		pUnloadee->Locomotor->Power_Off();
	}

	if(!pUnloadee->Locomotor->Is_Powered()) {
		*ppCell = pPosition;
	}

	return 0;
}

DEFINE_HOOK(736135, UnitClass_Update_Deactivated, 6)
{
	GET(UnitClass*, pThis, ESI);
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	// don't sparkle on EMP, Operator, ....
	return pExt->IsPowered() ? 0x7361A9 : 0;
}

DEFINE_HOOK(73C143, UnitClass_DrawVXL_Deactivated, 5)
{
	GET(UnitClass*, pThis, ECX);
	REF_STACK(int, Value, 0x1E0);

	auto pRules = RulesExt::Global();
	double factor = 1.0;

	if(pThis->IsUnderEMP()) {
		factor = pRules->DeactivateDim_EMP;
	} else if(pThis->IsDeactivated()) {
		auto pExt = TechnoExt::ExtMap.Find(pThis);

		// use the operator check because it is more
		// efficient than the powered check.
		if(!pExt->IsOperated()) {
			factor = pRules->DeactivateDim_Operator;
		} else {
			factor = pRules->DeactivateDim_Powered;
		}
	}

	Value = static_cast<int>(Value * factor);

	return 0x73C15F;
}

// complete replacement
DEFINE_HOOK(70FBE0, TechnoClass_Activate, 6)
{
	GET(TechnoClass* const, pThis, ECX);
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	/* Check abort conditions:
		- Is the object currently EMP'd?
		- Does the object need an operator, but doesn't have one?
		- Does the object need a powering structure that is offline?
		If any of the above conditions, bail out and don't activate the object.
	*/
	if(pThis->IsUnderEMP() || !pExt->IsPowered() || !pExt->IsOperated()) {
		return 0x70FC85;
	}

	pThis->Guard();

	if(auto const pFoot = abstract_cast<FootClass*>(pThis)) {
		pFoot->Locomotor->Power_On();
	}

	auto const wasDeactivated = std::exchange(pThis->Deactivated, false);

	if(wasDeactivated) {
		auto const pType = pThis->GetTechnoType();

		// change: don't play sound when mutex active
		if(!Unsorted::IKnowWhatImDoing && pType->ActivateSound != -1) {
			VocClass::PlayAt(pType->ActivateSound, pThis->Location, nullptr);
		}

		// change: add spotlight
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		if(pTypeExt->Is_Spotlighted) {
			++Unsorted::IKnowWhatImDoing;
			auto const pSpotlight = GameCreate<BuildingLightClass>(pThis);
			pExt->SetSpotlight(pSpotlight);
			--Unsorted::IKnowWhatImDoing;
		}

		// change: update factories
		if(auto const pBld = abstract_cast<BuildingClass*>(pThis)) {
			BuildingExt::UpdateFactoryQueues(pBld);
		}
	}

	return 0x70FC85;
}

// complete replacement
DEFINE_HOOK(70FC90, TechnoClass_Deactivate, 6)
{
	GET(TechnoClass* const, pThis, ECX);

	// don't deactivate when inside/on the linked building
	if(pThis->unknown_bool_418) {
		auto const pLink = pThis->GetNthLink(0);

		if(pLink && pThis->GetCell()->GetBuilding() == pLink) {
			return 0x70FD6E;
		}
	}

	pThis->Guard();
	pThis->Deselect();

	if(auto const pFoot = abstract_cast<FootClass*>(pThis)) {
		pFoot->Locomotor->Power_Off();
	}

	auto const wasDeactivated = std::exchange(pThis->Deactivated, true);

	if(!wasDeactivated) {
		auto const pType = pThis->GetTechnoType();

		// change: don't play sound when mutex active
		if(!Unsorted::IKnowWhatImDoing && pType->DeactivateSound != -1) {
			VocClass::PlayAt(pType->DeactivateSound, pThis->Location, nullptr);
		}

		// change: remove spotlight
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		if(pTypeExt->Is_Spotlighted) {
			auto const pExt = TechnoExt::ExtMap.Find(pThis);
			pExt->SetSpotlight(nullptr);
		}

		// change: update factories
		if(auto const pBld = abstract_cast<BuildingClass*>(pThis)) {
			BuildingExt::UpdateFactoryQueues(pBld);
		}
	}

	return 0x70FD6E;
}
