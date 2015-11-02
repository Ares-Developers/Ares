#include "Body.h"
#include "../BulletType/Body.h"

#include <AnimClass.h>
#include <BulletClass.h>
#include <ScenarioClass.h>

DEFINE_HOOK(4232CE, AnimClass_Draw_SetPalette, 6)
{
	GET(AnimTypeClass *, AnimType, EAX);

	auto pData = AnimTypeExt::ExtMap.Find(AnimType);

	if(pData->Palette.Convert) {
		R->ECX<ConvertClass *>(pData->Palette.GetConvert());
		return 0x4232D4;
	}

	return 0;
}

DEFINE_HOOK(468379, BulletClass_Draw_SetAnimPalette, 6)
{
	GET(BulletClass *, Bullet, ESI);
	auto pExt = BulletTypeExt::ExtMap.Find(Bullet->Type);

	if(ConvertClass* Convert = pExt->GetConvert()) {
		R->EBX<ConvertClass *>(Convert);
		return 0x4683D7;
	}

	return 0;
}

DEFINE_HOOK_AGAIN(42511B, AnimClass_Expired_ScorchFlamer, 7)
DEFINE_HOOK_AGAIN(4250C9, AnimClass_Expired_ScorchFlamer, 7)
DEFINE_HOOK(42513F, AnimClass_Expired_ScorchFlamer, 7)
{
	GET(AnimClass*, pThis, ESI);
	auto pType = pThis->Type;

	CoordStruct crd = pThis->GetCoords();

	auto SpawnAnim = [&crd](AnimTypeClass* pType, int dist) {
		if(!pType) {
			return static_cast<AnimClass*>(nullptr);
		}

		CoordStruct crdAnim = crd;
		if(dist > 0) {
			auto crdNear = MapClass::GetRandomCoordsNear(crd, dist, false);
			crdAnim = MapClass::PickInfantrySublocation(crdNear, true);
		}

		auto count = ScenarioClass::Instance->Random.RandomRanged(1, 2);
		return GameCreate<AnimClass>(pType, crdAnim, 0, count, 0x600u, 0, false);
	};

	if(pType->Flamer) {
		// always create at least one small fire
		SpawnAnim(RulesClass::Instance->SmallFire, 64);

		// 50% to create another small fire
		if(ScenarioClass::Instance->Random.RandomRanged(0, 99) < 50) {
			SpawnAnim(RulesClass::Instance->SmallFire, 160);
		}

		// 50% chance to create a large fire
		if(ScenarioClass::Instance->Random.RandomRanged(0, 99) < 50) {
			SpawnAnim(RulesClass::Instance->LargeFire, 112);
		}

	} else if(pType->Scorch) {
		// creates a SmallFire anim that is attached to the same object
		// this anim is attached to.
		if(pThis->GetHeight() < 10) {
			switch(pThis->GetCell()->LandType) {
			case LandType::Water:
			case LandType::Beach:
			case LandType::Ice:
			case LandType::Rock:
				break;
			default:
				if(auto pAnim = SpawnAnim(RulesClass::Instance->SmallFire, 0)) {
					if(pThis->OwnerObject) {
						pAnim->SetOwnerObject(pThis->OwnerObject);
					}
				}
			}
		}
	}

	return 0;
}
