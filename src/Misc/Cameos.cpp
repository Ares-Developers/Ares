#include "../Ext/TechnoType/Body.h"
#include "../Ext/SWType/Body.h"

#include <PCX.h>

// bugfix #277 revisited: VeteranInfantry and friends don't show promoted cameos
DEFINE_HOOK(712045, TechnoTypeClass_GetCameo, 5)
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
ConvertClass * CurrentDrawnConvert = nullptr;
BSurface * CameoPCX = nullptr;

DEFINE_HOOK(6A9948, TabCameoListClass_Draw_SW, 6)
{
	GET(SuperWeaponTypeClass *, pSW, EAX);
	if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW)) {
		CurrentDrawnConvert = pData->CameoPal.GetConvert();
	}
	return 0;
}

DEFINE_HOOK(6A9A2A, TabCameoListClass_Draw_Main, 6)
{
	GET_STACK(TechnoTypeClass *, pTech, STACK_OFFS(0x4C4, 0x458));

	ConvertClass *pPalette = nullptr;
	if(pTech) {
		if(TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(pTech)) {
			pPalette = pData->CameoPal.GetConvert();
		}
	} else if(CurrentDrawnConvert) {
		pPalette = CurrentDrawnConvert;
		CurrentDrawnConvert = nullptr;
	}

	if(!pPalette) {
		pPalette = FileSystem::CAMEO_PAL;
	}
	R->EDX<ConvertClass *>(pPalette);
	return 0x6A9A30;
}


DEFINE_HOOK(6A9952, TabCameoListClass_Draw_GetSWPCX, 6)
{
	GET(SuperWeaponTypeClass *, pSW, EAX);
	auto pData = SWTypeExt::ExtMap.Find(pSW);
	CameoPCX = (*pData->SidebarPCX)
		? PCX::Instance->GetSurface(pData->SidebarPCX)
		: nullptr
	;

	return 0;
}

DEFINE_HOOK(6A980A, TabCameoListClass_Draw_GetTechnoPCX, 8)
{
	GET(TechnoTypeClass *, pType, EBX);

	auto pData = TechnoTypeExt::ExtMap.Find(pType);

	const char * pcxFilename = (pData->CameoIsElite() && *pData->AltCameoPCX)
		? pData->AltCameoPCX
		: pData->CameoPCX
	;

	CameoPCX = (*pcxFilename)
		? PCX::Instance->GetSurface(pcxFilename)
		: nullptr
	;

	return 0;
}

DEFINE_HOOK(6A99F3, TabCameoListClass_Draw_SkipSHPForPCX, 6)
{
	return (CameoPCX)
		? 0x6A9A43
		: 0;
}

DEFINE_HOOK(6A9A43, TabCameoListClass_Draw_DrawPCX, 6)
{
	if(CameoPCX) {
		GET(int, TLX, ESI);
		GET(int, TLY, EBP);
		RectangleStruct bounds = { TLX, TLY, 60, 48 };
		PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, CameoPCX);
		CameoPCX = nullptr;
	}
	return 0;
}
