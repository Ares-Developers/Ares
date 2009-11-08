#include "Body.h"

/* #633 - spy building infiltration */
DEFINE_HOOK(4571E0, BuildingClass_Infiltrate, 5)
{
	GET(BuildingClass *, EnteredBuilding, ECX);
	GET_STACK(HouseClass *, EnteredBy, 0x4);

	bool apply_normal_infiltration_logic = 1;

	//! RA1-Style Spying, as requested in issue #633
	//! This sets the respective bit to inform the game that a particular house has spied this building.
	//! Knowing that, the game will reveal the current production in this building to the players who have spied it.
	//! In practice, this means: If a player who has spied a factory clicks on that factory, he will see the cameo of whatever is being built in the factory.
	EnteredBuilding->DisplayProductionTo.Add(EnteredBy);

	// also note that the slow radar reparse call (MapClass::sub_4F42D0()) is not made here, meaning if you enter a radar, there will be a discrepancy between what you see on the map/tact map and what the game thinks you see

	// wrapper around the entire function, so return 0 or handle _every_ single thing
	return (apply_normal_infiltration_logic) ? 0 : 0x45759F;
}

// check before drawing the tooltip
DEFINE_HOOK(43E7EF, BuildingClass_DrawVisible_P1, 5)
{
	GET(BuildingClass *, B, ESI);
	return B->DisplayProductionTo.Contains(HouseClass::Player) ? 0x43E80E : 0x43E832;
}

// check before drawing production cameo
DEFINE_HOOK(43E832, BuildingClass_DrawVisible_P2, 6)
{
	GET(BuildingClass *, B, ESI);
	return B->DisplayProductionTo.Contains(HouseClass::Player) ? 0x43E856 : 0x43E8EC;
}

// fix palette for spied factory production cameo drawing
DEFINE_HOOK(43E8D9, BuildingClass_DrawVisible_P3, 6)
{
	R->set_EDX((DWORD)FileSystem::CAMEO_PAL);
	return 0x43E8DF;
}

// if this is a radar, change the owner's house bitfields responsible for radar reveals
DEFINE_HOOK(44161C, BuildingClass_Destroy_OldSpy1, 6)
{
	return 0x4416A2;
}

// if this is a radar, change the owner's house bitfields responsible for radar reveals
DEFINE_HOOK(448312, BuildingClass_ChangeOwnership_OldSpy1, a)
{
	return 0x4483A0;
}

// if this is a radar, drop the new owner from the bitfield
DEFINE_HOOK(448D95, BuildingClass_ChangeOwnership_OldSpy2, 8)
{
	return 0x448DB9;
}
