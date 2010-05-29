#include "Body.h"
#include <AnimClass.h>

#if 0
A_FINE_HOOK(4249EC, AnimClass_Update_MakeInf_Owner, 6)
{
	GET(AnimClass *, Anim, ESI);
	GET(HouseClass *, AnimOwner, EAX);

	AnimTypeExt::ExtData *pData = AnimTypeExt::ExtMap.Find(Anim->Type);

	R->EAX<HouseClass *>(AnimOwner);
	return 0;
}
#endif
