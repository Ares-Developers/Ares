#include "Body.h"
 
DEFINE_HOOK(53CC6E, IonBlastClass_Update, 6)
{
	GET(IonBlastClass *, IB, EBX);
	return (WarheadTypeExt::IonExt.find(IB) == WarheadTypeExt::IonExt.end())
	  ? 0
	  : 0x53CE0A;
}

DEFINE_HOOK(53CC0D, IonBlastClass_Update_DTOR, 5)
{
	GET(IonBlastClass *, IB, EBX);
	WarheadTypeExt::IonExt.erase(WarheadTypeExt::IonExt.find(IB));
	return 0;
}

DEFINE_HOOK(53CBF5, IonBlastClass_Update_Duration, 5)
{
	GET(int, Idx, EAX);
	GET(IonBlastClass *, IB, EBX);
	if(WarheadTypeExt::IonExt.find(IB) == WarheadTypeExt::IonExt.end()) { return 0; };
	WarheadTypeExt::ExtData *pData = WarheadTypeExt::IonExt[IB];
	return (Idx < pData->Ripple_Radius)
	  ? 0x53CC3A
	  : 0x53CBFA;
}
