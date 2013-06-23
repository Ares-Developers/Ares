#include <ScenarioClass.h>

#include "Body.h"
#include "../Techno/Body.h"
/*
A_FINE_HOOK(4691D5, RadSite_Create_1, 5)
{
	GET(RadSiteClass *, Rad, EDI);
	GET(BulletClass *, B, ESI);

	WeaponTypeExt::ExtData *pData = WeaponTypeExt::ExtMap.Find(B->get_WeaponType());
	WeaponTypeExt::RadSiteExt[Rad] = pData;

	// replacing code we abused
	GET(DWORD, XY, EAX);
	Rad->set_BaseCell((CellStruct*)&XY);
	return 0x4691DA;
}

A_FINE_HOOK(65B593, RadSiteClass_Radiate_0, 6)
{
	GET(RadSiteClass *, Rad, ECX);
	WeaponTypeExt::ExtData *pData = WeaponTypeExt::RadSiteExt[Rad];

	R->set_ECX(pData->Rad_Type->Level_Delay);
	R->set_EAX(pData->Rad_Type->Light_Delay);

	return 0x65B59F;
}


A_FINE_HOOK(65B5CE, RadSiteClass_Radiate_1, 6)
{
	GET(RadSiteClass *, Rad, ESI);
	WeaponTypeExt::ExtData *pData = WeaponTypeExt::RadSiteExt[Rad];

	R->set_EAX(0);
	R->set_EBX(0);
	R->set_EDX(0);

	ColorStruct clr = pData->Rad_Type->Color;

	// NOT MY HACK, HELLO WESTWEIRD
	if(ScenarioClass::Global()->get_Theater() == th_Snow) {
		clr.R = clr.B = 0x80;
	}

	R->set_DL(clr.G);
	R->set_EBP(R->get_EDX());

	R->set_BL(clr.B);

	R->set_AL(clr.R);

	return 0x65B604;
}

A_FINE_HOOK(65B63E, RadSiteClass_Radiate_2, 6)
{
	GET(RadSiteClass *, Rad, EDI);
	WeaponTypeExt::ExtData *pData = WeaponTypeExt::RadSiteExt[Rad];

	double factor = pData->Rad_Type->Light_Factor;
	__asm { fmul factor };

	return 0x65B644;
}

A_FINE_HOOK(65B6A0, RadSiteClass_Radiate_3, 6)
{
	GET(RadSiteClass *, Rad, EDI);
	WeaponTypeExt::ExtData *pData = WeaponTypeExt::RadSiteExt[Rad];

	double factor = pData->Rad_Type->Tint_Factor;
	__asm { fmul factor };
	return 0x65B6A6;
}

A_FINE_HOOK(65B6CA, RadSiteClass_Radiate_4, 6)
{
	GET(RadSiteClass *, Rad, EDI);
	WeaponTypeExt::ExtData *pData = WeaponTypeExt::RadSiteExt[Rad];

	double factor = pData->Rad_Type->Tint_Factor;
	__asm { fmul factor };
	return 0x65B6D0;
}


A_FINE_HOOK(65B6F2, RadSiteClass_Radiate_5, 6)
{
	GET(RadSiteClass *, Rad, EDI);
	WeaponTypeExt::ExtData *pData = WeaponTypeExt::RadSiteExt[Rad];

	double factor = pData->Rad_Type->Tint_Factor;
	__asm { fmul factor };
	return 0x65B6F8;
}


A_FINE_HOOK(65B73A, RadSiteClass_Radiate_6, 5)
{
	if(!R->get_EAX()) {
		R->set_EAX(1);
	}
	return 0;
}


A_FINE_HOOK(65B843, RadSiteClass_Update_1, 6)
{
	GET(RadSiteClass *, Rad, ESI);
	WeaponTypeExt::ExtData *pData = WeaponTypeExt::RadSiteExt[Rad];

	R->set_ECX(pData->Rad_Type->Level_Delay);
	return 0x65B849;
}

A_FINE_HOOK(65B8B9, RadSiteClass_Update_2, 6)
{
	GET(RadSiteClass *, Rad, ESI);
	WeaponTypeExt::ExtData *pData = WeaponTypeExt::RadSiteExt[Rad];

	R->set_ECX(pData->Rad_Type->Light_Delay);
	return 0x65B8BF;
}

// -- doesn't work logically
FINE_HOOK(487CB0, CellClass_GetRadLevel, 5)
{
	GET(CellClass *, Cell, ECX);
	RadSiteClass * Rad = Cell->get_RadSite();
	if(!Rad) {
		R->set_EAX(0);
	} else {
		WeaponTypeClassExt::WeaponTypeClassData *pData = WeaponTypeClassExt::RadSiteExt[Rad];
		double RadLevel = Cell->get_RadLevel();
		if(pData->Rad_Level_Max < RadLevel) {
			RadLevel = pData->Rad_Level_Max;
		}
		R->set_EAX(FloatToInt(RadLevel));
	}
	return 0x487E39;
}
*/

DEFINE_HOOK(65B5FB, RadSiteClass_Radiate_UnhardcodeSnow, 0)
{
	return 0x65B604;
}
