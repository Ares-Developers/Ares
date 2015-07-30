#include "Body.h"
#include "../House/Body.h"
#include "../HouseType/Body.h"
#include "../SWType/Body.h"

#include "../../Ares.CRT.h"
#include <SuperClass.h>
#include <ProgressScreenClass.h>
#include <VoxClass.h>

//0x4F8C97
DEFINE_HOOK(4F8C97, Sides_BuildConst, 6)
{
	GET(HouseClass *, pThis, ESI);

	enum { NotifyLowPower = 0x4F8D02, Skip = 0x4F8DB1 };

	// disable FSW on low power
	auto pHouseData = HouseExt::ExtMap.Find(pThis);
	pHouseData->SetFirestormState(false);

	// should play low power EVA for more than three BuildConst items
	for(auto pItem : RulesClass::Instance->BuildConst) {
		if(pThis->OwnedBuildingTypes1[pItem->ArrayIndex] > 0) {
			return NotifyLowPower;
		}
	}

	return Skip;
}

//0x4F8F54
DEFINE_HOOK(4F8F54, Sides_SlaveMinerCheck, 6)
{
	GET(HouseClass *, pThis, ESI);
	GET(int, n, EDI);

	for(int i = 0; i < RulesClass::Instance->BuildRefinery.Count; ++i) {
		 //new sane way to find a slave miner
		if(RulesClass::Instance->BuildRefinery.Items[i]->SlavesNumber > 0) {
			n += pThis->OwnedBuildingTypes1.GetItemCount(
				RulesClass::Instance->BuildRefinery.Items[i]->ArrayIndex);
		}
	}

	R->EDI(n);
	return 0x4F8F75;
}

DEFINE_HOOK_AGAIN(507DBA, Sides_BaseDefenses, 6) // HouseClass_PickAntiArmorDefense
DEFINE_HOOK_AGAIN(507FAA, Sides_BaseDefenses, 6) // HouseClass_PickAntiInfantryDefense
DEFINE_HOOK(507BCA, Sides_BaseDefenses, 6) // HouseClass_PickAntiAirDefense
{
	GET(HouseTypeClass *, pCountry, EAX);
	static DynamicVectorClass<BuildingTypeClass*> dummy;

	SideClass* pSide = SideClass::Array->GetItemOrDefault(pCountry->SideIndex);
	if(auto pData = SideExt::ExtMap.Find(pSide)) {
		auto it = pData->GetBaseDefenses();
		dummy.Items = const_cast<BuildingTypeClass**>(it.begin());
		dummy.Count = dummy.Capacity = it.size();

		R->EBX(&dummy);
		return R->Origin() + 0x36;
	} else {
		return 0;
	}
}

DEFINE_HOOK(52267D, InfantryClass_GetDisguise_Disguise, 6)
{
	GET(HouseClass *, pHouse, EAX);

	if(auto pData = HouseExt::ExtMap.Find(pHouse)) {
		R->EAX<InfantryTypeClass*>(pData->GetDisguise());
		return 0x5226B7;
	} else {
		return 0;
	}
}

DEFINE_HOOK_AGAIN(6F422F, Sides_Disguise, 6) // TechnoClass_Init
DEFINE_HOOK(5227A3, Sides_Disguise, 6) // InfantryClass_SetDefaultDisguise
{
	GET(HouseClass *, pHouse, EAX);
	InfantryClass* pThis = nullptr;
	DWORD dwReturnAddress = 0;

	if(R->Origin() == 0x5227A3) {
		pThis = R->ECX<InfantryClass*>();
		dwReturnAddress = 0x5227EC;
	} else {
		pThis = R->ESI<InfantryClass*>();
		dwReturnAddress = 0x6F4277;
	}

	if(auto pData = HouseExt::ExtMap.Find(pHouse)) {
		pThis->Disguise = pData->GetDisguise();
		return dwReturnAddress;
	} else {
		return 0;
	}
}

