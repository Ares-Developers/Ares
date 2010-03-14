#include "Body.h"

template<> const DWORD Extension<AnimTypeClass>::Canary = 0xEEEEEEEE;
Container<AnimTypeExt> AnimTypeExt::ExtMap;

template<> AnimTypeExt::TT *Container<AnimTypeExt>::SavingObject = NULL;
template<> IStream *Container<AnimTypeExt>::SavingStream = NULL;

// =============================
// container hooks

DEFINE_HOOK(42784B, AnimTypeClass_CTOR, 5)
{
	GET(AnimTypeClass*, pItem, EAX);

	AnimTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(427888, AnimTypeClass_DTOR, 5)
{
	GET(AnimTypeClass*, pItem, ECX);

	AnimTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(428800, AnimTypeClass_SaveLoad_Prefix, A)
DEFINE_HOOK_AGAIN(428970, AnimTypeClass_SaveLoad_Prefix, 8)
{
	GET_STACK(AnimTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<AnimTypeExt>::SavingObject = pItem;
	Container<AnimTypeExt>::SavingStream = pStm;

	return 0;
}

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

DEFINE_HOOK(4287DC, AnimTypeClass_LoadFromINI, A)
DEFINE_HOOK_AGAIN(4287E9, AnimTypeClass_LoadFromINI, A)
{
	GET(AnimTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xBC);

	AnimTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
