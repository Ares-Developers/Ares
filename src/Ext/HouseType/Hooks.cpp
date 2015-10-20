#include "Body.h"
#include "../House/Body.h"
#include "../../Ares.h"
#include <Audio.h>
#include <PCX.h>
#include <ScenarioClass.h>
#include <StringTable.h>

DEFINE_HOOK(553412, LoadProgressMgr_Draw_LSFile, 0)
{
	GET(int, n, EBX);

	HouseTypeExt::ExtData* pData = nullptr;
	if(auto pThis = HouseTypeClass::Array->GetItemOrDefault(n)) {
		pData = HouseTypeExt::ExtMap.Find(pThis);
	}

	const char* pLSFile = nullptr;

	if(pData) {
		pLSFile = pData->LoadScreenBackground;
	} else if(n == 0) {
		pLSFile = "ls%sustates.shp";
	} else {
		return 0x553421;
	}

	R->EDX(pLSFile);
	return 0x55342C;
}

DEFINE_HOOK(5536DA, LoadProgressMgr_Draw_LSName, 0)
{
	GET(int, n, EBX);

	HouseTypeExt::ExtData* pData = nullptr;
	if(auto pThis = HouseTypeClass::Array->GetItemOrDefault(n)) {
		pData = HouseTypeExt::ExtMap.Find(pThis);
	}

	const wchar_t* pLSName = nullptr;

	if(pData) {
		pLSName = pData->LoadScreenName.Get();
	} else if(n == 0) {
		pLSName = StringTable::LoadString("Name:Americans");
	} else {
		return 0x5536FB;
	}

	R->EDI(pLSName);
	return 0x553820;
}

DEFINE_HOOK(553A05, LoadProgressMgr_Draw_LSSpecialName, 6)
{
	GET_STACK(int, n, 0x38);

	HouseTypeExt::ExtData* pData = nullptr;
	if(auto pThis = HouseTypeClass::Array->GetItemOrDefault(n)) {
		pData = HouseTypeExt::ExtMap.Find(pThis);
	}

	const wchar_t* text = L"";

	if(pData) {
		text = pData->LoadScreenSpecialName.Get();
	} else if(n == 0) {
		text = StringTable::LoadString("Name:Para");
	} else if(n > 0 && n <= 9) {
		R->EAX(n);
		return 0x553A28;
	}

	R->EAX(text); // limited to wchar_t[110], must not be null
	return 0x553B3B;
}

DEFINE_HOOK(553D06, LoadProgressMgr_Draw_LSBrief, 6)
{
	GET_STACK(int, n, 0x38);

	HouseTypeExt::ExtData* pData = nullptr;
	if(auto pThis = HouseTypeClass::Array->GetItemOrDefault(n)) {
		pData = HouseTypeExt::ExtMap.Find(pThis);
	}

	const wchar_t* LSBrief = nullptr;

	if(pData) {
		LSBrief = pData->LoadScreenBrief.Get();
	} else if(n == 0) {
		LSBrief = StringTable::LoadString("LoadBrief:USA");
	} else {
		return 0x553D2B;
	}

	R->ESI(LSBrief); // limited to some tiny amount
	return 0x553E54;
}

DEFINE_HOOK(4E3579, HTExt_DrawFlag, 0)
{
	GET(int, n, ECX);

	HouseTypeExt::ExtData* pData = nullptr;
	if(auto pThis = HouseTypeClass::Array->GetItemOrDefault(n)) {
		pData = HouseTypeExt::ExtMap.Find(pThis);
	}

	BSurface* pFlag = nullptr;

	if(pData) {
		pFlag = pData->FlagFile.GetSurface();
	} else if(n == 0) {
		pFlag = PCX::Instance->GetSurface("usai.pcx");
	} else {
		return 0x4E3590;
	}

	R->EAX(pFlag);

	return 0x4E3686;
}

DEFINE_HOOK(72B690, HTExt_LSPAL, 0)
{
	GET(int, n, EDI);

	HouseTypeExt::ExtData* pData = nullptr;
	if(auto pThis = HouseTypeClass::Array->GetItemOrDefault(n)) {
		pData = HouseTypeExt::ExtMap.Find(pThis);
	}

	const char* pPALFile = nullptr;

	if(pData) {
		pPALFile = pData->LoadScreenPalette;
	} else if(n == 0) {
		pPALFile = "mplsu.pal";	//need to recode cause I broke the code with the jump
	} else {
		return 0x72B6B6;
	}

	//some ASM-less magic! =)
	auto ppPalette = reinterpret_cast<BytePalette**>(0xB0FB94);
	auto ppDestination = reinterpret_cast<ConvertClass**>(0xB0FB98);
	ConvertClass::CreateFromFile(pPALFile, *ppPalette, *ppDestination);

	return 0x72B804;
}

