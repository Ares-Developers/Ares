#include "Body.h"

#include "../Rules/Body.h"
#include "../../Misc/TrajectoryHelper.h"

#include <AnimClass.h>
#include <DiskLaserClass.h>

DEFINE_HOOK_AGAIN(6F7631, TechnoClass_IsCloseEnoughToTarget_Obstacle, 6)
DEFINE_HOOK(6F7511, TechnoClass_IsCloseEnoughToTarget_Obstacle, 6)
{
	GET_BASE(WeaponTypeClass* const, pWeapon, 0x10);
	GET_BASE(AbstractClass* const, pTarget, 0xC);
	GET(CoordStruct const* const, pSource, ESI);
	REF_STACK(CoordStruct const, dest, STACK_OFFS(0x3C, 0x1C));

	auto const pThis = (R->Origin() == 0x6F7631)
		? R->EDI<TechnoClass*>()
		: R->EAX<TechnoClass*>();

	R->EAX(AresTrajectoryHelper::FindFirstImpenetrableObstacle(
		*pSource, dest, pThis, pTarget, pWeapon, pThis->Owner));

	return 0x6F7647;
}

DEFINE_HOOK(4A76ED, DiskLaserClass_Update_Anim, 7)
{
	GET(DiskLaserClass* const, pThis, ESI);
	REF_STACK(CoordStruct, coords, STACK_OFFS(0x54, 0x1C));

	auto const pWarhead = pThis->Weapon->Warhead;

	if(RulesExt::Global()->DiskLaserAnimEnabled) {
		auto const pType = MapClass::SelectDamageAnimation(
			pThis->Damage, pWarhead, LandType::Clear, coords);

		if(pType) {
			GameCreate<AnimClass>(pType, coords);
		}
	}

	MapClass::FlashbangWarheadAt(pThis->Damage, pWarhead, coords);

	return 0;
}
