#include "Body.h"
#include <ScenarioClass.h>
#include <AnimClass.h>

// =============================
// other hooks

//hook at 0x45EC90
DEFINE_HOOK(45EC90, Foundations_GetFoundationWidth, 6)
{
	GET(BuildingTypeClass*, pThis, ECX);
	BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis);

	if(pData->IsCustom) {
		R->set_EAX(pData->CustomWidth);
		return 0x45EC9D;
	}

	return 0;
}

DEFINE_HOOK(45ECA0, Foundations_GetFoundationHeight, 6)
{
	GET(BuildingTypeClass*, pThis, ECX);
	BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis);

	if(pData->IsCustom) {
		bool bIncludeBib = (R->get_StackVar8(0x4) != 0 );
		
		int fH = pData->CustomHeight;
		if(bIncludeBib && pThis->Bib) {
			++fH;
		}

		R->set_EAX(fH);
		return 0x45ECDA;
	}

	return 0;
}

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

		R->set_EAX(pData->CustomHeight == 1 && pData->CustomWidth == 1);
		return 0x465D6D;
	}
	return 0;
}

DEFINE_HOOK(465550, sub_465550, 6)
{
	GET(BuildingTypeClass *, pThis, ECX);
	if(pThis->Foundation == FOUNDATION_CUSTOM) {
		BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis);

		R->set_EAX((DWORD)&pData->OutlineData);
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
		R->set_EAX((DWORD)Coords);
		return 0x464B2C;
	}
	return 0;
}

DEFINE_HOOK(45ECE0, BuildingTypeClass_GetMaxPips, 6)
{
	GET(BuildingTypeClass *, pThis, ECX);
	if(pThis->Foundation == FOUNDATION_CUSTOM) {
		BuildingTypeExt::ExtData* pData = BuildingTypeExt::ExtMap.Find(pThis);

		R->set_EAX(pData->CustomWidth);
		return 0x45ECED;
	}
	return 0;
}
