#include "../Ext/Techno/Body.h"
#include "../Ext/TechnoType/Body.h"

#include <HouseClass.h>
#include <Powerups.h>

/*
	generic crate-handler file
	currently used only to shim crates into TechnoExt
	since Techno fields are used by AttachEffect

	Graion Dilach, 2013-05-31
*/

//overrides for crate checks
//481D52 - pass
//481C86 - override with Money

DEFINE_HOOK(481C6C, CellClass_CrateBeingCollected_Armor1, 6)
{
	GET(TechnoClass *, Unit, EDI);
	TechnoExt::ExtData *UnitExt = TechnoExt::ExtMap.Find(Unit);
	if (UnitExt->Crate_ArmorMultiplier == 1.0){
		return 0x481D52;
	}
	return 0x481C86;
}

DEFINE_HOOK(481CE1, CellClass_CrateBeingCollected_Speed1, 6)
{
	GET(FootClass *, Unit, EDI);
	TechnoExt::ExtData *UnitExt = TechnoExt::ExtMap.Find(Unit);
	if (UnitExt->Crate_SpeedMultiplier == 1.0){
		return 0x481D52;
	}
	return 0x481C86;
}

DEFINE_HOOK(481D0E, CellClass_CrateBeingCollected_Firepower1, 6)
{
	GET(TechnoClass *, Unit, EDI);
	TechnoExt::ExtData *UnitExt = TechnoExt::ExtMap.Find(Unit);
	if (UnitExt->Crate_FirepowerMultiplier == 1.0){
		return 0x481D52;
	}
	return 0x481C86;
}

DEFINE_HOOK(481D3D, CellClass_CrateBeingCollected_Cloak1, 6)
{
	GET(TechnoClass *, Unit, EDI);
	TechnoExt::ExtData *UnitExt = TechnoExt::ExtMap.Find(Unit);
	if (TechnoExt::CanICloakByDefault(Unit) || UnitExt->Crate_Cloakable){
		return 0x481C86;
	}

	auto pType = Unit->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	// cloaking forbidden for type
	if(!pTypeExt->CloakAllowed) {
		return 0x481C86;
	}

	return 0x481D52;
}

//overrides on actual crate effect applications

DEFINE_HOOK(48294F, CellClass_CrateBeingCollected_Cloak2, 7)
{
	GET(TechnoClass *, Unit, EDX);
	TechnoExt::ExtData *UnitExt = TechnoExt::ExtMap.Find(Unit);
	UnitExt->Crate_Cloakable = 1;
	UnitExt->RecalculateStats();
	return 0x482956;
}

DEFINE_HOOK(482E57, CellClass_CrateBeingCollected_Armor2, 6)
{
	GET(TechnoClass *, Unit, ECX);
	GET_STACK(double, Pow_ArmorMultiplier, 0x20);
	TechnoExt::ExtData *UnitExt = TechnoExt::ExtMap.Find(Unit);
	if (UnitExt->Crate_ArmorMultiplier == 1.0){
		UnitExt->Crate_ArmorMultiplier = Pow_ArmorMultiplier;
		UnitExt->RecalculateStats();
		R->AL(Unit->GetOwningHouse()->PlayerControl);
		return 0x482E89;
	}
	return 0x482E92;
}


DEFINE_HOOK(48303A, CellClass_CrateBeingCollected_Speed2, 6)
{
	GET(FootClass *, Unit, EDI);
	GET_STACK(double, Pow_SpeedMultiplier, 0x20);
	TechnoExt::ExtData *UnitExt = TechnoExt::ExtMap.Find(Unit);
	if (UnitExt->Crate_SpeedMultiplier == 1.0 && Unit->WhatAmI() != AbstractType::AircraftType){
		UnitExt->Crate_SpeedMultiplier = Pow_SpeedMultiplier;
		UnitExt->RecalculateStats();
		R->CL(Unit->GetOwningHouse()->PlayerControl);
		return 0x483078;
	}
	return 0x483081;
}

DEFINE_HOOK(483226, CellClass_CrateBeingCollected_Firepower2, 6)
{
	GET(TechnoClass *, Unit, ECX);
	GET_STACK(double, Pow_FirepowerMultiplier, 0x20);
	TechnoExt::ExtData *UnitExt = TechnoExt::ExtMap.Find(Unit);
	if (UnitExt->Crate_FirepowerMultiplier == 1.0){
		UnitExt->Crate_FirepowerMultiplier = Pow_FirepowerMultiplier;
		UnitExt->RecalculateStats();
		R->AL(Unit->GetOwningHouse()->PlayerControl);
		return 0x483258;
	}
	return 0x483261;
}