/*
 * this is as good as it can get without tearing the scenario reader apart
 * - find house early, set color from its data...
 * but finding house needs the house array to be ready
 * instantiating house needs data from rules
 * instantiating rules takes shitloads of time, we can't show a blank screen so long
A_FINE_HOOK(687586, INIClass_ReadScenario, 7)
{
	GET(LoadProgressManager *, Mgr, EAX);
	if(SessionClass::Instance->GameMode == GameMode::Campaign) {
		GET_STACK(CCINIClass *, pINI, STACK_OFFS(0x174, 0x15C));

		HouseClass::LoadFromINIList(pINI); // comment out this line to make it work everywhere except for the very first scenario you try

		pINI->ReadString("Basic", "Player", "Americans", Ares::readBuffer);
		int idxHouse = HouseClass::FindIndexByName(Ares::readBuffer);
		Debug::Log("Side was %d and iH = %d\n", ProgressScreenClass::Instance->GetSide(), idxHouse);
		if(idxHouse > -1 && idxHouse < HouseClass::Array->Count) {
			int idxSide = HouseClass::Array->GetItem(idxHouse)->Type->SideIndex;

			ProgressScreenClass::Instance->SetSide(idxSide);
			Debug::Log("Side is now %d\n", idxSide);
		}
	}

	Mgr->Draw();
	return 0x68758D;
}
*/

// WRONG! Stoopidwood passes CD= instead of Side= into singleplayer campaigns, TODO: fix that shit
DEFINE_HOOK(642B36, Sides_LoadTextColor1, 5)
	{ return SideExt::LoadTextColor(R, 0x68CAA9); }

// WRONG! Stoopidwood passes CD= instead of Side= into singleplayer campaigns, TODO: fix that shit
DEFINE_HOOK(643BB9, Sides_LoadTextColor2, 5)
	{ return SideExt::LoadTextColor(R, 0x643BEF); }

DEFINE_HOOK(642B91, Sides_LoadTextColor3, 5)
	{ return SideExt::LoadTextColor(R, 0x68CAA9); }

DEFINE_HOOK(6847B7, Sides_LoadTextColor_CacheMP, 6) {
	GET(HouseTypeClass*, pType, EAX);

	SideExt::CurrentLoadTextColor = -1;

	if(HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pType)) {
		if(pData->LoadTextColor != -1) {
			SideExt::CurrentLoadTextColor = pData->LoadTextColor;
		}
	}

	return 0;
}

DEFINE_HOOK(686D7F, Sides_LoadTextColor_CacheSP, 6) {
	LEA_STACK(INIClass*, pINI, 0x1C);

	const char* pDefault = "";
	const char* pID =  ScenarioClass::Instance->FileName;

	if(!_strnicmp(pID, "SOV", 3)) {
		pDefault = "SovietLoad";
	} else if(!_strnicmp(pID, "YUR", 3)) {
		pDefault = "YuriLoad";
	} else if(!_strnicmp(pID, "TUT", 3)) {
		pDefault = "LightGrey";
	} else {
		pDefault = "AlliedLoad";
	}

	SideExt::CurrentLoadTextColor = -1;

	if(pINI->ReadString(ScenarioClass::Instance->FileName, "LoadScreenText.Color", pDefault, Ares::readBuffer)) {
		if(ColorScheme* pCS = ColorScheme::Find(Ares::readBuffer)) {
			SideExt::CurrentLoadTextColor = pCS->ArrayIndex; // TODO: check if off by one. see ColorScheme.h
		}
	}

	return 0;
}

// issue 906
// do not draw a box below the label text if there is none.
DEFINE_HOOK(553E54, LoadProgressMgr_Draw_SkipShadowOnNullString, 6) {
	GET(wchar_t*, pBrief, ESI);

	if(!pBrief || !wcslen(pBrief)) {
		return 0x554027;
	}

	return 0;
}

// do not draw a box for the country name.
DEFINE_HOOK(553820, LoadProgressMgr_Draw_SkipShadowOnNullString2, 5) {
	GET(wchar_t*, pCountry, EDI);

	if(!pCountry || !wcslen(pCountry)) {
		return 0x5539E4;
	}

	return 0;
}

// do not draw a box for an empty LoadingEx string
DEFINE_HOOK(55403D, LoadProgressMgr_Draw_SkipShadowOnNullString3, 6) {
	GET(wchar_t*, pLoading, EAX);

	if(!pLoading || !wcslen(pLoading)) {
		return 0x554097;
	}

	return 0;
}

