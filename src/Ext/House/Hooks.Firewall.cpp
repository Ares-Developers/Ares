#include <AnimClass.h>
#include <BulletClass.h>
#include <BuildingClass.h>
#include <CellClass.h>

#include "Body.h"
#include "../BuildingType/Body.h"
#include "../Building/Body.h"
#include "../House/Body.h"
#include "../BulletType/Body.h"
#include "../TechnoType/Body.h"

DEFINE_HOOK_AGAIN(6FF860, TechnoClass_Fire_FSW, 8)
DEFINE_HOOK(6FF008, TechnoClass_Fire_FSW, 8)
{
	REF_STACK(CoordStruct const, src, 0x44);
	REF_STACK(CoordStruct const, tgt, 0x88);

	if(!HouseExt::IsAnyFirestormActive) {
		return 0;
	}

	auto const Bullet = R->Origin() == 0x6FF860
		? R->EDI<BulletClass*>()
		: R->EBX<BulletClass*>()
	;

	auto const pBulletData = BulletTypeExt::ExtMap.Find(Bullet->Type);
	if(!pBulletData->SubjectToFirewall) {
		return 0;
	}

//	check the path of the projectile to see if there are any firestormed cells along the way
//	if so, redirect the proj to the nearest one so it crashes
//	this is technically only necessary for invisible projectiles which don't move to the target
//	- the BulletClass::Update hook above wouldn't work for them

// screw having two code paths

	auto const crd = MapClass::Instance->FindFirstFirestorm(src, tgt, Bullet->Owner->Owner);

	if(crd != CoordStruct::Empty) {
		auto const pCell = MapClass::Instance->GetCellAt(crd);
		Bullet->Target = pCell->GetContent();
		Bullet->Owner->ShouldLoseTargetNow = 1;
//		Bullet->Owner->SetTarget(nullptr);
//		Bullet->Owner->Scatter(0xB1CFE8, 1, 0);
	}

	return 0;
}
