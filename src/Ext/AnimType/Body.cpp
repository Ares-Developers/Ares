#include "Body.h"
#include "../House/Body.h"
#include "../../Ares.h"
#include "../../Utilities/TemplateDef.h"
#include <AnimClass.h>
#include <HouseTypeClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

template<> const DWORD Extension<AnimTypeClass>::Canary = 0xEEEEEEEE;
AnimTypeExt::ExtContainer AnimTypeExt::ExtMap;

void AnimTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	const char* pID = this->OwnerObject()->ID;

	INI_EX exINI(pINI);

	this->MakeInfantryOwner.Read(exINI, pID, "MakeInfantryOwner");

	this->Palette.LoadFromINI(pINI, pID, "CustomPalette");
}

OwnerHouseKind AnimTypeExt::SetMakeInfOwner(AnimClass *pAnim, HouseClass *pInvoker, HouseClass *pVictim, HouseClass *pKiller)
{
	auto pAnimData = AnimTypeExt::ExtMap.Find(pAnim->Type);

	auto newOwner = HouseExt::GetHouseKind(pAnimData->MakeInfantryOwner, true,
		nullptr, pInvoker, pKiller, pVictim);

	if(newOwner) {
		pAnim->Owner = newOwner;
		if(pAnim->Type->MakeInfantry > -1) {
			pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;
		}
	}

	return pAnimData->MakeInfantryOwner;
}

// =============================
// container

AnimTypeExt::ExtContainer::ExtContainer() : Container("AnimTypeClass") {
}

AnimTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// load / save

template <typename T>
void AnimTypeExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->MakeInfantryOwner)
		.Process(this->Palette);
}

void AnimTypeExt::ExtData::LoadFromStream(AresStreamReader &Stm) {
	Extension<AnimTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AnimTypeExt::ExtData::SaveToStream(AresStreamWriter &Stm) {
	Extension<AnimTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container hooks

DEFINE_HOOK(42784B, AnimTypeClass_CTOR, 5)
{
	GET(AnimTypeClass*, pItem, EAX);

	AnimTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(428EA8, AnimTypeClass_SDDTOR, 5)
{
	GET(AnimTypeClass*, pItem, ECX);

	AnimTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(428970, AnimTypeClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(428800, AnimTypeClass_SaveLoad_Prefix, A)
{
	GET_STACK(AnimTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	AnimTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK_AGAIN(42892C, AnimTypeClass_Load_Suffix, 6)
DEFINE_HOOK(428958, AnimTypeClass_Load_Suffix, 6)
{
	AnimTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(42898A, AnimTypeClass_Save_Suffix, 3)
{
	AnimTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(4287E9, AnimTypeClass_LoadFromINI, A)
DEFINE_HOOK(4287DC, AnimTypeClass_LoadFromINI, A)
{
	GET(AnimTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xBC);

	AnimTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