//0x534FB1
DEFINE_HOOK(534FB1, Sides_MixFileIndex, 5)
{
	GET(int, n, ESI);
	SideClass* pSide = SideClass::Array->GetItem(n);
	if(SideExt::ExtData *pData = SideExt::ExtMap.Find(pSide)) {
		// original code is
		// sprintf(mixname, "SIDEC%02dMD.MIX", ESI + 1);
		// it's easier to sub 1 here than to fix the calculation in the orig code
		R->ESI(pData->SidebarMixFileIndex - 1);
	} else if(n == 2) {
		R->ESI(1);
	}

	return 0x534FBB;
}

DEFINE_HOOK(72FA1A, Sides_MixFileYuriFiles1, 7)
	{ return SideExt::MixFileYuriFiles(R, 0x72FA23, 0x72FA6A); }

DEFINE_HOOK(72F370, Sides_MixFileYuriFiles2, 7)
	{ return SideExt::MixFileYuriFiles(R, 0x72F379, 0x72F3A0); }

DEFINE_HOOK(72FBC0, Sides_MixFileYuriFiles3, 5)
	{ return SideExt::MixFileYuriFiles(R, 0x72FBCE, 0x72FBF5); }

/* fixes to reorder the savegame */
DEFINE_HOOK(67D315, SaveGame_EarlySaveSides, 5)
{
	GET(LPSTREAM, pStm, ESI);
	return (Game::Save_Sides(pStm, SideClass::Array) >= 0)
		? 0
		: 0x67E0B8
	;
}

DEFINE_HOOK(67E09A, SaveGame_LateSkipSides, 5)
{
	GET(int, success, EAX);
	return success >= 0
		? 0x67E0C2
		: 0x67E0B8
	;
}


DEFINE_HOOK(67E74A, LoadGame_EarlyLoadSides, 5)
{
	GET(LPSTREAM, pStm, ESI);

	int length = 0;
	LPVOID out;
	if(pStm->Read(&length, 4, 0) < 0) {
		return 0x67F7A3;
	}
	for(int i = 0; i < length; ++i) {
		if((*Imports::OleLoadFromStream)(pStm, &IIDs::IUnknown, &out) < 0) {
			return 0x67F7A3;
		}
	}

	return 0;
}

DEFINE_HOOK(67F281, LoadGame_LateSkipSides, 7)
{
	return 0x67F2BF;
}

DEFINE_HOOK(41E893, AITriggerTypeClass_ConditionMet_SideIndex, 0)
{
	GET(HouseClass *, House, EDI);
	GET(int, triggerSide, EAX);

	enum Eligible { Yes = 0x41E8D7, No = 0x41E8A1 };
	if(!triggerSide) {
		return Yes;
	}

	--triggerSide;
	return(triggerSide == House->SideIndex)
		? Yes
		: No
	;
}

DEFINE_HOOK(7534E0, VoxClass_SetEVAIndex, 5)
{
	GET(int, side, ECX);

	if(side < 0) {
		VoxClass::EVAIndex = -1;
	} else {
		SideClass* pSide = SideClass::Array->GetItem(side);
		if(SideExt::ExtData *pData = SideExt::ExtMap.Find(pSide)) {
			VoxClass::EVAIndex = pData->EVAIndex;
		}
	}

	return 0x7534F3;
}

DEFINE_HOOK(6DE0D3, TActionClass_Execute_MessageColor, 6)
{
	int idxSide = ScenarioClass::Instance->PlayerSideIndex;
	int idxColor = 0;

	if(SideClass* pSide = SideClass::Array->GetItemOrDefault(idxSide)) {
		if(SideExt::ExtData* pExt = SideExt::ExtMap.Find(pSide)) {
			idxColor = pExt->MessageTextColorIndex;
		}
	}

	R->EAX(idxColor);
	return 0x6DE0DE;
}

DEFINE_HOOK(72F440, Game_InitializeToolTipColor, A)
{
	GET(int, idxSide, ECX);

	if(SideClass* pSide = SideClass::Array->GetItemOrDefault(idxSide)) {
		if(SideExt::ExtData* pExt = SideExt::ExtMap.Find(pSide)) {
			ColorStruct &clrToolTip = *reinterpret_cast<ColorStruct*>(0x0B0FA1C);
			clrToolTip = pExt->ToolTipTextColor;
			return 0x72F495;
		}
	}

	return 0;
}

// score screens

