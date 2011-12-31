#include "Body.h"
#include "../TechnoType/Body.h"
#include "../../Misc/Debug.h"

static bool IsDeactivated(TechnoClass * pThis) {
	return TechnoExt::ExtMap.Find(pThis)->IsDeactivated();
};

static eAction GetAction(TechnoClass * pThis, ObjectClass *pThat = NULL) {
	return TechnoExt::ExtMap.Find(pThis)->GetDeactivatedAction(pThat);
};

DEFINE_HOOK(0x447548, BuildingClass_GetCursorOverCell_Deactivated, 0x6)
{
	GET(BuildingClass *, pThis, ESI);
	if(IsDeactivated(pThis)) {
		R->EBX<eAction>(GetAction(pThis));
		return 0x44776D;
	}
	return 0;
}

DEFINE_HOOK(0x447218, BuildingClass_GetCursorOverObject_Deactivated, 0x6)
{
	GET(BuildingClass *, pThis, ESI);
	GET_STACK(ObjectClass *, pThat, 0x1C);
	if(IsDeactivated(pThis)) {
		R->EAX<eAction>(GetAction(pThis, pThat));
		return 0x447273;
	}
	return 0;
}

DEFINE_HOOK(0x7404B9, UnitClass_GetCursorOverCell_Deactivated, 0x6)
{
	GET(UnitClass *, pThis, ESI);
	if(IsDeactivated(pThis)) {
		R->EAX<eAction>(GetAction(pThis));
		return 0x740805;
	}
	return 0;
}

DEFINE_HOOK(0x73FD5A, UnitClass_GetCursorOverObject_Deactivated, 0x5)
{
	GET(UnitClass *, pThis, ECX);
	GET_STACK(ObjectClass *, pThat, 0x20);
	if(IsDeactivated(pThis)) {
		R->EAX<eAction>(GetAction(pThis, pThat));
		return 0x73FD72;
	}
	return 0;
}

DEFINE_HOOK(0x51F808, InfantryClass_GetCursorOverCell_Deactivated, 0x6)
{
	GET(InfantryClass *, pThis, EDI);
	if(IsDeactivated(pThis)) {
		R->EBX<eAction>(GetAction(pThis));
		return 0x51FAE2;
	}
	return 0;
}

DEFINE_HOOK(0x51E440, InfantryClass_GetCursorOverObject_Deactivated, 0x8)
{
	GET(InfantryClass *, pThis, EDI);
	GET_STACK(ObjectClass *, pThat, 0x3C);
	if(IsDeactivated(pThis)) {
		R->EAX<eAction>(GetAction(pThis, pThat));
		return 0x51E458;
	}
	return 0;
}

DEFINE_HOOK(0x417F83, AircraftClass_GetCursorOverCell_Deactivated, 0x6)
{
	GET(AircraftClass *, pThis, ESI);
	if(IsDeactivated(pThis)) {
		R->EAX<eAction>(GetAction(pThis));
		return 0x417F94;
	}
	return 0;
}

DEFINE_HOOK(0x417CCB, AircraftClass_GetCursorOverObject_Deactivated, 0x5)
{
	GET(AircraftClass *, pThis, ECX);
	GET_STACK(ObjectClass *, pThat, 0x20);
	if(IsDeactivated(pThis)) {
		R->EAX<eAction>(GetAction(pThis, pThat));
		return 0x417CDF;
	}
	return 0;
}

DEFINE_HOOK(0x4D74EC, FootClass_ClickedAction_Deactivated, 0x6)
{
	GET(FootClass *, pThis, ESI);
	return (IsDeactivated(pThis))
		? 0x4D77EC
		: 0
	;
}

// this hook shares the EIP with a trench enter handler, todo: check if they can brick each other
DEFINE_HOOK(0x443414, BuildingClass_ClickedAction_Deactivated, 0x6)
{
	GET(BuildingClass *, pThis, ECX);
	return (IsDeactivated(pThis))
		? 0x44344D
		: 0
	;
}

DEFINE_HOOK(0x4D7D58, FootClass_140_Deactivated, 0x6)
{
	GET(FootClass *, pThis, ESI);
	return (IsDeactivated(pThis))
		? 0x4D7D62
		: 0
	;
}

DEFINE_HOOK(0x4436F7, BuildingClass_140_Deactivated, 0x5)
{
	GET(BuildingClass *, pThis, ECX);
	return (IsDeactivated(pThis))
		? 0x443729
		: 0
	;
}

DEFINE_HOOK(0x5200B3, InfantryClass_UpdatePanic, 0x6)
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

DEFINE_HOOK(0x51D0DD, InfantryClass_Scatter, 0x6)
{
	GET(InfantryClass *, pThis, ESI);
	return (IsDeactivated(pThis))
		? 0x51D6E6
		: 0
	;
}
