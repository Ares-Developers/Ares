#include "../Ext/TechnoType/Body.h"
#include "../Ext/SWType/Body.h"

#include <PCX.h>

// bugfix #277 revisited: VeteranInfantry and friends don't show promoted cameos
DEFINE_HOOK(0x712045, TechnoTypeClass_GetCameo, 0x5)
{
	// egads and gadzooks
	retfunc<SHPStruct *> ret(R, 0x7120C6);

	GET(TechnoTypeClass *, T, ECX);

	SHPStruct *Cameo = T->Cameo;
	SHPStruct *Alt = T->AltCameo;

	auto pData = TechnoTypeExt::ExtMap.Find(T);

	return ret(
		(pData->CameoIsElite())
			? Alt
			: Cameo
	);
}

// a global var ewww
ConvertClass * CurrentDrawnConvert = NULL;
BSurface * CameoPCX = NULL;

DEFINE_HOOK(0x6A9948, TabCameoListClass_Draw_SW, 0x6)
{
	GET(SuperWeaponTypeClass *, pSW, EAX);
	if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW)) {
		CurrentDrawnConvert = pData->CameoPal.Convert;
	}
	return 0;
}

DEFINE_HOOK(0x6A9A2A, TabCameoListClass_Draw_Main, 0x6)
{
	GET_STACK(TechnoTypeClass *, pTech, STACK_OFFS(0x4C4, 0x458));

	ConvertClass *pPalette = NULL;
	if(pTech) {
		if(TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(pTech)) {
			pPalette = pData->CameoPal.Convert;
		}
	} else if(CurrentDrawnConvert) {
		pPalette = CurrentDrawnConvert;
		CurrentDrawnConvert = NULL;
	}

	if(!pPalette) {
		pPalette = FileSystem::CAMEO_PAL;
	}
	R->EDX<ConvertClass *>(pPalette);
	return 0x6A9A30;
}


DEFINE_HOOK(0x6A9952, TabCameoListClass_Draw_GetSWPCX, 0x6)
{
	GET(SuperWeaponTypeClass *, pSW, EAX);
	auto pData = SWTypeExt::ExtMap.Find(pSW);
	CameoPCX = (*pData->SidebarPCX)
		? PCX::Instance->GetSurface(pData->SidebarPCX)
		: NULL
	;

	return 0;
}

DEFINE_HOOK(0x6A980A, TabCameoListClass_Draw_GetTechnoPCX, 0x8)
{
	GET(TechnoTypeClass *, pType, EBX);

	auto pData = TechnoTypeExt::ExtMap.Find(pType);

	const char * pcxFilename = (pData->CameoIsElite() && *pData->AltCameoPCX)
		? pData->AltCameoPCX
		: pData->CameoPCX
	;

	CameoPCX = (*pcxFilename)
		? PCX::Instance->GetSurface(pcxFilename)
		: NULL
	;

	return 0;
}

DEFINE_HOOK(0x6A99F3, TabCameoListClass_Draw_SkipSHPForPCX, 0x6)
{
	return (CameoPCX)
		? 0x6A9A43
		: 0;
}

DEFINE_HOOK(0x6A9A43, TabCameoListClass_Draw_DrawPCX, 0x6)
{
	if(CameoPCX) {
		GET(int, TLX, ESI);
		GET(int, TLY, EBP);
		RectangleStruct bounds = { TLX, TLY, 60, 48 };
		PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, CameoPCX);
		CameoPCX = NULL;
	}
	return 0;
}
