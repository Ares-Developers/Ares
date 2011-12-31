//A whole source file simply for the purpose to dissolve WW's copy protections!

#include "../Ares.h"

DEFINE_HOOK(0x4A80D0, CD_AlwaysFindYR, 0x6)
{
	if(Ares::bNoCD) {
		R->EAX(2);
		return 0x4A8265;
	}
	return 0;
}

DEFINE_HOOK(0x4790E0, CD_AlwaysAvailable, 0x7)
{
	if(Ares::bNoCD) {
		R->AL(1);
		return 0x479109;
	}
	return 0;
}

DEFINE_HOOK(0x479110, CD_NeverAsk, 0x5)
{
	if(Ares::bNoCD) {
		R->AL(1);
		return 0x4791EA;
	}
	return 0;
}

DEFINE_HOOK(0x49F5C0, CopyProtection_IsLauncherRunning, 0x8)
{
	R->AL(1);
	return 0x49F61A;
}

DEFINE_HOOK(0x49F620, CopyProtection_NotifyLauncher, 0x5)
{
	R->AL(1);
	return 0x49F733;
}

DEFINE_HOOK(0x49F7A0, CopyProtection_CheckProtectedData, 0x8)
{
	R->AL(1);
	return 0x49F8A7;
}

// this douchebag blows your base up when it thinks you're cheating
DEFINE_HOOK(0x55CFDF, BlowMeUp, 0x0)
{
	return 0x55D059;
}

