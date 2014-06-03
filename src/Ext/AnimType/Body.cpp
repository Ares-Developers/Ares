#include "Body.h"
#include "../../Ares.h"
#include "../../Utilities/TemplateDef.h"
#include <AnimClass.h>
#include <HouseTypeClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

template<> const DWORD Extension<AnimTypeClass>::Canary = 0xEEEEEEEE;
Container<AnimTypeExt> AnimTypeExt::ExtMap;

template<> AnimTypeExt::TT *Container<AnimTypeExt>::SavingObject = nullptr;
template<> IStream *Container<AnimTypeExt>::SavingStream = nullptr;

void AnimTypeExt::ExtData::LoadFromINIFile(AnimTypeClass *pThis, CCINIClass *pINI)
{
	if(pINI->ReadString(pThis->ID, "MakeInfantryOwner", "", Ares::readBuffer, Ares::readLength)) {
		// fugly. C++ needs switch over strings.
		if(strcmp(Ares::readBuffer, "invoker") == 0) {
			this->MakeInfantryOwner = MakeInfantryHouse::Invoker;
		} else if(strcmp(Ares::readBuffer, "killer") == 0) {
			this->MakeInfantryOwner = MakeInfantryHouse::Killer;
		} else if(strcmp(Ares::readBuffer, "victim") == 0) {
			this->MakeInfantryOwner = MakeInfantryHouse::Victim;
		} else if(strcmp(Ares::readBuffer, "neutral") == 0) {
			this->MakeInfantryOwner = MakeInfantryHouse::Neutral;
		} else if(strcmp(Ares::readBuffer, "random") == 0) {
			this->MakeInfantryOwner = MakeInfantryHouse::Random;
		} else {
			Debug::INIParseFailed(pThis->ID, "MakeInfantryOwner", Ares::readBuffer);
		}
	}

	this->Palette.LoadFromINI(pINI, pThis->ID, "CustomPalette");
}

AnimTypeExt::ExtData::MakeInfantryHouse AnimTypeExt::SetMakeInfOwner(AnimClass *pAnim, HouseClass *pInvoker, HouseClass *pVictim, HouseClass *pKiller)
{
	auto pAnimData = AnimTypeExt::ExtMap.Find(pAnim->Type);

	HouseClass *newOwner = nullptr;
	switch(pAnimData->MakeInfantryOwner) {
	case ExtData::MakeInfantryHouse::Neutral:
		newOwner = HouseClass::FindNeutral();
		break;

	case ExtData::MakeInfantryHouse::Random:
		newOwner = HouseClass::Array->GetItem(ScenarioClass::Instance->Random.RandomRanged(0, HouseClass::Array->Count - 1));
		break;

	case ExtData::MakeInfantryHouse::Victim:
		newOwner = pVictim;
		break;

	case ExtData::MakeInfantryHouse::Invoker:
		newOwner = pInvoker;
		break;

	case ExtData::MakeInfantryHouse::Killer:
	default:
		newOwner = pKiller;
		break;
	}

	if(newOwner) {
		pAnim->Owner = newOwner;
		if(pAnim->Type->MakeInfantry > -1) {
			pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;
		}
	}

	return pAnimData->MakeInfantryOwner;
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
	GET_STACK(AnimTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<AnimTypeExt>::PrepareStream(pItem, pStm);

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
