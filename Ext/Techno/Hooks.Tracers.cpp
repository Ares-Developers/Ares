#include "Body.h"
#include "../TechnoType/Body.h"
#include "../../Misc/Debug.h"

#if 0
A_FINE_HOOK(6F2B40, TechnoClass_CTOR_Log, 6)
{
	GET(TechnoClass *, pThis, ECX);
	Debug::Log("CTOR: %X", pThis);
	Debug::DumpStack(R, 0x20);
	return 0;
}

A_FINE_HOOK(6F4500, TechnoClass_DTOR_Log, 5)
{
	GET(TechnoClass *, pThis, ECX);
	Debug::Log("DTOR: %X", pThis);
	Debug::DumpStack(R, 0x20);
	return 0;
}
#endif;
