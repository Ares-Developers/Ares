/**
 * this file contains placeholder hooks for certain features so that C++ coders can implement them without any ASM issues
 * when you implement a feature, please move the hook from this file to wherever it's appropriate
 * (that would be where the rest of the implementation is, consult the project leaders if you're not sure)
 * do not simply move it outside this if0/endif block
 */

#if 0
#include <SpecificStructures.h>

/* #179 - New Upgrade System - promotions */
A_FINE_HOOK(6FA0F0, TechnoClass_Update_TurnedVeteran, 6)
{
	GET(TechnoClass *, Unit, ESI);
	bool apply_normal_logic = 1;

	// wraps around promotion audio effects
	return (apply_normal_logic) ? 0 : 0x6FA13E;
}

A_FINE_HOOK(6FA083, TechnoClass_Update_TurnedElite, 6)
{
	GET(TechnoClass *, Unit, ESI);
	bool apply_normal_logic = 1;

	// wraps around promotion audio effects and elite flashing
	return (apply_normal_logic) ? 0 : 0x6FA13E;
}

/* #306 - infantry exits armory after being promoted */
// note: a hook to say "can't enter armory" will come later
A_FINE_HOOK(44BB3C, BuildingClass_Mi_Repair, 6)
{
	GET(BuildingClass *, Armory, EBP);
	GET(InfantryClass *, Soldier, ESI);
	return 0;
}

/* #368 - vehicle enters a grinder */
// passengers have already been paid for and erased, same for parasite
// the unit itself has also been paid for
A_FINE_HOOK(73A1BC, UnitClass_UpdatePosition, 7)
{
	GET(UnitClass *, Vehicle, EBP);
	GET(BuildingClass *, Grinder, EBX);
	return 0;
}

/* #320 - objects are being instantiated (merge these into fixes for #183 in Bugfixes.cpp) */
/* see also #277 */
A_FINE_HOOK(442CE0, BuildingClass_Init_Veterancy, 6)
{
	GET(BuildingClass *, Item, ESI);
	bool apply_normal_logic = 1;

	// wraps around the scanning for infiltrated warfactory, which should make this veteran if it's trainable and can UndeploysInto= something
	return (apply_normal_logic) ? 0 : 0x442D1B;
}

A_FINE_HOOK(413FA3, AircraftClass_Init_Veterancy, 5)
{
	GET(AircraftClass *, Item, ESI);
	bool apply_normal_logic = 1;

	// wraps around VeteranAircraft scan
	return (apply_normal_logic) ? 0 : 0x413FD2;
}

A_FINE_HOOK(517CF1, InfantryClass_Init, 5)
{
	GET(InfantryClass *, Item, ESI);
	bool apply_normal_logic = 1;

	// wraps around VeteranInfantry and infiltrated barracks scans
	return (apply_normal_logic) ? 0 : 0x517D51;
}

A_FINE_HOOK(746831, UnitClass_Init, 5)
{
	GET(UnitClass *, Item, ESI);
	bool apply_normal_logic = 1;

	// wraps around VeteranUnits and infiltrated warfac scans
	return (apply_normal_logic) ? 0 : 0x74689B;
}

/* #283 - overpowerable cloak generators */
// DO NOT USE YET - the ranges are only updated when building is placed/removed, needs updating on deploy as well
A_FINE_HOOK(44E29D, BuildingClass_Unload, 6)
{
	GET(BuildingClass *, Item, EBP);

	return 0;
}

// determining the maximum size of cloak buffers needed
A_FINE_HOOK(46546C, AllocateCloakBuffer_ToTile, 7)
{
	GET(BuildingTypeClass *, BType, EAX);
	int radius = BType->CloakRadiusInCells; // max (normal radius, deployed radius)
	R->set_EAX(radius);
	return 0x465473;
}

// determining radius of indicator
A_FINE_HOOK(45673E, BuildingClass_GetRangeOfRadial, 7)
{
	GET(BuildingClass *, Building, ESI);
	int radius = Building->Type->CloakRadiusInCells; // max (normal radius, deployed radius)
	R->set_EAX(radius);
	return 0x456745;
}

/* #617 - robot control centers */
// hook also into technoclass::ctor/dtor to update power requirements, or use the hook for #347 below
// don't forget RegisteredAsPoweredUnitSource

// structure with a PowersUnit= just came online
A_FINE_HOOK(4549D1, BuildingClass_UpdatePowered_PowersUnit_Online, A)
{
	GET(BuildingClass *, Structure, ESI);
	return 0x4549F8;
}

// structure with a PowersUnit= just came offline

A_FINE_HOOK(454B16, BuildingClass_UpdatePowered_PowersUnit_Offline, A)
{
	GET(BuildingClass *, Structure, ESI);
	return 0x454B3D;
}

// structure with a PowersUnit= was destroyed
A_FINE_HOOK(502645, HouseClass_RegisterTechnoLoss_PowersUnit, 6)
{
	GET(BuildingClass *, Structure, ESI);
	return 0x50277B;
}

// do we have powered centers for this type?
A_FINE_HOOK(50E1B0, HouseClass_HasPoweredCenters, 6)
{
	GET(HouseClass *, Owner, ECX);
	GET_STACK(TechnoTypeClass *, Type, 0x4);

	bool Online = 1;

	R->set_AL(Online);

	return 0x50E1BD;
}

/* #346 - Veterancy from MC */
A_FINE_HOOK(702FFC, TechnoClass_RegisterDestruction, 6)
{
	GET(TechnoClass *, Victim, ESI);
	GET(TechnoClass *, Killer, EDI);
	// normal veterancy has been awarded already
	return 0;
}

