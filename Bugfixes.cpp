#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif
#pragma warning(disable: 4035)	//"no return value" - there is one, just not in our code ;)

#include <YRPP.h>

// 71A92A, 5
EXPORT _Temporal_AvoidFriendlies(REGISTERS *R)
{
	TemporalClass *m = (TemporalClass *)R->get_ESI(); 

	HouseClass *hv = m->get_TargetUnit()->get_Owner();
	HouseClass *ho = m->get_OwningUnit()->get_Owner();

	if(ho->IsAlliedWith(hv)) {
		return 0x71A97D;
	}

	return 0;
}
