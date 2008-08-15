//TEST DLL
//=============
//

#define UNICODE

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif
#pragma warning(disable: 4035)	//"no return value" - there is one, just not in our code ;)

#include <stdio.h>
#include "../ASMMacros.h"
#include "../YRPP.h"
#include "../ArrayClasses.h"
//include "../Drawing.cpp"
//include "../MessageBox.cpp"
//include "../CommandClass.h"
//include "../StringTable.cpp"
#include "../Debugger.cpp"

// 0x71A92A - Temporal veterancy
EXPORT _TEST_AvoidFriendlies() {
	TemporalClass *m = (TemporalClass *)Debugger::GetReg32(rESI);
	HouseClass *hv = m->get_TargetUnit()->get_Owner();
	HouseClass *ho = m->get_OwningUnit()->get_Owner();

	if(!ho->IsAlliedWith(hv)) {
		return;
	}
//	Mouse gets frozen on last cursor, needs fixing
//	MessageBox::Show(L"No cheating!", L"OK", L"OK");
	Debugger::SetReturnEIP(0x71A97D);
}
