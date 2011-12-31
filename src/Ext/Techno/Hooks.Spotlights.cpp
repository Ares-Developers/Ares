#include "Body.h"
#include "../TechnoType/Body.h"

#include <YRMath.h>
#include <HouseClass.h>
#include <TacticalClass.h>

hash_SpotlightExt TechnoExt::SpotlightExt;
BuildingLightClass * TechnoExt::ActiveBuildingLight = NULL;

// just in case
DEFINE_HOOK(0x420F40, Spotlights_UpdateFoo, 0x6)
{
	return Game::CurrentFrameRate >= Game::GetMinFrameRate()
		? 0
		: 0x421346;
}

// bugfix #182: Spotlights cause an IE
DEFINE_HOOK(0x5F5155, ObjectClass_Put, 0x6)
{
	return R->EAX()
		? 0
		: 0x5F5210;
}

DEFINE_HOOK(0x6F6D0E, TechnoClass_Put_1, 0x7)
{
	GET(TechnoClass *, T, ESI);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());

	if(pTypeData->Is_Spotlighted) {
		BuildingLightClass *placeholder;
		GAME_ALLOC(BuildingLightClass, placeholder, T);
		if(BuildingClass * B = specific_cast<BuildingClass *>(T)) {
			if(B->Spotlight) {
				delete B->Spotlight;
			}
			B->Spotlight = placeholder;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F6F20, TechnoClass_Put_2, 0x6)
{
	GET(TechnoClass *, T, ESI);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());

	if(pTypeData->Is_Spotlighted) {
		BuildingLightClass *placeholder;
		GAME_ALLOC(BuildingLightClass, placeholder, T);
		if(BuildingClass * B = specific_cast<BuildingClass *>(T)) {
			if(B->Spotlight) {
				delete B->Spotlight;
			}
			B->Spotlight = placeholder;
		}
	}

	return 0;
}

DEFINE_HOOK(0x441163, BuildingClass_Put_DontSpawnSpotlight, 0x0)
{
	return 0x441196;
}

DEFINE_HOOK(0x435820, BuildingLightClass_CTOR, 0x6)
{
	GET_STACK(TechnoClass *, T, 0x4);
	GET(BuildingLightClass *, BL, ECX);

	hash_SpotlightExt::iterator i = TechnoExt::SpotlightExt.find(T);
	if(i != TechnoExt::SpotlightExt.end()) {
		TechnoExt::SpotlightExt.erase(i);
	}
	TechnoExt::SpotlightExt[T] = BL;
	return 0;
}

DEFINE_HOOK(0x4370C0, BuildingLightClass_SDDTOR, 0xA)
{
	GET(BuildingLightClass *, BL, ECX);

	TechnoClass *T = BL->OwnerObject;
	hash_SpotlightExt::iterator i = TechnoExt::SpotlightExt.find(T);
	if(i != TechnoExt::SpotlightExt.end()) {
		TechnoExt::SpotlightExt.erase(i);
	}
	return 0;
}

DEFINE_HOOK(0x6F4500, TechnoClass_DTOR_Spotlight, 0x5)
{
	GET(TechnoClass*, pItem, ECX);
	hash_SpotlightExt::iterator i = TechnoExt::SpotlightExt.find(pItem);
	if(i != TechnoExt::SpotlightExt.end()) {
		GAME_DEALLOC(i->second);
		TechnoExt::SpotlightExt.erase(i);
	}
	return 0;
}

DEFINE_HOOK(0x70FBE3, TechnoClass_Activate, 0x5)
{
	GET(TechnoClass *, T, ECX);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(T);

	/* Check abort conditions:
		- Is the object currently EMP'd?
		- Does the object need an operator, but doesn't have one?
		- Does the object need a powering structure that is offline?
	   If any of the above conditions, bail out and don't activate the object.
	*/
	if(T->IsUnderEMP() || !pData->IsPowered() || !pData->IsOperated()) {
		return 0x70FC82;
	}

	if(pTypeData->Is_Spotlighted) {
		hash_SpotlightExt::iterator i = TechnoExt::SpotlightExt.find(T);
		if(i != TechnoExt::SpotlightExt.end()) {
			TechnoExt::SpotlightExt.erase(i);
			GAME_DEALLOC(i->second);
		}
		++Unsorted::IKnowWhatImDoing;
		BuildingLightClass *placeholder;
		GAME_ALLOC(BuildingLightClass, placeholder, T);
		if(BuildingClass * B = specific_cast<BuildingClass *>(T)) {
			if(B->Spotlight) {
				delete B->Spotlight;
			}
			B->Spotlight = placeholder;
		}
		--Unsorted::IKnowWhatImDoing;
	}
	return 0;
}

