#include <InfantryClass.h>
#include <IonBlastClass.h>
#include <ScenarioClass.h>
#include <WeaponTypeClass.h>
#include <HouseTypeClass.h>
#include <HouseClass.h>
#include <SideClass.h>
#include "Body.h"
#include "../Techno/Body.h"
#include "../Bullet/Body.h"
#include "../WeaponType/Body.h"
#include "../../Enum/ArmorTypes.h"

// feature #384: Permanent MindControl Warheads + feature #200: EMP Warheads
// attach #407 here - set TechnoClass::Flashing.Duration // that doesn't exist, according to yrpp::TechnoClass.h::struct FlashData
// attach #561 here, reuse #407's additional hooks for colouring
DEFINE_HOOK(46920B, BulletClass_Detonate, 6) {
	GET(BulletClass* const, pThis, ESI);
	GET_BASE(const CoordStruct* const, pCoordsDetonation, 0x8);

	auto const pWarhead = pThis->WH;
	auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	auto const pOwnerHouse = pThis->Owner ? pThis->Owner->Owner : nullptr;

	// this snapping stuff does not belong here. it should go into BulletClass::Fire
	auto coords = *pCoordsDetonation;
	auto snapped = false;

	static auto const SnapDistance = 64;
	if(pThis->Target && pThis->DistanceFrom(pThis->Target) < SnapDistance) {
		coords = pThis->Target->GetCoords();
		snapped = true;
	}

	// these effects should be applied no matter what happens to the target
	pWHExt->applyRipples(coords);

	bool targetStillOnMap = true;
	if(snapped) {
		if(auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pThis->WeaponType)) {
			targetStillOnMap = !pWeaponExt->conductAbduction(pThis);
		}
	}

	// if the target gets abducted, there's nothing there to apply IC, EMP, etc. to
	// mind that conductAbduction() neuters the bullet, so if you wish to change
	// this check, you have to fix that as well
	if(targetStillOnMap) {
		auto const damage = pThis->WeaponType ? pThis->WeaponType->Damage : 0;
		pWHExt->applyIronCurtain(coords, pOwnerHouse, damage);
		pWHExt->applyEMP(coords, pThis->Owner);
		pWHExt->applyAttachedEffect(coords, pThis->Owner);

		if(snapped) {
			WarheadTypeExt::applyOccupantDamage(pThis);
			pWHExt->applyKillDriver(pThis->Owner, pThis->Target);
		}
	}

	return pWHExt->applyPermaMC(pOwnerHouse, pThis->Target) ? 0x469AA4u : 0u;
}

// issue 472: deglob WarpAway
DEFINE_HOOK(71A900, TemporalClass_Update_WarpAway, 6) {
	auto pData = WarheadTypeExt::ExtMap.Find(WarheadTypeExt::Temporal_WH);
	R->EDX<AnimTypeClass *>(pData->Temporal_WarpAway.Get(RulesClass::Global()->WarpAway));
	return 0x71A906;
}

DEFINE_HOOK(517FC1, InfantryClass_ReceiveDamage_DeployedDamage, 6) {
	GET(InfantryClass *, I, ESI);
	bool IgnoreDefenses = R->BL() != 0;

	if (!I->IsDeployed() || IgnoreDefenses) {
		return 0;
	}
	GET(WarheadTypeClass *, WH, EBP);
	GET(int *, Damage, EDI);

	WarheadTypeExt::ExtData *pData = WarheadTypeExt::ExtMap.Find(WH);

	*Damage = static_cast<int>(*Damage * pData->DeployedDamage);

	// yes, let's make sure the pointer's safe AFTER we've dereferenced it... Failstwood!
	return WH ? 0x517FF9u : 0x518016u;
}
/*
 * Fixing issue #722
 */

DEFINE_HOOK(7384BD, UnitClass_ReceiveDamage_OreMinerUnderAttack, 6)
{
	GET_STACK(WarheadTypeClass *, WH, STACK_OFFS(0x44, -0xC));

	auto pData = WarheadTypeExt::ExtMap.Find(WH);
	return !pData->Malicious ? 0x738535u : 0u;
}

DEFINE_HOOK(4F94A5, HouseClass_BuildingUnderAttack, 6)
{
	GET_STACK(DWORD, Caller, 0x14);
	if(Caller == 0x442980) {
		//Debug::DumpStack(R, 0xF0, 0xA0);
		GET_STACK(WarheadTypeClass *, WH, 0x14 + 0xA4 + 0xC);
		if(auto pData = WarheadTypeExt::ExtMap.Find(WH)) {
			if(!pData->Malicious) {
				return 0x4F95D4;
			}
		}
	}
	return 0;
}

DEFINE_HOOK(702669, TechnoClass_ReceiveDamage_SuppressDeathWeapon, 9)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(WarheadTypeClass* const, pWarhead, STACK_OFFS(0xC4, -0xC));

	auto const pExt = WarheadTypeExt::ExtMap.Find(pWarhead);
	auto const abs = pThis->WhatAmI();

	auto const suppressed =
		(abs == AbstractType::Unit && pExt->SuppressDeathWeapon_Vehicles)
		|| (abs == AbstractType::Infantry && pExt->SuppressDeathWeapon_Infantry)
		|| pExt->SuppressDeathWeapon.Contains(pThis->GetTechnoType());
	
	if(!suppressed) {
		pThis->FireDeathWeapon(0);
	}

	return 0x702672;
}
