/* #633 - spy building infiltration */
A_FINE_HOOK(4571E0, BuildingClass_Infiltrate, 5)
{
	GET(BuildingClass *, EnteredBuilding, ECX);
	GET_STACK(HouseClass *, EnteredBy, 0x4);

	bool apply_normal_infiltration_logic = 1;

	//! RA1-Style Spying, as requested in issue #633
	//! This sets the respective bit to inform the game that a particular house has spied this building.
	//! Knowing that, the game will reveal the current production in this building to the players who have spied it.
	//! In practice, this means: If a player who has spied a factory clicks on that factory, he will see the cameo of whatever is being built in the factory.
	EnteredBuilding->DisplayProductionToHouses |= (1 << EnteredBy->ArrayIndex);

	// wrapper around the entire function, so you better handle _every_ single thing if you return 0
	return (apply_normal_infiltration_logic) ? 0 : 0x45759F;
}

// fix palette for spied factory production cameo drawing
A_FINE_HOOK(43E8D9, BuildingClass_DrawVisible, 6)
{
	R->set_EDX((DWORD)FileSystem::CAMEO_PAL);
	return 0x43E8DF;
}
