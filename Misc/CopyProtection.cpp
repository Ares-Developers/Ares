//A whole source file simply for the purpose to dissolve WW's copy protections!

#include "..\Ares.h"

//TODO: Add a function that determines versions of the exes (detects a cracked ra2md.exe etc)?

DEFINE_HOOK(4A80D0, CD_AlwaysFindYR, 6)
{
	if(Ares::bNoCD)
	{
		R->set_EAX(2);
		return 0x4A8265;
	}
	return 0;
}

DEFINE_HOOK(4790E0, CD_AlwaysAvailable, 7)
{
	if(Ares::bNoCD)
	{
		R->set_AL(1);
		return 0x479109;
	}
	return 0;
}

DEFINE_HOOK(479110, CD_NeverAsk, 5)
{
	if(Ares::bNoCD)
	{
		R->set_AL(1);
		return 0x4791EA;
	}
	return 0;
}

DEFINE_HOOK(49F5C0, CopyProtection_IsLauncherRunning, 8)
{
	R->set_AL(1);
	return 0x49F61A;
}

DEFINE_HOOK(49F620, CopyProtection_NotifyLauncher, 5)
{
	R->set_AL(1);
	return 0x49F733;
}

DEFINE_HOOK(49F7A0, CopyProtection_CheckProtectedData, 8)
{
	R->set_AL(1);
	return 0x49F8A7;
}
