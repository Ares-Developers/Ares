#include "Body.h"
#include "../TechnoType/Body.h"

#include <YRMath.h>
#include <HouseClass.h>
#include <SpotlightClass.h>
#include <TacticalClass.h>

BuildingLightClass * TechnoExt::ActiveBuildingLight = nullptr;

// just in case
DEFINE_HOOK(420F40, Spotlights_UpdateFoo, 6)
{
	return Game::CurrentFrameRate >= Game::GetMinFrameRate() ? 0u : 0x421346u;
}

// bugfix #182: Spotlights cause an IE
DEFINE_HOOK(5F5155, ObjectClass_Put, 6)
{
	return R->EAX() ? 0u : 0x5F5210u;
}

DEFINE_HOOK(6F6D0E, TechnoClass_Put_1, 7)
{
	GET(TechnoClass *, T, ESI);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());

	if(pTypeData->Is_Spotlighted) {
		auto pExt = TechnoExt::ExtMap.Find(T);
		auto pSpotlight = GameCreate<BuildingLightClass>(T);
		pExt->SetSpotlight(pSpotlight);
	}

	return 0;
}

DEFINE_HOOK(6F6F20, TechnoClass_Put_2, 6)
{
	GET(TechnoClass *, T, ESI);
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());

	if(pTypeData->Is_Spotlighted) {
		auto pExt = TechnoExt::ExtMap.Find(T);
		auto pSpotlight = GameCreate<BuildingLightClass>(T);
		pExt->SetSpotlight(pSpotlight);
	}

	return 0;
}

DEFINE_HOOK(441163, BuildingClass_Put_DontSpawnSpotlight, 0)
{
	return 0x441196;
}

DEFINE_HOOK(435820, BuildingLightClass_CTOR, 6)
{
	GET_STACK(TechnoClass *, T, 0x4);
	GET(BuildingLightClass *, BL, ECX);

	if(auto pExt = TechnoExt::ExtMap.Find(T)) {
		pExt->Spotlight = BL;
	}
	return 0;
}

DEFINE_HOOK(4370C0, BuildingLightClass_SDDTOR, A)
{
	GET(BuildingLightClass*, pThis, ECX);

	if(!Ares::bShuttingDown) {
		auto pTechno = pThis->OwnerObject;
		if(auto pExt = TechnoExt::ExtMap.Find(pTechno)) {
			pExt->Spotlight = nullptr;
		}
	}
	return 0;
}

DEFINE_HOOK(435C08, BuildingLightClass_Draw_ForceType, 5)
{
	return 0x435C16;
}

DEFINE_HOOK(435C32, BuildingLightClass_Draw_PowerOnline, A)
{
	GET(TechnoClass* const, pTechno, EDI);

	if(auto const pBld = abstract_cast<BuildingClass*>(pTechno)) {
		if(!pBld->IsPowerOnline() || pBld->IsFogged) {
			return 0x4361BC;
		}
	}

	return 0x435C52;
}

DEFINE_HOOK(436459, BuildingLightClass_Update, 6)
{
	GET(BuildingLightClass *, BL, EDI);
	TechnoClass *Owner = BL->OwnerObject;
	if(Owner && Owner->WhatAmI() != AbstractType::Building) {
		TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
		CoordStruct Loc = Owner->Location;
		DirStruct Facing;
		switch(pTypeData->Spot_AttachedTo) {
		case TechnoTypeExt::SpotlightAttachment::Barrel:
			Facing = Owner->BarrelFacing.current();
			break;
		case TechnoTypeExt::SpotlightAttachment::Turret:
			Facing = Owner->TurretFacing.current();
			break;
		case TechnoTypeExt::SpotlightAttachment::Body:
		default:
			Facing = Owner->Facing.current();
		}

		static const double Facing2Rad = (2 * 3.14) / 0xFFFF;
		double Angle = Facing2Rad * static_cast<DirStruct::unsigned_type>(Facing.value());
		Loc.Y -= static_cast<int>(pTypeData->Spot_Distance * Math::cos(Angle));
		Loc.X += static_cast<int>(pTypeData->Spot_Distance * Math::sin(Angle));

		BL->field_B8 = Loc;
		BL->field_C4 = Loc;
//		double zer0 = 0.0;
		__asm { fldz }
	} else {
		double Angle = RulesClass::Instance->SpotlightAngle;
		__asm { fld Angle }
	}

	return R->AL() ? 0x436461u : 0x4364C8u;
}

DEFINE_HOOK(435BE0, BuildingLightClass_Draw_Start, 6)
{
	GET(BuildingLightClass *, BL, ECX);
	TechnoExt::ActiveBuildingLight = BL;
	return 0;
}

DEFINE_HOOK(436072, BuildingLightClass_Draw_430, 6)
{
	TechnoClass *Owner = TechnoExt::ActiveBuildingLight->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
	R->EAX((pTypeData ? pTypeData->Spot_Height : 250) + 180);
	return 0x436078;
}

DEFINE_HOOK(4360D8, BuildingLightClass_Draw_400, 6)
{
	TechnoClass *Owner = TechnoExt::ActiveBuildingLight->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
	R->ECX((pTypeData ? pTypeData->Spot_Height : 250) + 150);
	return 0x4360DE;
}

DEFINE_HOOK(4360FF, BuildingLightClass_Draw_250, 6)
{
	TechnoClass *Owner = TechnoExt::ActiveBuildingLight->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());
	R->ECX(pTypeData ? pTypeData->Spot_Height : 250);
	TechnoExt::ActiveBuildingLight = nullptr;
	return 0x436105;
}

DEFINE_HOOK(435CD3, SpotlightClass_CTOR, 6)
{
	GET_STACK(SpotlightClass *, Spot, 0x14);
	GET(BuildingLightClass *, BL, ESI);

	TechnoClass *Owner = BL->OwnerObject;
	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(Owner->GetTechnoType());

	SpotlightFlags Flags = SpotlightFlags::None;
	if(pTypeData->Spot_Reverse) {
		Flags |= SpotlightFlags::NoColor;
	}
	if(pTypeData->Spot_DisableR) {
		Flags |= SpotlightFlags::NoRed;
	}
	if(pTypeData->Spot_DisableG) {
		Flags |= SpotlightFlags::NoGreen;
	}
	if(pTypeData->Spot_DisableB) {
		Flags |= SpotlightFlags::NoBlue;
	}

	Spot->DisableFlags = Flags;

	return 0;
}
