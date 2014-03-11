#include "Body.h"

template<> const DWORD Extension<AbstractClass>::Canary = 0xAB5005BA;
Container<AbstractExt> AbstractExt::ExtMap;

template<> AbstractExt::TT *Container<AbstractExt>::SavingObject = nullptr;
template<> IStream *Container<AbstractExt>::SavingStream = nullptr;

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
#endif
