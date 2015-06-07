#include "Body.h"
#include "../TechnoType/Body.h"
#include "../HouseType/Body.h"

#include <AnimClass.h>
#include <AircraftClass.h>
#include <Conversions.h>
#include <HouseClass.h>

/* #604 - customizable parachutes */
DEFINE_HOOK(5F5ADD, Parachute_Animation, 6)
{
	GET(TechnoClass *, T, ESI);

	if(!generic_cast<FootClass*>(T)) {
		return 0;
	}

	TechnoTypeExt::ExtData *pTypeData = TechnoTypeExt::ExtMap.Find(T->GetTechnoType());
	if(pTypeData->Is_Bomb) {
		T->IsABomb = 1;
	}

	AnimTypeClass* pAnim = pTypeData->Parachute_Anim;
	if(!pAnim) {
		if(HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(T->Owner->Type)) {
			pAnim = pData->GetParachuteAnim();
		}
	}

	R->EDX<AnimTypeClass *>(pAnim);
	return 0x5F5AE3;
}

DEFINE_HOOK(73C725, UnitClass_DrawSHP_DrawShadowEarlier, 6)
{
	GET(UnitClass *, U, EBP);

	auto pData = TechnoExt::ExtMap.Find(U);

	DWORD retAddr = (U->IsClearlyVisibleTo(HouseClass::Player))
		? 0
		: 0x73CE0D
	;

	// TODO: other conditions where it would not make sense to draw shadow
	switch(U->VisualCharacter(VARIANT_FALSE, nullptr)) {
		case VisualType::Normal:
		case VisualType::Indistinct:
			break;
		default:
			return retAddr;
	}

	if(U->CloakState != CloakState::Uncloaked || U->Type->Underwater || U->Type->SmallVisceroid || U->Type->LargeVisceroid) {
		return retAddr;
	}

	GET(SHPStruct *, Image, EDI);

	if(Image) { // bug #960
		GET(int, FrameToDraw, EBX);
		GET_STACK(Point2D, coords, 0x12C);
		LEA_STACK(RectangleStruct *, BoundingRect, 0x134);

		if(U->IsOnCarryall) {
			coords.Y -= 14;
		}

		Point2D XYAdjust = {0, 0};
		U->Locomotor->Shadow_Point(&XYAdjust);
		coords += XYAdjust;

		int ZAdjust = U->GetZAdjustment() - 2;

		FrameToDraw += Image->Frames / 2;

		DSurface::Hidden_2->DrawSHP(FileSystem::THEATER_PAL, Image, FrameToDraw, &coords, BoundingRect, BlitterFlags(0x2E01),
				0, ZAdjust, 0, 1000, 0, 0, 0, 0, 0);

		pData->ShadowDrawnManually = true;
	}

	return retAddr;
}

DEFINE_HOOK(73C733, UnitClass_DrawSHP_SkipTurretedShadow, 7)
{
	return 0x73C7AC;
}

DEFINE_HOOK(705FF3, TechnoClass_Draw_A_SHP_File_SkipUnitShadow, 6)
{
	GET(TechnoClass *, T, ESI);
	auto pData = TechnoExt::ExtMap.Find(T);
	if(pData->ShadowDrawnManually) {
		pData->ShadowDrawnManually = false;
		return 0x706007;
	}
	return 0;
}

/*
 * this was the old implementation of apcw, no longer needed
A_FINE_HOOK(73B672, UnitClass_DrawVXL, 6)
{
	GET(UnitClass *, U, EBP);
	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(U->Type);
	if(pData->WaterAlt) {
		if(!U->OnBridge && U->GetCell()->LandType == lt_Water) {
			R->EAX(0);
			return 0x73B68B;
		}
	}
	return 0;
}
*/

