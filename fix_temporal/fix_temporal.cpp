//TEST DLL
//=============
//
// keep this part in
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif
#pragma warning(disable: 4035)	//"no return value" - there is one, just not in our code ;)

// standard C++ headers
#include <stdio.h>

// YR++ library
#include <YRPP.h>

// the actual function 

//hook at 0x71A92A
EXPORT _Temporal_AvoidFriendlies(REGISTERS *R) {
	// (ASM knowledge) the ESI register contains a pointer to an instance of TemporalClass, let's get it
	TemporalClass *m = (TemporalClass *)R->get_ESI(); 

	// House that owns the unit firing this weapon
	HouseClass *hv = m->get_TargetUnit()->get_Owner();

	// House that owns the unit being erased
	HouseClass *ho = m->get_OwningUnit()->get_Owner();

	// if the two houses aren't allied with each other, there's nothing to fix
	if(!ho->IsAlliedWith(hv)) {
		return 0;
	}

	// (ASM knowledge) force the game to jump over the experience-granting code
	return 0x71A97D;
}
