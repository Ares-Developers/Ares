#include "Body.h"

#include "../Building/Body.h"
#include "../House/Body.h"
#include "../Techno/Body.h"
#include "../TechnoType/Body.h"

#include <ScenarioClass.h>
#include <AnimClass.h>

// =============================
// other hooks

DEFINE_HOOK(445F80, BuildingClass_Place, 5)
{
	GET(BuildingClass *, pThis, ECX);
	if(pThis->Type->SecretLab) {
		auto pExt = BuildingExt::ExtMap.Find(pThis);
		pExt->UpdateSecretLab();
	}

	auto bldTTExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	auto bldTExt = TechnoExt::ExtMap.Find(pThis);
	auto pNewOwnerExt = HouseExt::ExtMap.Find(pThis->Owner);

	if (bldTTExt->FactoryOwners_HaveAllPlans) {
		auto &plans = pNewOwnerExt->FactoryOwners_GatheredPlansOf;

		if(!plans.Contains(bldTExt->OriginalHouseType)) {
			plans.push_back(bldTExt->OriginalHouseType);
		}
	}

	return 0;
}

DEFINE_HOOK(43FB6D, BuildingClass_Update_LFP, 6)
{
	GET(BuildingClass*, B, ESI);
	if(B->Type->LaserFencePost) {
		B->CreateEndPost(1);
	}
	return 0;
}

DEFINE_HOOK(465D4A, BuildingType_IsUndeployable, 6)
{
	GET(BuildingTypeClass *, pThis, ECX);
	if(pThis->Foundation == BuildingTypeExt::CustomFoundation) {
		BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis);

		R->EAX(pData->CustomHeight == 1 && pData->CustomWidth == 1);
		return 0x465D6D;
	}
	return 0;
}

DEFINE_HOOK(465550, BuildingTypeClass_GetFoundationOutline, 6)
{
	GET(BuildingTypeClass *, pThis, ECX);
	if(pThis->Foundation == BuildingTypeExt::CustomFoundation) {
		BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis);

		R->EAX(pData->OutlineData.data());
		return 0x46556D;
	}
	return 0;
}

DEFINE_HOOK(464AF0, BuildingTypeClass_GetSizeInLeptons, 6)
{
	GET(BuildingTypeClass *, pThis, ECX);
	if(pThis->Foundation == BuildingTypeExt::CustomFoundation) {
		GET_STACK(CoordStruct *, Coords, 0x4);
		BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis);

		Coords->X = pData->CustomWidth * 256;
		Coords->Y = pData->CustomHeight * 256;
		Coords->Z = BuildingTypeClass::HeightInLeptons * pThis->Height;
		R->EAX(Coords);
		return 0x464B2C;
	}
	return 0;
}

DEFINE_HOOK(45ECE0, BuildingTypeClass_GetMaxPips, 6)
{
	GET(BuildingTypeClass *, pThis, ECX);
	if(pThis->Foundation == BuildingTypeExt::CustomFoundation) {
		BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis);

		R->EAX(pData->CustomWidth);
		return 0x45ECED;
	}
	return 0;
}

DEFINE_HOOK(45F2E1, BuildingTypeClass_Load2DArt_BuildupTime, 6)
{
	GET(BuildingTypeClass* const, pThis, EBP);
	GET_STACK(int const, frames, STACK_OFFS(0x168, 0x158));

	auto const pExt = BuildingTypeExt::ExtMap.Find(pThis);
	auto const buildup_time = static_cast<int>(pExt->BuildupTime.Get(
		RulesClass::Instance->BuildupTime) * 900.0 / frames);

	R->ECX(buildup_time);
	return 0x45F2FE;
}