DEFINE_HOOK(0x70FC97, TechnoClass_Deactivate, 0x6)
{
	GET(TechnoClass *, T, ESI);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());

	if(pTypeData->Is_Spotlighted) {
		hash_SpotlightExt::iterator i = TechnoExt::SpotlightExt.find(T);
		if(i != TechnoExt::SpotlightExt.end()) {
//			TechnoExt::SpotlightExt.erase(i);
			GAME_DEALLOC(i->second);
		}
	}
	return 0;
}

DEFINE_HOOK(0x435C08, BuildingLightClass_Draw_ForceType, 0x5)
{
	return 0x435C16;
}

DEFINE_HOOK(0x435C32, BuildingLightClass_Draw_PowerOnline, 0xA)
{
	GET(TechnoClass *, T, EDI);
	return (T->WhatAmI() != abs_Building || (T->IsPowerOnline() && !reinterpret_cast<BuildingClass *>(T)->IsFogged))
		? 0x435C52
		: 0x4361BC
	;
}

DEFINE_HOOK(0x436459, BuildingLightClass_Update, 0x6)
{
	static const double Facing2Rad = (2 * 3.14) / 0xFFFF;
	GET(BuildingLightClass *, BL, EDI);
	TechnoClass *Owner = BL->OwnerObject;
	if(Owner && Owner->WhatAmI() != abs_Building) {
		TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
		CoordStruct Loc = Owner->Location;
		DWORD Facing;
		switch(pTypeData->Spot_AttachedTo) {
			case TechnoTypeExt::sa_Barrel:
				Owner->BarrelFacing.GetFacing(&Facing);
				break;
			case TechnoTypeExt::sa_Turret:
				Owner->TurretFacing.GetFacing(&Facing);
				break;
			default:
				Owner->Facing.GetFacing(&Facing);
		}
		WORD F = (WORD)Facing;

		double Angle = Facing2Rad * F;
		Loc.Y -= pTypeData->Spot_Distance * Math::cos(Angle);
		Loc.X += pTypeData->Spot_Distance * Math::sin(Angle);

		BL->field_B8 = Loc;
		BL->field_C4 = Loc;
//		double zer0 = 0.0;
		__asm { fldz }
	} else {
		double Angle = RulesClass::Instance->SpotlightAngle;
		__asm { fld Angle }
	}

	return R->AL()
		? 0x436461
		: 0x4364C8
	;
}

DEFINE_HOOK(0x435BE0, BuildingLightClass_Draw_Start, 0x6)
{
	GET(BuildingLightClass *, BL, ECX);
	TechnoExt::ActiveBuildingLight = BL;
	return 0;
}

DEFINE_HOOK(0x436072, BuildingLightClass_Draw_430, 0x6)
{
	TechnoClass *Owner = TechnoExt::ActiveBuildingLight->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
	R->EAX((pTypeData ? pTypeData->Spot_Height : 250) + 180);
	return 0x436078;
}

DEFINE_HOOK(0x4360D8, BuildingLightClass_Draw_400, 0x6)
{
	TechnoClass *Owner = TechnoExt::ActiveBuildingLight->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
	R->ECX((pTypeData ? pTypeData->Spot_Height : 250) + 150);
	return 0x4360DE;
}

DEFINE_HOOK(0x4360FF, BuildingLightClass_Draw_250, 0x6)
{
	TechnoClass *Owner = TechnoExt::ActiveBuildingLight->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
	R->ECX(pTypeData ? pTypeData->Spot_Height : 250);
	TechnoExt::ActiveBuildingLight = NULL;
	return 0x436105;
}

DEFINE_HOOK(0x435CD3, SpotlightClass_CTOR, 0x6)
{
	GET_STACK(SpotlightClass *, Spot, 0x14);
	GET(BuildingLightClass *, BL, ESI);

	TechnoClass *Owner = BL->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());

	eSpotlightFlags Flags = 0;
	if(pTypeData->Spot_Reverse) {
		Flags |= sf_NoColor;
	}
	if(pTypeData->Spot_DisableR) {
		Flags |= sf_NoRed;
	}
	if(pTypeData->Spot_DisableG) {
		Flags |= sf_NoGreen;
	}
	if(pTypeData->Spot_DisableB) {
		Flags |= sf_NoBlue;
	}

	Spot->DisableFlags = Flags;

	return 0;
}