/* #347 - Power= tag on units */
A_FINE_HOOK(508D4A, HouseClass_Update_Power, 6)
{
	GET(HouseClass *, House, ESI);
	// loop over UnitClass::Array , count Power values, House->PowerOutput += x; House->PowerDrain += y;
	return 0;
}

/* #407 - Flash RGB */
	// if (has custom tint) currentTint |= Drawing::Color16bit(tint);
A_FINE_HOOK(73C15F, UnitClass_DrawVXL_Tint, 7)
{
	GET(UnitClass *, Item, EBP);
	DWORD currentTint = R->get_ESI();

	R->set_ESI(currentTint);
	return 0;
}

A_FINE_HOOK(43D52D, BuildingClass_Draw_Tint, 5)
{
	GET(BuildingClass *, Item, ESI);
	DWORD currentTint = R->get_EDI();

	R->set_EDI(currentTint);
	return 0;
}


A_FINE_HOOK(5190C5, InfantryClass_Draw_Tint, 5)
{
	GET(InfantryClass *, Item, ESI);
	DWORD currentTint = R->get_StackVar32(0x18);

	R->set_StackVar32(currentTint, 0x18);
	return 0;
}

/* #468 - Deglobalize WarpIn/Out */
A_FINE_HOOK(719439, TeleportLocomotionClass_ILocomotion_Update_1, 6)
{
	GET(LocomotionClass *, Loco, ESI);
	AnimTypeClass *Anim = NULL;
	// WarpOut
	R->set_EDX((DWORD)Anim);
	return 0x71943F;
}

A_FINE_HOOK(719788, TeleportLocomotionClass_ILocomotion_Update_2, 6)
{
	GET(LocomotionClass *, Loco, ESI);
	AnimTypeClass *Anim = NULL;
	// WarpIn
	R->set_EDX((DWORD)Anim);
	return 0x71978E;
}

A_FINE_HOOK(71986A, TeleportLocomotionClass_ILocomotion_Update_3, 6)
{
	GET(LocomotionClass *, Loco, ESI);
	AnimTypeClass *Anim = NULL;
	// WarpOut again
	R->set_ECX((DWORD)Anim);
	return 0x719870;
}

/* #599 - new alternate arts */
	// sprintf newly formed file basename back into ObjectType::ImageFile
	// might need a different method for VXL to avoid duplicating all related files
	/*
	if(!ObjType->ArcticArtInUse) {
		char buffer[0x19];
		sprintf(buffer, "%s%s", ObjType->ImageFile, custom extension);
		strcpy(ObjType->ImageFile, buffer);
		ObjType->ArcticArtInUse = 1;
	}
	 */


// for SHPs
A_FINE_HOOK(5F907A, ObjectTypeClass_Load2DArt, 6)
{
	GET(ObjectTypeClass *, ObjType, EBP);

	return 0x5F9119;
}


A_FINE_HOOK(5F811E, ObjectTypeClass_Load3DArt, 6)
{
	GET(ObjectTypeClass *, ObjType, ESI);

	return 0;
}

/* #594 - Custom IFV Names */
A_FINE_HOOK(746BCB, UnitClass_GetUIName, 4)
{
	GET(UnitClass *, Unit, ESI);
	GET(InfantryClass *, Gunner, EDI);
	GET(int, ActiveTurretIndex, EAX);
/*
	if(have custom name) {
		Unit->set_ToolTipText(some_custom_text);
		R->set_EAX((DWORD)Unit->get_ToolTipText());
		return 0x746C76;
	}
	return ActiveTurretIndex ? 0x746BE7 : 0x746BCF;
*/
}

/* #453 - crate logic */
// see http://forums.renegadeprojects.com/showthread.php?tid=1128

// replaces crate result if , i.e. an Elite unit finds a veterancy crate
A_FINE_HOOK(481B22, CellClass_CrateBeingCollected_Part1_Overrides, 5)
{
	GET(Powerup, RandomPowerup, EBX);
	GET(CellClass *, pThis, ESI);
	bool Handled = 0;
	return Handled ? 0x481D86 : 0;
}

A_FINE_HOOK(481DB8, CellClass_CrateBeingCollected_Part2_ActualEffects, 5)
{
	GET(Powerup, RandomPowerup, EBX);
	GET(CellClass *, pThis, ESI);
	bool Handled = 0;
	return Handled ? 0x483389 : 0;
}

/* outrageously delicious
 * start for multicell units
 * marks cells as occupied, mouseover events work
 * pathfinding of other units around this one works
 * pathfinding of this unit around others doesn't
 * 	click target becomes {0, 0} and unit disregards its own child cells when pathfinding
 * 	so can park {0, 0} next to a structure, then {1, 0} ends up inside said structure and unit explodes upon stopping
 * */
A_FINE_HOOK(5F7641, ObjectTypeClass_GetFoundationData, 6)
{
	CellStruct *StubFoundation = (CellStruct *)0xAC1438;
	CellStruct *TestFoundation = (CellStruct *)0x89C978;
	GET_STACK(ObjectTypeClass *, ObjType, 0x0);

	if(ObjType && ObjType->WhatAmI() == AbstractType::UnitType && strcmp(ObjType->ID, "MTNK") == 0) {
		Debug::Log("EHLO\n");
		R->EAX<CellStruct *>(TestFoundation);
		return 0x5F7697;
	} else {
		R->EAX<CellStruct *>(StubFoundation);
		return 0;
	}
}
#endif
