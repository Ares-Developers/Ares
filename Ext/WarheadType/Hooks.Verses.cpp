#include "Body.h"

#define GET_VERSES(reg_wh, reg_armor) \
		GET(WarheadTypeClass *, WH, reg_wh); \
	GET(int, Armor, reg_armor); \
	WarheadTypeExt::ExtData *pData = WarheadTypeExt::ExtMap.Find(WH); \
	double x = pData->Verses[Armor].Verses;

#define FLD_VERSES(reg_wh, reg_armor) \
	GET_VERSES(reg_wh, reg_armor) \
	__asm{ fld x }; \
	return R->get_Origin() + 7;

#define FMUL_VERSES(reg_wh, reg_armor) \
	GET_VERSES(reg_wh, reg_armor) \
	__asm{ fmul x }; \
	return R->get_Origin() + 7;

// temp, will be taken out when SelectWeapon is remade
DEFINE_HOOK(6F36FE, Verses_fld_0, 7)
{
	FLD_VERSES(EAX, ECX);
}

DEFINE_HOOK(6F7D3D, Verses_fld_1, 7)
{
	FLD_VERSES(ECX, EAX);
}

DEFINE_HOOK(708AF7, Verses_fld_2, 7)
{
	FLD_VERSES(ECX, EAX);
}

DEFINE_HOOK(6FCB6A, Verses_fld_3, 7)
{
	FLD_VERSES(EDI, EAX);
}

// temp, will be taken out when SelectWeapon is remade
DEFINE_HOOK(6F3731, Verses_fld_4, 7)
{
	FLD_VERSES(EDX, EAX);
}

DEFINE_HOOK(70CEB2, Verses_fmul_0, 7)
{
	FMUL_VERSES(EAX, ECX);
}

DEFINE_HOOK(70CEC7, Verses_fmul_1, 7)
{
	FMUL_VERSES(EAX, EDX);
}

DEFINE_HOOK(70CF49, Verses_fmul_2, 7)
{
	FMUL_VERSES(ECX, EAX);
}

DEFINE_HOOK(48923D, Verses_fmul_3, 7)
{
	FMUL_VERSES(EDI, EDX);
}

DEFINE_HOOK(75DDCC, Verses_OrigParser, 7)
{
	// should really be doing something smarter due to westwood's weirdass code, but cannot be bothered atm
	// will fix if it is reported to actually break things
	// this breaks 51E33D which stops infantry with verses (heavy=0 & steel=0) from targeting non-infantry at all
	// (whoever wrote that code must have quite a few gears missing in his head)
	return 0x75DE98;
}
