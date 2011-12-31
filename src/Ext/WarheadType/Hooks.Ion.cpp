#include "Body.h"

DEFINE_HOOK(0x53CC6E, IonBlastClass_Update, 0x6)
{
	GET(IonBlastClass *, IB, EBX);
	return (WarheadTypeExt::IonExt.find(IB) == WarheadTypeExt::IonExt.end())
		? 0
		: 0x53CE0A
	;
}

DEFINE_HOOK(0x53CC0D, IonBlastClass_Update_DTOR, 0x5)
{
	GET(IonBlastClass *, IB, EBX);
	hash_ionExt::iterator i = WarheadTypeExt::IonExt.find(IB);
	if(i != WarheadTypeExt::IonExt.end()) {
		WarheadTypeExt::IonExt.erase(i);
	}
	return 0;
}

DEFINE_HOOK(0x53CBF5, IonBlastClass_Update_Duration, 0x5)
{
	GET(IonBlastClass *, IB, EBX);

	int Ripple_Radius;
	if(WarheadTypeExt::IonExt.find(IB) == WarheadTypeExt::IonExt.end()) {
		Ripple_Radius = 79;
	} else {
		WarheadTypeExt::ExtData *pData = WarheadTypeExt::IonExt[IB];
		Ripple_Radius = std::min(79, pData->Ripple_Radius + 1);
	}

	if(IB->Lifetime < Ripple_Radius) {
//		++IB->Lifetime;
		return 0x53CC3A;
	} else {
		return 0x53CBFA;
	}
}