DEFINE_HOOK(4E38D8, HTExt_GetSTT, 0)
{
	GET(int, n, ECX);

	HouseTypeExt::ExtData* pData = nullptr;
	if(auto pThis = HouseTypeClass::Array->GetItemOrDefault(n)) {
		pData = HouseTypeExt::ExtMap.Find(pThis);
	}

	const wchar_t* pSTT = nullptr;

	if(pData) {
		pSTT = pData->StatusText.Get();
	} else if(n == 0) {
		pSTT = StringTable::LoadString("STT:PlayerSideAmerica");
	} else {
		return 0x4E38F3;
	}

	R->EAX(pSTT);
	return 0x4E39F1;
}

DEFINE_HOOK(752BA1, HTExt_GetTaunt, 6)
{
	GET(TauntDataStruct, TauntData, ECX);
//	LEA_STACK(char*, pFileName, 0);

	HouseTypeClass* pThis = HouseTypeClass::Array->Items[TauntData.countryIdx];
	HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis);
	if(pData) {
		_snprintf_s(Ares::readBuffer, _TRUNCATE, pData->TauntFile, TauntData.tauntIdx);
		R->EAX(AudioStream::Instance->PlayWAV(Ares::readBuffer, false));
		return 0x752C5F;
	}

	return 0;
}

DEFINE_HOOK(4E3792, HTExt_Unlimit1, 0)
{ return 0x4E37AD; }

DEFINE_HOOK(4E3A9C, HTExt_Unlimit2, 0)
{ return 0x4E3AA1; }

DEFINE_HOOK(4E3F31, HTExt_Unlimit3, 0)
{ return 0x4E3F4C; }

DEFINE_HOOK(4E412C, HTExt_Unlimit4, 0)
{ return 0x4E4147; }

DEFINE_HOOK(4E41A7, HTExt_Unlimit5, 0)
{ return 0x4E41C3; }

//0x69B774
DEFINE_HOOK(69B774, HTExt_PickRandom_Human, 0)
{
	R->EAX(HouseTypeExt::PickRandomCountry());
	return 0x69B788;
}

//0x69B670
DEFINE_HOOK(69B670, HTExt_PickRandom_AI, 0)
{
	R->EAX(HouseTypeExt::PickRandomCountry());
	return 0x69B684;
}

DEFINE_HOOK(4FE782, HouseClass_AI_BaseConstructionUpdate_PickPowerplant, 6)
{
	GET(HouseClass* const, pThis, EBP);
	auto const pExt = HouseTypeExt::ExtMap.Find(pThis->Type);

	constexpr auto const DefaultSize = 10;
	BuildingTypeClass* buffer[DefaultSize];
	DynamicVectorClass<BuildingTypeClass*> Eligible(DefaultSize, buffer);

	auto const it = pExt->GetPowerplants();
	for(auto const& pPower : it) {
		if(HouseExt::PrereqValidate(pThis, pPower, false, true) == 1) {
			Eligible.AddItem(pPower);
		}
	}

	BuildingTypeClass* pResult = nullptr;
	if(Eligible.Count > 0) {
		auto& Random = ScenarioClass::Instance->Random;
		auto const idx = Random.RandomRanged(0, Eligible.Count - 1);
		pResult = Eligible[idx];
	} else if(!it.empty()) {
		pResult = it.at(0);
		Debug::Log(
			"Country [%s] wanted to build a powerplant but does not meet "
			"prerequisites for any possible plant. Going to give it the first "
			"one on the list (%s)\n", pThis->Type->ID, pResult->ID);
	} else {
		Debug::Log(
			"Country [%s] did not find any powerplants it could construct! "
			"The AI's probably going to crash now...\n", pThis->Type->ID);
	}

	R->EDI(pResult);
	return 0x4FE893;
}

