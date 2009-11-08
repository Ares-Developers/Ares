#include "Body.h"
#include "..\House\Body.h"

#include <ScenarioClass.h>

DEFINE_HOOK(5536DA, HTExt_GetLSName, 0)
{
	int n = R->get_EBX();
	HouseTypeClass* pThis = HouseTypeClass::Array->Items[n];

	char* pLSName = NULL;

	HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis);
	if(pData) {
		pLSName = pData->LSName;
	} else if(n == 0) {
		pLSName = "Name:Americans";
	} else {
		return 0x5536FB;
	}

	R->set_EDI((DWORD)StringTable::LoadString(pLSName));
	return 0x553820;
}

DEFINE_HOOK(553A05, HTExt_GetLSSpecialName, 6)
{
	int n = R->get_StackVar32(0x38);
	HouseTypeClass* pThis = HouseTypeClass::Array->Items[n];

	HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis);
	if(pData) {
		R->set_EAX((DWORD)StringTable::LoadString(pData->LSSpecialName));
		return 0x553B3B;
	}

	return 0;
}

DEFINE_HOOK(553D06, HTExt_GetLSBrief, 6)
{
	int n = R->get_StackVar32(0x38);
	HouseTypeClass* pThis = HouseTypeClass::Array->Items[n];

	HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis);
	if(pData) {
		R->set_ESI((DWORD)StringTable::LoadString(pData->LSBrief));
		return 0x553E54;
	}

	return 0;
}

DEFINE_HOOK(4E3579, HTExt_DrawFlag, 0)
{
	int n = R->get_ECX();
	HouseTypeClass* pThis = HouseTypeClass::Array->Items[n];
	
	char* pFlagFile = NULL;

//	Debug::Log("Flag of %s\n", pThis->get_ID());

	HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis);
	if(pData) {
		pFlagFile = pData->FlagFile;
	} else if(n == 0) {
		pFlagFile = "usai.pcx";
	} else {
		return 0x4E3590;
	}

//	Debug::Log("Flag resolves to %s\n", pFlagFile);
	R->set_EAX((DWORD)PCX::GetSurface(pFlagFile));

	return 0x4E3686;
}

DEFINE_HOOK(72B690, HTExt_LSPAL, 0)
{
	int n = R->get_EDI();
	HouseTypeClass* pThis = HouseTypeClass::Array->Items[n];

	char* pPALFile = NULL;

	HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis);
	if(pData) {
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
	int n = R->get_ECX();
	HouseTypeClass* pThis = HouseTypeClass::Array->Items[n];

	char* pSTT = NULL;

	HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis);
	if(pData) {
		pSTT = pData->StatusText;
	} else if(n == 0) {
		pSTT = "STT:PlayerSideAmerica";
	} else {
		return 0x4E38F3;
	}

	R->set_EAX((DWORD)StringTable::LoadString(pSTT));
	return 0x4E39F1;
}

DEFINE_HOOK(553412, HTExt_LSFile, 0)
{
	int n = R->get_EBX();
	HouseTypeClass* pThis = HouseTypeClass::Array->Items[n];

	char* pLSFile = NULL;

	HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis);
	if(pData) {
		pLSFile = pData->LSFile;
	} else if(n == 0) {
		pLSFile = "ls%sustates.shp";
	} else {
		return 0x553421;
	}

	R->set_EDX((DWORD)pLSFile);
	return 0x55342C;
}

DEFINE_HOOK(752BA1, HTExt_GetTaunt, 6)
{
	char* pFileName = (char*)R->get_ESP() + 0x04;
	int nTaunt = R->get_CL() & 0xF;
	int nCountry = (R->get_CL() >> 4) & 0xF;	//ARF 16-country-limit >.<

	HouseTypeClass* pThis = HouseTypeClass::Array->Items[nCountry];
	HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pThis);
	if(pData) {
		_snprintf(pFileName, 32, pData->TauntFile, nTaunt);
		R->set_ECX(*((DWORD*)0xB1D4D8));
		return 0x752C54;
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
	R->set_EAX(HouseTypeExt::PickRandomCountry());
	return 0x69B788;
}

//0x69B670
DEFINE_HOOK(69B670, HTExt_PickRandom_AI, 0)
{
	R->set_EAX(HouseTypeExt::PickRandomCountry());
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
	if(Eligible.size() == 0) {
		char message [0x100];
		_snprintf(message, 0x100, "Country [%s] did not find any powerplants it could construct!", H->Type);
		Debug::FatalError(message);
	}
	BuildingTypeClass *pResult = NULL;
	int idx = ScenarioClass::Global()->get_Random()->RandomRanged(0, Eligible.size() - 1);
	pResult = Eligible.at(idx);

	R->set_EDI((DWORD)pResult);
	return 0x4FE893;
}
