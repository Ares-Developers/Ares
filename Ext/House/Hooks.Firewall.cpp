#include <AnimClass.h>
#include <BulletClass.h>
#include <BuildingClass.h>
#include <CellClass.h>

#include "../../Misc/Applicators.h"

#include "Body.h"
#include "../BuildingType/Body.h"
#include "../Building/Body.h"
#include "../House/Body.h"
#include "../BulletType/Body.h"
#include "../TechnoType/Body.h"

DEFINE_HOOK(6FF008, TechnoClass_Fire_FSW, 8)
DEFINE_HOOK_AGAIN(6FF860, TechnoClass_Fire_FSW, 8)
{
	CoordStruct src = *R->lea_Stack<CoordStruct *>(0x44);
	CoordStruct tgt = *R->lea_Stack<CoordStruct *>(0x88);

	BulletClass * Bullet = R->get_Origin() == 0x6FF860 ? R->EDI<BulletClass *>() : R->EBX<BulletClass *>();

	BulletTypeExt::ExtData *pBulletData = BulletTypeExt::ExtMap.Find(Bullet->Type);

	bool FirestormActive = 0;
	for(int i = 0; i < HouseClass::Array->Count; ++i) {
		HouseExt::ExtData *pData = HouseExt::ExtMap.Find(HouseClass::Array->Items[i]);
		if(pData && pData->FirewallActive) {
			FirestormActive = 1;
			break;
		}
	}

	if(!FirestormActive || !pBulletData->SubjectToFirewall) {
		return 0;
	}

//	check the path of the projectile to see if there are any firestormed cells along the way
//	if so, redirect the proj to the nearest one so it crashes
//	this is technically only necessary for invisible projectiles which don't move to the target
//	- the BulletClass::Update hook above wouldn't work for them

// screw having two code paths

	FirestormFinderApplicator FireFinder(Bullet->Owner->Owner);

	CellSequence Path(&src, &tgt);

	Path.Apply(FireFinder);

	if(FireFinder.found) {
		CellClass::Cell2Coord(&FireFinder.target, &tgt);
		Bullet->Target = MapClass::Global()->GetCellAt(&tgt)->GetContent();
		Bullet->Owner->ShouldLoseTargetNow = 1;
//		Bullet->Owner->SetTarget(NULL);
//		Bullet->Owner->Scatter(0xB1CFE8, 1, 0);
	}

	return 0;
}

DEFINE_HOOK(4DA53E, FootClass_Update, 6)
{
	GET(FootClass *, F, ESI);

	CellClass *C = F->GetCell();
	if(BuildingClass * B = C->GetBuilding()) {
		BuildingTypeClass *BT = B->Type;
		HouseClass *H = B->Owner;
		BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(BT);
		HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(B->Owner);
		if(pTypeData->Firewall_Is && pHouseData->FirewallActive && !F->InLimbo && F->IsAlive && F->Health) {
			int Damage = F->Health;
			F->ReceiveDamage(&Damage, 0, RulesClass::Global()->C4Warhead, 0, 1, 0, H);
			if(AnimTypeClass *FSAnim = AnimTypeClass::Find(F->IsInAir() ? "FSAIR" : "FSGRND")) {
				CoordStruct XYZ;
				F->GetCoords(&XYZ);
				AnimClass * placeholder;
				GAME_ALLOC(AnimClass, placeholder, FSAnim, &XYZ);
			}
		}
	}

	return 0;
}

DEFINE_HOOK(4F8440, HouseClass_Update_FSW_Recalc, 5)
{
	GET(HouseClass *, H, ECX);
	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(H);
	if(pHouseData->FirewallRecalc > 0) {
		--pHouseData->FirewallRecalc;
		HouseExt::Firestorm_SetState(H, pHouseData->FirewallActive);
	} else if(pHouseData->FirewallRecalc < 0) {
		pHouseData->FirewallRecalc = 0;
	}
	return 0;
}

DEFINE_HOOK(4F8C97, HouseClass_Update_FSW_LowPower, 6)
{
	GET(HouseClass *, H, ESI);
	HouseExt::Firestorm_SetState(H, 0);
	return 0;
}