// issue #521: sort order for countries / countries can be hidden
DEFINE_HOOK(4E3A6A, hWnd_PopulateWithCountryNames, 6) {
	GET(HWND const, hWnd, ESI);
	
	using Ext_t = HouseTypeExt::ExtData*;
	std::vector<Ext_t> Eligible;

	for(auto const& pCountry : *HouseTypeClass::Array) {
		if(pCountry->Multiplay && pCountry->UIName && *pCountry->UIName) {
			auto const pExt = HouseTypeExt::ExtMap.Find(pCountry);

			if(pExt->CountryListIndex >= 0) {
				Eligible.push_back(pExt);
			}
		}
	}

	auto sortCountries = [](const Ext_t &lhs, const Ext_t &rhs) -> bool {
		if(lhs->CountryListIndex != rhs->CountryListIndex) {
			return lhs->CountryListIndex < rhs->CountryListIndex;
		} else {
			return lhs->OwnerObject()->ArrayIndex2 < rhs->OwnerObject()->ArrayIndex2;
		}
	};

	std::sort(Eligible.begin(), Eligible.end(), sortCountries);

	for(auto pCountryExt : Eligible) {
		auto const pCountry = pCountryExt->OwnerObject();
		auto const idx = SendMessageA(hWnd, WW_CB_ADDITEM, 0, reinterpret_cast<LPARAM>(pCountry->UIName));
		SendMessageA(hWnd, CB_SETITEMDATA, static_cast<WPARAM>(idx), pCountry->ArrayIndex2);
	}
	
	return 0x4E3ACF;
}

DEFINE_HOOK(6AA0CA, TabCameoListClass_Draw_DrawObserverBackground, 6)
{
	enum { DrawSHP = 0x6AA0ED, DontDraw = 0x6AA159 };

	GET(HouseTypeClass *, pCountry, EAX);

	auto pData = HouseTypeExt::ExtMap.Find(pCountry);

	if(pData->ObserverBackgroundSHP) {
		R->EAX<SHPStruct *>(pData->ObserverBackgroundSHP);
		return DrawSHP;
	} else if(auto PCXSurface = pData->ObserverBackground.GetSurface()) {
		GET(int, TLX, EDI);
		GET(int, TLY, EBX);
		RectangleStruct bounds = { TLX, TLY, pData->ObserverBackgroundWidth, pData->ObserverBackgroundHeight };
		PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, PCXSurface);
		return DontDraw;
	} else {
		return DontDraw;
	}
}


DEFINE_HOOK(6AA164, TabCameoListClass_Draw_DrawObserverFlag, 6)
{
	enum { IDontKnowYou = 0x6AA16D, DrawSHP = 0x6AA1DB, DontDraw = 0x6AA2CE };

	GET(HouseTypeClass *, pCountry, EAX);

	auto pData = HouseTypeExt::ExtMap.Find(pCountry);

	if(!pData) {
		R->EAX<HouseTypeClass *>(pCountry);
		R->ECX<int>(pCountry->ArrayIndex2 + 3);
		return IDontKnowYou;
	} else if(pData->ObserverFlagSHP) {
		R->ESI<SHPStruct *>(pData->ObserverFlagSHP);
		R->EAX<int>(!!pData->ObserverFlagYuriPAL ? 9 : 0);
		return DrawSHP;
	} else if(auto PCXSurface = pData->ObserverFlag.GetSurface()) {
		GET(int, TLX, EDI);
		GET(int, TLY, EBX);
		RectangleStruct bounds = { TLX + pData->ObserverFlagPCXX , TLY + pData->ObserverFlagPCXY,
				pData->ObserverFlagPCXWidth, pData->ObserverFlagPCXHeight
		};
		PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, PCXSurface);
		return DontDraw;
	} else {
		return DontDraw;
	}
}

#if 0
// reactivate when testing observer drawing - this will draw observer sidebar instead of your real one in singleplayer
// cameos will not be shown but tooltips and clicking the right spaces will still work
// observer stats will be all zeroes
A_FINE_HOOK(6A964E, TabCameoListClass_Draw_IFilmMyself, 0)
{
	enum { DrawObserver = 0x6AA05B, DrawNormal = 0x6A9654 };

	GET(HouseClass *, HumanHouse, EBX);
	GET(HouseClass *, ObserverHouse, EBP);

	MouseClass::Instance->DiplomacyNumHouses = 1;
	MouseClass::Instance->DiplomacyHouses[0] = HumanHouse;
	MouseClass::Instance->DiplomacyColors[0] = ColorScheme::Array->GetItem(HumanHouse->ColorSchemeIndex);

	return DrawObserver;
}
#endif