// campaign
DEFINE_HOOK(72D300, Game_LoadCampaignScoreAssets, 5)
{
	GET(const int, idxSide, ECX);
	auto pSide = SideClass::Array->GetItemOrDefault(idxSide);
	auto pExt = SideExt::ExtMap.Find(pSide);

	auto& AlreadyLoaded = *reinterpret_cast<bool*>(0xB0FBAC);

	if(!AlreadyLoaded) {

		// load the images
		auto& SxCRBKyy_SHP = *reinterpret_cast<SHPStruct**>(0xB0FB34);
		auto& SxCRTyy_SHP = *reinterpret_cast<SHPStruct**>(0xB0FB00);
		auto& SxCRAyy_SHP = *reinterpret_cast<SHPStruct**>(0xB0FB30);

		auto& SxCRBKyy_Loaded = *reinterpret_cast<bool*>(0xB0FC70);
		auto& SxCRTyy_Loaded = *reinterpret_cast<bool*>(0xB0FC71);
		auto& SxCRAyy_Loaded = *reinterpret_cast<bool*>(0xB0FC72);

		SxCRBKyy_SHP = FileSystem::LoadWholeFileEx<SHPStruct>(pExt->ScoreCampaignBackground, SxCRBKyy_Loaded);
		SxCRTyy_SHP = FileSystem::LoadWholeFileEx<SHPStruct>(pExt->ScoreCampaignTransition, SxCRTyy_Loaded);
		SxCRAyy_SHP = FileSystem::LoadWholeFileEx<SHPStruct>(pExt->ScoreCampaignAnimation, SxCRAyy_Loaded);

		// load the palette
		auto& xSCORE_Palette = *reinterpret_cast<BytePalette**>(0xB0FBA4);
		auto& xSCORE_Convert = *reinterpret_cast<ConvertClass**>(0xB0FBA8);

		ConvertClass::CreateFromFile(pExt->ScoreCampaignPalette, xSCORE_Palette, xSCORE_Convert);

		AlreadyLoaded = true;
	}

	return 0x72D345;
}

// multiplayer
DEFINE_HOOK(72D730, Game_LoadMultiplayerScoreAssets, 5)
{
	GET(const int, idxSide, ECX);
	auto pSide = SideClass::Array->GetItemOrDefault(idxSide);
	auto pExt = SideExt::ExtMap.Find(pSide);

	auto& AlreadyLoaded = *reinterpret_cast<bool*>(0xB0FBB8);

	if(!AlreadyLoaded) {

		// load the images
		auto& MPxSCRNy_SHP = *reinterpret_cast<SHPStruct**>(0xB0FB1C);
		auto& MPxSCRNy_Loaded = *reinterpret_cast<bool*>(0xB0FC7D);

		MPxSCRNy_SHP = FileSystem::LoadWholeFileEx<SHPStruct>(pExt->ScoreMultiplayBackground, MPxSCRNy_Loaded);

		// load the palette
		auto& MPxSCRN_Palette = *reinterpret_cast<BytePalette**>(0xB0FBB0);
		auto& MPxSCRN_Convert = *reinterpret_cast<ConvertClass**>(0xB0FBB4);

		ConvertClass::CreateFromFile(pExt->ScoreMultiplayPalette, MPxSCRN_Palette, MPxSCRN_Convert);

		AlreadyLoaded = true;
	}

	return 0x72D775;
}

DEFINE_HOOK(5CA110, Game_GetMultiplayerScoreScreenBar, 5)
{
	GET(unsigned int, idxBar, ECX);

	int idxSide = ScenarioClass::Instance->PlayerSideIndex;
	auto pSide = SideClass::Array->GetItemOrDefault(idxSide);
	auto pExt = SideExt::ExtMap.Find(pSide);

	auto pFilename = pExt->GetMultiplayerScoreBarFilename(idxBar);
	auto ret = PCX::Instance->GetSurface(pFilename);

	R->EAX(ret);
	return 0x5CA41D;
}

// customizable global graphics

DEFINE_HOOK(53534C, Game_LoadUI_LoadSideData, 7)
{
	SideExt::UpdateGlobalFiles();
	return 0;
}

