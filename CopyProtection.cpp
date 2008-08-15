//A whole source file simply for the purpose to dissolve WW's copy protections!

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif
#pragma warning(disable: 4035)	//"no return value" - there is one, just not in our code ;)

#include "Ares.h"

//TODO: Add a function that determines versions of the exes (detects a cracked ra2md.exe etc)?

//Hook at 0x4A80D0
EXPORT CD_AlwaysFindYR(REGISTERS* R)
{
	if(Ares::bNoCD)
	{
		R->set_EAX(2);
		return 0x4A8265;
	}
	return 0;
}

//Hook at 0x4790E0
EXPORT CD_AlwaysAvailable(REGISTERS* R)
{
	if(Ares::bNoCD)
	{
		R->set_AL(1);
		return 0x479109;
	}
	return 0;
}

//Hook at 0x479110
EXPORT CD_NeverAsk(REGISTERS* R)
{
	if(Ares::bNoCD)
	{
		R->set_AL(1);
		return 0x4791EA;
	}
	return 0;
}

//Hook at 0x49F5C0
EXPORT CopyProtection_IsLauncherRunning(REGISTERS* R)
{
	R->set_AL(1);
	return 0x49F61A;
}

//Hook at 0x49F620
EXPORT CopyProtection_NotifyLauncher(REGISTERS* R)
{
	R->set_AL(1);
	return 0x49F733;
}

//Hook at 0x49F7A0
EXPORT CopyProtection_CheckProtectedData(REGISTERS* R)
{
	R->set_AL(1);
	return 0x49F8A7;
}
