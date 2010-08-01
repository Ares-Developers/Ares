#include "Body.h"
#include <ScenarioClass.h>
#include <AnimClass.h>

// =============================
// other hooks

DEFINE_HOOK(445F80, BuildingClass_ChangeOwnership, 5)
{
	GET(BuildingClass *, pThis, ESI);
	if(pThis->Type->SecretLab) {
		BuildingTypeExt::UpdateSecretLabOptions(pThis);
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
	if(pThis->Foundation == FOUNDATION_CUSTOM) {
		BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis);

		R->EAX(pData->CustomHeight == 1 && pData->CustomWidth == 1);
		return 0x465D6D;
	}
	return 0;
}

DEFINE_HOOK(465550, sub_465550, 6)
{
	GET(BuildingTypeClass *, pThis, ECX);
	if(pThis->Foundation == FOUNDATION_CUSTOM) {
		BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis);

		R->EAX(&pData->OutlineData);
		return 0x46556D;
	}
	return 0;
}

DEFINE_HOOK(464AF0, BuildingTypeClass_GetSizeInLeptons, 6)
{
	GET(BuildingTypeClass *, pThis, ECX);
	if(pThis->Foundation == FOUNDATION_CUSTOM) {
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
	if(pThis->Foundation == FOUNDATION_CUSTOM) {
		BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis);

		R->EAX(pData->CustomWidth);
		return 0x45ECED;
	}
	return 0;
}
