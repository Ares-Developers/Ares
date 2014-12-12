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
DEFINE_HOOK(46920B, BulletClass_Fire, 6) {
	GET(BulletClass *, Bullet, ESI);
	//LEA_STACK(CoordStruct *, detonationXYZ, 0xAC); // looks unused?
	WarheadTypeClass *pThis = Bullet->WH;

	CoordStruct coords = Bullet->GetTargetCoords();

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	HouseClass *OwnerHouse = (Bullet->Owner)
		? Bullet->Owner->Owner
		: nullptr
	;

	int damage = 0;
	WeaponTypeExt::ExtData* WeaponTypeExt = nullptr;
	if(Bullet->WeaponType) {
		damage = Bullet->WeaponType->Damage;
		WeaponTypeExt = WeaponTypeExt::ExtMap.Find(Bullet->WeaponType);
	}

	// these effects should be applied no matter what happens to the target
	pWHExt->applyRipples(coords);

	bool targetStillOnMap = true;
	if(WeaponTypeExt) {
		targetStillOnMap = !WeaponTypeExt->conductAbduction(Bullet);
	}

	// if the target gets abducted, there's nothing there to apply IC, EMP, etc. to
	// mind that conductAbduction() neuters the bullet, so if you wish to change
	// this check, you have to fix that as well
	if(targetStillOnMap) {
		pWHExt->applyIronCurtain(coords, OwnerHouse, damage);
		pWHExt->applyEMP(coords, Bullet->Owner);
		WarheadTypeExt::applyOccupantDamage(Bullet);
		pWHExt->applyKillDriver(Bullet->Owner, Bullet->Target);
		pWHExt->applyAttachedEffect(coords, Bullet->Owner);
	}

/*
 * this is a little demo I made to test DP
	if(_strcmpi(Bullet->Type->ID, "Cannon") == 0) {
		UnitClass * Drop = reinterpret_cast<UnitClass *>(UnitTypeClass::Find("APOC")->CreateObject(HouseClass::Player));
		CoordStruct XYZ = coords;
		XYZ.Z += 800;
		if(!TechnoExt::CreateWithDroppod(Drop, &XYZ)) {
			Drop->UnInit();
		}
	}
*/

	return pWHExt->applyPermaMC(OwnerHouse, Bullet->Target) ? 0x469AA4u : 0u;
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

	return WH // yes, let's make sure the pointer's safe AFTER we've dereferenced it... Failstwood!
		? 0x517FF9
		: 0x518016
	;
}
/*
 * Fixing issue #722
 */

DEFINE_HOOK(7384BD, UnitClass_ReceiveDamage_OreMinerUnderAttack, 6)
{
	GET_STACK(WarheadTypeClass *, WH, STACK_OFFS(0x44, -0xC));

	auto pData = WarheadTypeExt::ExtMap.Find(WH);
	return !pData->Malicious
		? 0x738535
		: 0
	;
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
