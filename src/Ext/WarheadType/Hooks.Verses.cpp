#include "Body.h"

#define GET_VERSES(reg_wh, reg_armor) \
	GET(WarheadTypeClass *, WH, reg_wh); \
	GET(int, Armor, reg_armor); \
	WarheadTypeExt::ExtData *pData = WarheadTypeExt::ExtMap.Find(WH); \
	WarheadTypeExt::VersesData *vsData = &pData->Verses[Armor];

#define FLD_VERSES(reg_wh, reg_armor) \
	GET_VERSES(reg_wh, reg_armor) \
	double VS = vsData->Verses; \
	__asm{ fld VS }; \
	return R->Origin() + 7;

#define FMUL_VERSES(reg_wh, reg_armor) \
	GET_VERSES(reg_wh, reg_armor) \
	double VS = vsData->Verses; \
	__asm{ fmul VS }; \
	return R->Origin() + 7;

#ifdef _MSC_VER
DEFINE_HOOK(6F36FE, Verses_fld_0, 0)
{
	GET_VERSES(EAX, ECX);
	return vsData->Verses == 0.0 // vsData->ForceFire - taking this out because it has nothing to do with _forcing_ fire
		? 0x6F37AD
		: 0x6F3716
	;
}

DEFINE_HOOK(6F3731, Verses_fld_1, 0)
{
	GET_VERSES(EDX, EAX);
	return vsData->Verses == 0.0
		? 0x6F3745
		: 0x6F3754
	;
}

DEFINE_HOOK(708AF7, Verses_fld_2, 0)
{
	GET_VERSES(ECX, EAX);
	return vsData->Retaliate
		? 0x708B0B
		: 0x708B17
	;
}

DEFINE_HOOK(6FCB6A, Verses_fld_3, 0)
{
	GET_VERSES(EDI, EAX);
	return vsData->ForceFire
		? 0x6FCB8D
		: 0x6FCB7E
	;
}

DEFINE_HOOK(6F7D3D, Verses_fld_4, 0)
{
	GET_VERSES(ECX, EAX);
	return vsData->PassiveAcquire
		? 0x6F7D55
		: 0x6F894F
	;
}

//
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

#endif

DEFINE_HOOK(75DDCC, Verses_OrigParser, 7)
{
	// should really be doing something smarter due to westwood's weirdass code, but cannot be bothered atm
	// will fix if it is reported to actually break things
	// this breaks 51E33D which stops infantry with verses (heavy=0 & steel=0) from targeting non-infantry at all
	// (whoever wrote that code must have quite a few gears missing in his head)
	return 0x75DE98;
}
