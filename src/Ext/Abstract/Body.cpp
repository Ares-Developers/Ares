#include "Body.h"

template<> const DWORD Extension<AbstractClass>::Canary = 0xAB5005BA;
Container<AbstractExt> AbstractExt::ExtMap;

template<> AbstractClass* Container<AbstractExt>::SavingObject = nullptr;
template<> IStream *Container<AbstractExt>::SavingStream = nullptr;

// =============================
// container hooks

#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
A_FINE_HOOK(4101B6, AbstractClass_CTOR, 1)
{
	GET(AbstractClass*, pItem, EAX);

	AbstractExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

A_FINE_HOOK(4101F0, AbstractClass_DTOR, 6)
{
	GET(AbstractClass*, pItem, ECX);

	AbstractExt::ExtMap.Remove(pItem);
	return 0;
}

A_FINE_HOOK_AGAIN(410320, AbstractClass_SaveLoad_Prefix, 5)
A_FINE_HOOK(410380, AbstractClass_SaveLoad_Prefix, 5)
{
	GET_STACK(AbstractClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<AbstractExt>::PrepareStream(pItem, pStm);

	return 0;
}

A_FINE_HOOK(4103D6, AbstractClass_Load_Suffix, 4)
{
	AbstractExt::ExtMap.LoadStatic();
	return 0;
}

A_FINE_HOOK(410372, AbstractClass_Save_Suffix, 5)
{
	AbstractExt::ExtMap.SaveStatic();
	return 0;
}
#endif