// graphical text banner
DEFINE_HOOK(6D4E79, TacticalClass_DrawOverlay_GraphicalText, 6)
{
	auto pConvert = SideExt::GetGraphicalTextConvert();
	auto pShp = SideExt::GetGraphicalTextImage();

	R->EBX(pConvert);
	R->ESI(pShp);

	return (pConvert && pShp) ? 0x6D4E8D : 0x6D4EF4;
}

// dialog background
DEFINE_HOOK(622223, sub_621E90_DialogBackground, 6)
{
	auto pShp = SideExt::DialogBackgroundImage.get();
	auto pConvert = SideExt::DialogBackgroundConvert.get();

	R->EDI(pShp);
	R->EAX(pConvert);

	return (pConvert && pShp) ? 0x62226A : 0;
}

// score options

// multiplayer score music depending on win or lose
DEFINE_HOOK(5C9B75, Global_DrawScoreScreen_ScoreTheme, 5)
{
	REF_STACK(const char*, pTheme, 0x0);

	if(!HouseClass::IsPlayerObserver()) {
		int idxSide = ScenarioClass::Instance->PlayerSideIndex;
		auto pSide = SideClass::Array->GetItemOrDefault(idxSide);
		auto pExt = SideExt::ExtMap.Find(pSide);

		pTheme = HouseClass::Player->Defeated
			? pExt->ScoreMultiplayThemeLose
			: pExt->ScoreMultiplayThemeWin;
	}

	return 0;
}

// score music for single player missions
static const char* pSinglePlayerScoreTheme = nullptr;

DEFINE_HOOK(6C922C, ScoreDialog_Handle_ScoreThemeA, 5)
{
	GET(int, elapsed, EDI);
	GET(int, par, ESI);

	auto pScen = ScenarioClass::Instance;

	int idxSide = pScen->PlayerSideIndex;
	auto pSide = SideClass::Array->GetItemOrDefault(idxSide);
	auto pExt = SideExt::ExtMap.Find(pSide);

	// replicate skipped instructions, and also update the score id
	const char* pTitle = nullptr;
	const char* pMessage = nullptr;

	if(elapsed > par) {
		pTitle = pScen->OverParTitle;
		pMessage = pScen->OverParMessage;
		pSinglePlayerScoreTheme = pExt->ScoreCampaignThemeOverPar;
	} else {
		pTitle = pScen->UnderParTitle;
		pMessage = pScen->UnderParMessage;
		pSinglePlayerScoreTheme = pExt->ScoreCampaignThemeUnderPar;
	}

	R->ECX(pTitle);
	R->ESI(pMessage);
	return 0x6C924F;
}

DEFINE_HOOK(6C935C, ScoreDialog_Handle_ScoreThemeB, 5)
{
	REF_STACK(const char*, pTheme, 0x0);

	if(pSinglePlayerScoreTheme) {
		pTheme = pSinglePlayerScoreTheme;
	}

	return 0;
}

// music piece when loading a match or mission
int idxLoadingTheme = -2;

DEFINE_HOOK(683C70, sub_683AB0_LoadingScoreA, 7)
{
	LEA_STACK(CCINIClass*, pINI, STACK_OFFS(0xFC, 0xE0));

	// magic value for the default loading theme
	idxLoadingTheme = -2;

	if(SessionClass::Instance->GameMode == GameMode::Campaign) {
		// single player missions read from the scenario
		idxLoadingTheme = pINI->ReadTheme("Basic", "LoadingTheme", -2);

	} else {
		// override the default for multiplayer matches
		if(auto pSpot = SessionClass::Instance->StartSpots.GetItemOrDefault(0)) {
			if(auto pType = HouseTypeClass::Array->GetItemOrDefault(pSpot->Country)) {

				// get theme from the side
				auto pSide = SideClass::Array->GetItemOrDefault(pType->SideIndex);
				auto pRulesINI = CCINIClass::INI_Rules;
				idxLoadingTheme = pRulesINI->ReadTheme(pSide->ID, "LoadingTheme", -2);

				// ...then from the house
				idxLoadingTheme = pRulesINI->ReadTheme(pType->ID, "LoadingTheme", idxLoadingTheme);
			}
		}
	}

	return 0;
}

DEFINE_HOOK(683D05, sub_683AB0_LoadingScoreB, 5)
{
	R->EAX(idxLoadingTheme);
	return (idxLoadingTheme == -2) ? 0 : 0x683D14;
}
