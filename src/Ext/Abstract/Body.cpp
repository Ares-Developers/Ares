#include "Body.h"

Container<AbstractExt> AbstractExt::ExtMap;

#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
DEFINE_HOOK(4101B6, AbstractClass_CTOR, 1)
{
	GET(AbstractClass*, pItem, EAX);

	AbstractExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(4101F0, AbstractClass_DTOR, 6)
{
	GET(AbstractClass*, pItem, ECX);

	AbstractExt::ExtMap.Remove(pItem);
	return 0;
}
#endif