DEFINE_HOOK(73B4A0, UnitClass_DrawVXL_WaterType, 9)
{
	R->ESI(0);
	GET(UnitClass *, U, EBP);
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(U);

	ObjectTypeClass * Image = U->Type;

	if(!U->IsClearlyVisibleTo(HouseClass::Player)) {
		Image = U->GetDisguise(true);
	}

	if(U->Deployed) {
		if(TechnoTypeClass * Unloader = U->Type->UnloadingClass) {
			Image = Unloader;
		}
	}

	if(UnitTypeClass * pCustomType = pData->GetUnitType()) {
		Image = pCustomType;
	}

	R->EBX<ObjectTypeClass *>(Image);
	return 0x73B4DA;
}


DEFINE_HOOK(73C5FC, UnitClass_DrawSHP_WaterType, 6)
{
	GET(UnitClass *, U, EBP);
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(U);

	SHPStruct * Image = U->GetImage();

	if(UnitTypeClass * pCustomType = pData->GetUnitType()) {
		Image = pCustomType->GetImage();
	}

	if(Image) {
		R->EAX<SHPStruct *>(Image);
		return 0x73C602;
	}
	return 0x73CE00;
}

DEFINE_HOOK_AGAIN(73C69D, UnitClass_DrawSHP_ChangeType1, 6)
DEFINE_HOOK_AGAIN(73C702, UnitClass_DrawSHP_ChangeType1, 6)
DEFINE_HOOK(73C655, UnitClass_DrawSHP_ChangeType1, 6)
{
	GET(UnitClass *, U, EBP);
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(U);

	if(UnitTypeClass * pCustomType = pData->GetUnitType()) {
		R->ECX<UnitTypeClass *>(pCustomType);
		return R->Origin() + 6;
	}

	return 0;
}

DEFINE_HOOK(415085, AircraftClass_Update_DamageSmoke, 7)
{
	GET(AircraftClass*, pThis, ESI);
	auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if(pThis->GetHealthPercentage() < RulesClass::Instance->ConditionRed) {
		if(pThis->GetHeight() > 0) {
			if(AnimTypeClass* pType = pExt->SmokeAnim) {
				int dice = ScenarioClass::Instance->Random.RandomRanged(0, 99);
				int chance = (pThis->Health > 0) ? pExt->SmokeChanceRed.Get(10) : pExt->SmokeChanceDead.Get(80);

				if(dice < chance) {
					GameCreate<AnimClass>(pType, pThis->Location);
				}
			}
		}
	}

	return 0x41512C;
}

DEFINE_HOOK(73C613, UnitClass_DrawSHP_FacingsA, 7) {
	GET(UnitClass*, pThis, EBP);

	unsigned int ret = 0;

	auto highest = Conversions::Int2Highest(pThis->Type->Facings);

	// 2^highest is the frame count, 3 means 8 frames
	if(highest >= 3 && !pThis->IsDisguised()) {
		auto offset = 1u << (highest - 3);
		ret = TranslateFixedPoint(16, highest, static_cast<WORD>(pThis->Facing.current().value()), offset);
	}

	R->EBX(ret);
	return 0x73C64B;
}

DEFINE_HOOK(73CD01, UnitClass_DrawSHP_FacingsB, 5)
{
	GET(UnitClass*, pThis, EBP);
	GET(UnitTypeClass*, pType, ECX);
	GET(int, facing, EAX);

	R->ECX(pThis);
	R->EAX(facing + pType->WalkFrames * pType->Facings);

	return 0x73CD06;
}

DEFINE_HOOK(6FF2D1, TechnoClass_Fire_Facings, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	AnimTypeClass* pAnim = nullptr;

	auto highest = Conversions::Int2Highest(pWeapon->Anim.Count);

	// 2^highest is the frame count, 3 means 8 frames
	if(highest >= 3) {
		auto offset = 1u << (highest - 3);
		auto index = TranslateFixedPoint(16, highest, static_cast<WORD>(pThis->GetRealFacing().value()), offset);
		pAnim = pWeapon->Anim.GetItemOrDefault(index);
	} else {
		pAnim = pWeapon->Anim.GetItemOrDefault(0);
	}

	R->EDI(pAnim);
	return 0x6FF31B;
}
