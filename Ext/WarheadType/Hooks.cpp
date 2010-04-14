#include <InfantryClass.h>
#include <IonBlastClass.h>
#include <ScenarioClass.h>
#include <WeaponTypeClass.h>
#include <HouseTypeClass.h>
#include <HouseClass.h>
#include <SideClass.h>
#include "Body.h"
#include "../Bullet/Body.h"
#include "../../Enum/ArmorTypes.h"

// feature #384: Permanent MindControl Warheads + feature #200: EMP Warheads
// attach #407 here - set TechnoClass::Flashing.Duration // that doesn't exist, according to yrpp::TechnoClass.h::struct FlashData
// attach #561 here, reuse #407's additional hooks for colouring
DEFINE_HOOK(46920B, BulletClass_Fire, 6) {
	GET(BulletClass *, Bullet, ESI);
	//LEA_STACK(CoordStruct *, detonationXYZ, 0xAC); // looks unused?
	WarheadTypeClass *pThis = Bullet->WH;

	CoordStruct coords;
	if (Bullet->Target) {
		Bullet->Target->GetCoords(&coords);
	} else {
		Bullet->GetCoords(&coords);
	}

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	pWHExt->applyRipples(&coords);
	pWHExt->applyIronCurtain(&coords, Bullet->Owner->Owner);
	pWHExt->applyEMP(&coords);
	WarheadTypeExt::applyOccupantDamage(Bullet);

	return pWHExt->applyPermaMC(&coords, Bullet->Owner->Owner, Bullet->Target) ? 0x469AA4 : 0;
}

// issue 472: deglob WarpAway
DEFINE_HOOK(71A87B, TemporalClass_Update_CacheWH, 6) {
	WarheadTypeExt::Temporal_WH = R->EAX<WeaponTypeClass *> ()->Warhead;
	return 0;
}

// issue 472: deglob WarpAway
DEFINE_HOOK(71A900, TemporalClass_Update_WarpAway, 6) {
	WarheadTypeExt::ExtData *pData = WarheadTypeExt::ExtMap.Find(WarheadTypeExt::Temporal_WH);

	R->EDX<AnimTypeClass *> (pData->Temporal_WarpAway);
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

	*Damage = int(*Damage * pData->DeployedDamage);

	return WH // yes, let's make sure the pointer's safe AFTER we've dereferenced it... Failstwood!
		? 0x517FF9
		: 0x518016
	;
}
