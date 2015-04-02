#include "Body.h"

#include <HouseClass.h>

DEFINE_HOOK(71E949, EventClass_HasOccured, 7)
{
	GET(TEventClass*, pEvent, EBP);

	// check for events handled in Ares.
	bool ret = false;
	if(TEventExt::HasOccured(pEvent, &ret)) {
		// returns true or false
		return ret ? 0x71F1B1 : 0x71F163;
	}
	
	// not handled in Ares.
	return 0;
}

// the general events requiring a house
DEFINE_HOOK(71F06C, EventClass_HasOccured_PlayerAtX1, 5)
{
	GET(int const, param, ECX);

	auto const pHouse = TEventExt::ResolveHouseParam(param);
	R->EAX(pHouse);

	// continue normally if a house was found or this isn't Player@X logic,
	// otherwise return false directly so events don't fire for non-existing
	// players.
	return (pHouse || !HouseClass::Index_IsMP(param)) ? 0x71F071u : 0x71F0D5u;
}

// validation for Spy as House, the Entered/Overflown Bys and the Crossed V/H Lines
DEFINE_HOOK_AGAIN(71ED33, EventClass_HasOccured_PlayerAtX2, 5)
DEFINE_HOOK_AGAIN(71F1C9, EventClass_HasOccured_PlayerAtX2, 5)
DEFINE_HOOK_AGAIN(71F1ED, EventClass_HasOccured_PlayerAtX2, 5)
DEFINE_HOOK(71ED01, EventClass_HasOccured_PlayerAtX2, 5)
{
	GET(int const, param, ECX);
	R->EAX(TEventExt::ResolveHouseParam(param));
	return R->Origin() + 5;
}

// param for Attacked by House is the array index
DEFINE_HOOK(71EE79, EventClass_HasOccured_PlayerAtX3, 9)
{
	GET(int, param, EAX);
	GET(HouseClass* const, pHouse, EDX);

	// convert Player @ X to real index
	if(HouseClass::Index_IsMP(param)) {
		auto const pPlayer = TEventExt::ResolveHouseParam(param);
		param = pPlayer ? pPlayer->ArrayIndex : -1;
	}

	return (pHouse->ArrayIndex == param) ? 0x71EE82u : 0x71F163u;
}
