#include "Body.h"
#include "../House/Body.h"
#include "../../Ares.h"
#include <Audio.h>
#include <ScenarioClass.h>

DEFINE_HOOK(553412, LoadProgressMgr_Draw_LSFile, 0)
{
	int n = R->EBX();
	HouseTypeClass* pThis = HouseTypeClass::Array->Items[n];

	char* pLSFile = NULL;

	if(HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis)) {
		pLSFile = pData->LSFile;
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
	int n = R->EBX();
	HouseTypeClass* pThis = HouseTypeClass::Array->Items[n];

	char* pLSName = NULL;

	if(HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis)) {
		pLSName = pData->LSName;
	} else if(n == 0) {
		pLSName = "Name:Americans";
	} else {
		return 0x5536FB;
	}

	R->EDI(StringTable::LoadString(pLSName));
	return 0x553820;
}

DEFINE_HOOK(553A05, LoadProgressMgr_Draw_LSSpecialName, 6)
{
	int n = R->Stack32(0x38);
	HouseTypeClass* pThis = HouseTypeClass::Array->Items[n];

	if(HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis)) {
		R->EAX(StringTable::LoadString(pData->LSSpecialName)); // limited to wchar_t[110]
		return 0x553B3B;
	}

	return 0;
}

DEFINE_HOOK(553D06, LoadProgressMgr_Draw_LSBrief, 6)
{
	int n = R->Stack32(0x38);
	HouseTypeClass* pThis = HouseTypeClass::Array->Items[n];

	if(HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis)) {
		R->ESI(StringTable::LoadString(pData->LSBrief)); // limited to some tiny amount
		return 0x553E54;
	}

	return 0;
}

DEFINE_HOOK(4E3579, HTExt_DrawFlag, 0)
{
	int n = R->ECX();
	HouseTypeClass* pThis = HouseTypeClass::Array->Items[n];

	char* pFlagFile = NULL;

	if(HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis)) {
		pFlagFile = pData->FlagFile;
	} else if(n == 0) {
		pFlagFile = "usai.pcx";
	} else {
		return 0x4E3590;
	}

	R->EAX(PCX::Instance->GetSurface(pFlagFile));

	return 0x4E3686;
}

DEFINE_HOOK(72B690, HTExt_LSPAL, 0)
{
	int n = R->EDI();
	HouseTypeClass* pThis = HouseTypeClass::Array->Items[n];

	char* pPALFile = NULL;

	if(HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis)) {
		pPALFile = pData->LSPALFile;
	} else if(n == 0) {
		pPALFile = "mplsu.pal";	//need to recode cause I broke the code with the jump
	} else {
		return 0x72B6B6;
	}

	//some ASM magic! =)
	PUSH_IMM(0xB0FB98);
	SET_REG32(edx, 0xB0FB94);
	SET_REG32(ecx, pPALFile);
	CALL(0x72ADE0);

	return 0x72B804;
}

DEFINE_HOOK(4E38D8, HTExt_GetSTT, 0)
{
	int n = R->ECX();
	HouseTypeClass* pThis = HouseTypeClass::Array->Items[n];

	char* pSTT = NULL;

	if(HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis)) {
		pSTT = pData->StatusText;
	} else if(n == 0) {
		pSTT = "STT:PlayerSideAmerica";
	} else {
		return 0x4E38F3;
	}

	R->EAX(StringTable::LoadString(pSTT));
	return 0x4E39F1;
}

DEFINE_HOOK(752BA1, HTExt_GetTaunt, 6)
{
	GET(TauntDataStruct, TauntData, ECX);
//	LEA_STACK(char*, pFileName, 0);

	HouseTypeClass* pThis = HouseTypeClass::Array->Items[TauntData.countryIdx];
	HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis);
	if(pData) {
		_snprintf(Ares::readBuffer, Ares::readLength, pData->TauntFile, TauntData.tauntIdx);
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

DEFINE_HOOK(4FE782, HTExt_PickPowerplant, 6)
{
	GET(HouseClass *, H, EBP);
	HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(H->Type);

	std::vector<BuildingTypeClass *> Eligible;

	for(int i = 0; i < pData->Powerplants.Count; ++i) {
		BuildingTypeClass *pPower = pData->Powerplants[i];
		if(HouseExt::PrereqValidate(H, pPower, 0, 1) == 1) {
			Eligible.push_back(pPower);
		}
	}

	BuildingTypeClass *pResult = NULL;
	if(Eligible.size() > 0) {
		int idx = ScenarioClass::Instance->Random.RandomRanged(0, Eligible.size() - 1);
		pResult = Eligible.at(idx);
	} else if(pData->Powerplants.Count) {
		pResult = pData->Powerplants[0];
		Debug::Log("Country [%s] wanted to build a powerplant but does not meet prerequisites for any possible plant. Going to give it the first one on the list (%s)\n", H->Type->ID, pResult->ID);
	} else {
		Debug::Log("Country [%s] did not find any powerplants it could construct! The AI's probably going to crash now...\n", H->Type->ID);
	}


	R->EDI(pResult);
	return 0x4FE893;
}

// issue #521: sort order for countries / countries can be hidden
DEFINE_HOOK(4E3A6A, hWnd_PopulateWithCountryNames, 6) {
	GET(HWND, hWnd, ESI);
	
	std::vector<HouseTypeExt::ExtData*> Eligible;

	for(int i=0; i<HouseTypeClass::Array->Count; ++i) {
		if(HouseTypeClass* pCountry = HouseTypeClass::Array->GetItem(i)) {
			if(pCountry->Multiplay && pCountry->UIName && *pCountry->UIName) {
				HouseTypeExt::ExtData *pExt = HouseTypeExt::ExtMap.Find(pCountry);

				if(pExt->CountryListIndex >= 0) {
					Eligible.push_back(pExt);
				}
			}
		}
	}

	auto sortCountries = [](const HouseTypeExt::ExtData* a, const HouseTypeExt::ExtData* b) -> bool {
		if(a->CountryListIndex != b->CountryListIndex) {
			return a->CountryListIndex < b->CountryListIndex;
		} else {
			return a->AttachedToObject->ArrayIndex2 < b->AttachedToObject->ArrayIndex2;
		}
	};

	std::sort(Eligible.begin(), Eligible.end(), sortCountries);

	for(std::vector<HouseTypeExt::ExtData*>::iterator iterator = Eligible.begin(); iterator != Eligible.end(); iterator++) {
		int idx = SendMessageA(hWnd, 0x4C2u, 0, (LPARAM)(*iterator)->AttachedToObject->UIName);
		SendMessageA(hWnd, CB_SETITEMDATA, idx, (*iterator)->AttachedToObject->ArrayIndex2);
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
	} else if(*pData->ObserverBackground) {
		if(auto PCXSurface = PCX::Instance->GetSurface(pData->ObserverBackground)) {
			GET(int, TLX, EDI);
			GET(int, TLY, EBX);
			RectangleStruct bounds = { TLX, TLY, pData->ObserverBackgroundWidth, pData->ObserverBackgroundHeight };
			PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, PCXSurface);
		}
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
	} else if(*pData->ObserverFlag) {
		if(auto PCXSurface = PCX::Instance->GetSurface(pData->ObserverFlag)) {
			GET(int, TLX, EDI);
			GET(int, TLY, EBX);
			RectangleStruct bounds = { TLX + pData->ObserverFlagPCXX , TLY + pData->ObserverFlagPCXY,
					pData->ObserverFlagPCXWidth, pData->ObserverFlagPCXHeight
			};
			PCX::Instance->BlitToSurface(&bounds, DSurface::Sidebar, PCXSurface);
		}
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
