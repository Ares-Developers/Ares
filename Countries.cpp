#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif
#pragma warning(disable: 4035)	//"no return value" - there is one, just not in our code ;)

#include "Countries.h"

//Static init
stdext::hash_map<HouseTypeClass*,Countries::CountryExtensionStruct> Countries::CountryExt;

void __stdcall Countries::Construct(HouseTypeClass* pThis)
{
	CountryExtensionStruct Ext;

	char* pID = pThis->get_ID();

	//We assign default values by country ID rather than index so you simply add a new country
	//without having to specify all the tags for the old ones

	if(!_strcmpi(pID, "Americans"))	//USA
	{
		strcpy(Ext.FlagFile, "usai.pcx");
		strcpy(Ext.LSBrief, "LoadBrief:USA");
		strcpy(Ext.LSFile, "ls%sustates.shp");
		strcpy(Ext.LSName, "Name:Americans");
		strcpy(Ext.LSPALFile, "mplsu.pal");
		strcpy(Ext.LSSpecialName, "Name:Para");
		strcpy(Ext.StatusText, "STT:PlayerSideAmerica");
		strcpy(Ext.TauntFile, "taunts\\tauam%02i.wav");
	}
	else if(!_strcmpi(pID, "Alliance"))	//Korea
	{
		strcpy(Ext.FlagFile, "japi.pcx");
		strcpy(Ext.LSBrief, "LoadBrief:Korea");
		strcpy(Ext.LSFile, "ls%skorea.shp");
		strcpy(Ext.LSName, "Name:Alliance");
		strcpy(Ext.LSPALFile, "mplsk.pal");
		strcpy(Ext.LSSpecialName, "Name:BEAGLE");
		strcpy(Ext.StatusText, "STT:PlayerSideKorea");
		strcpy(Ext.TauntFile, "taunts\tauko%02i.wav");
	}
	else if(!_strcmpi(pID, "French"))	//France
	{
		strcpy(Ext.FlagFile, "frai.pcx");
		strcpy(Ext.LSBrief, "LoadBrief:French");
		strcpy(Ext.LSFile, "ls%sfrance.shp");
		strcpy(Ext.LSName, "Name:French");
		strcpy(Ext.LSPALFile, "mplsf.pal");
		strcpy(Ext.LSSpecialName, "Name:GTGCAN");
		strcpy(Ext.StatusText, "STT:PlayerSideFrance");
		strcpy(Ext.TauntFile, "taunts\taufr%02i.wav");
	}
	else if(!_strcmpi(pID, "Germans"))	//Germany
	{
		strcpy(Ext.FlagFile, "geri.pcx");
		strcpy(Ext.LSBrief, "LoadBrief:Germans");
		strcpy(Ext.LSFile, "ls%sgermany.shp");
		strcpy(Ext.LSName, "Name:Germans");
		strcpy(Ext.LSPALFile, "mplsg.pal");
		strcpy(Ext.LSSpecialName, "Name:TNKD");
		strcpy(Ext.StatusText, "STT:PlayerSideGermany");
		strcpy(Ext.TauntFile, "taunts\tauge%02i.wav");
	}
	else if(!_strcmpi(pID, "British"))	//United Kingdom
	{
		strcpy(Ext.FlagFile, "gbri.pcx");
		strcpy(Ext.LSBrief, "LoadBrief:British");
		strcpy(Ext.LSFile, "ls%sukingdom.shp");
		strcpy(Ext.LSName, "Name:British");
		strcpy(Ext.LSPALFile, "mplsuk.pal");
		strcpy(Ext.LSSpecialName, "Name:SNIPE");
		strcpy(Ext.StatusText, "STT:PlayerSideBritain");
		strcpy(Ext.TauntFile, "taunts\taubr%02i.wav");
	}
	else if(!_strcmpi(pID, "Africans"))	//Libya
	{
		strcpy(Ext.FlagFile, "djbi.pcx");
		strcpy(Ext.LSBrief, "LoadBrief:Lybia");
		strcpy(Ext.LSFile, "ls%slibya.shp");
		strcpy(Ext.LSName, "Name:Africans");
		strcpy(Ext.LSPALFile, "mplsl.pal");
		strcpy(Ext.LSSpecialName, "Name:DTRUCK");
		strcpy(Ext.StatusText, "STT:PlayerSideLibya");
		strcpy(Ext.TauntFile, "taunts\tauli%02i.wav");
	}
	else if(!_strcmpi(pID, "Arabs"))	//Iraq
	{
		strcpy(Ext.FlagFile, "arbi.pcx");
		strcpy(Ext.LSBrief, "LoadBrief:Iraq");
		strcpy(Ext.LSFile, "ls%siraq.shp");
		strcpy(Ext.LSName, "Name:Arabs");
		strcpy(Ext.LSPALFile, "mplsi.pal");
		strcpy(Ext.LSSpecialName, "Name:DESO");
		strcpy(Ext.StatusText, "STT:PlayerSideIraq");
		strcpy(Ext.TauntFile, "taunts\tauir%02i.wav");
	}
	else if(!_strcmpi(pID, "Confederation"))	//Cuba
	{
		strcpy(Ext.FlagFile, "lati.pcx");
		strcpy(Ext.LSBrief, "LoadBrief:Cuba");
		strcpy(Ext.LSFile, "ls%scuba.shp");
		strcpy(Ext.LSName, "Name:Confederation");
		strcpy(Ext.LSPALFile, "mplsc.pal");
		strcpy(Ext.LSSpecialName, "Name:TERROR");
		strcpy(Ext.StatusText, "STT:PlayerSideCuba");
		strcpy(Ext.TauntFile, "taunts\taucu%02i.wav");
	}
	else if(!_strcmpi(pID, "Russians"))	//Russia
	{
		strcpy(Ext.FlagFile, "rusi.pcx");
		strcpy(Ext.LSBrief, "LoadBrief:Russia");
		strcpy(Ext.LSFile, "ls%srussia.shp");
		strcpy(Ext.LSName, "Name:Russians");
		strcpy(Ext.LSPALFile, "mplsr.pal");
		strcpy(Ext.LSSpecialName, "Name:TTNK");
		strcpy(Ext.StatusText, "STT:PlayerSideRussia");
		strcpy(Ext.TauntFile, "taunts\tauru%02i.wav");
	}
	else if(!_strcmpi(pID, "YuriCountry"))	//Yuri
	{
		strcpy(Ext.FlagFile, "yrii.pcx");
		strcpy(Ext.LSBrief, "LoadBrief:YuriCountry");
		strcpy(Ext.LSFile, "ls%syuri.shp");
		strcpy(Ext.LSName, "Name:YuriCountry");
		strcpy(Ext.LSPALFile, "mpyls.pal");
		strcpy(Ext.LSSpecialName, "Name:YURI");
		strcpy(Ext.StatusText, "STT:PlayerSideYuriCountry");
		strcpy(Ext.TauntFile, "taunts\tauyu%02i.wav");
	}
	else	//Unknown
	{
		strcpy(Ext.FlagFile, "rani.pcx");
		strcpy(Ext.LSBrief, "GUI:Unknown");
		strcpy(Ext.LSFile, "ls%sobs.shp");
		strcpy(Ext.LSName, "GUI:Unknown");
		strcpy(Ext.LSPALFile, "mplsobs.pal");
		strcpy(Ext.LSSpecialName, "GUI:Unknown");
		strcpy(Ext.StatusText, "GUI:Unknown");
		strcpy(Ext.TauntFile, "taunts\tauam%02i.wav");
	}
	Ext.RandomSelectionWeight = 1;

	CountryExt[pThis] = Ext;
}

void __stdcall Countries::LoadFromINI(HouseTypeClass* pThis, CCINIClass* pINI)
{
	if(pINI && CountryExt.find(pThis) != CountryExt.end())
	{
		CountryExtensionStruct* pExt = &CountryExt[pThis];

		char buffer[0x80] = "\0";
		char* pID = pThis->get_ID();

		if(pINI->ReadString(pID, "FlagFile", "", buffer, 0x80))
		{
			strncpy(pExt->FlagFile, buffer, 0x20);

			//Load PCX File so it can be drawn
			PCX::LoadFile(pExt->FlagFile);
		}

		if(pINI->ReadString(pID, "LSFile", "", buffer, 0x80))
			strncpy(pExt->LSFile, buffer, 0x20);

		if(pINI->ReadString(pID, "LSPALFile", "", buffer, 0x80))
			strncpy(pExt->LSPALFile, buffer, 0x20);

		if(pINI->ReadString(pID, "TauntFile", "", buffer, 0x80))
			strncpy(pExt->FlagFile, buffer, 0x20);

		if(pINI->ReadString(pID, "LSName", "", buffer, 0x80))
			strncpy(pExt->LSName, buffer, 0x20);

		if(pINI->ReadString(pID, "LSSpecialName", "", buffer, 0x80))
			strncpy(pExt->LSSpecialName, buffer, 0x20);

		if(pINI->ReadString(pID, "LSBrief", "", buffer, 0x80))
			strncpy(pExt->LSBrief, buffer, 0x20);

		if(pINI->ReadString(pID, "StatusText", "", buffer, 0x80))
			strncpy(pExt->StatusText, buffer, 0x20);

		pExt->RandomSelectionWeight = pINI->ReadInteger(pID, "RandomSelectionWeight", pExt->RandomSelectionWeight);
	}
}

//0x5536DA
EXPORT Countries_GetLSName(REGISTERS* R)
{
	int n = R->get_EBX();
	HouseTypeClass* pThis = HouseTypeClass::Array->GetItem(n);

	char* pLSName = NULL;

	if(Countries::CountryExt.find(pThis) != Countries::CountryExt.end())
		pLSName = Countries::CountryExt[pThis].LSName;
	else if(n == 0)
		pLSName = "Name:Americans";
	else 
		return 0x5536FB;

	R->set_EDI((DWORD)StringTable::LoadString(pLSName));
	return 0x553820;
}

//0x553A05
EXPORT Countries_GetLSSpecialName(REGISTERS* R)
{
	int n = R->get_StackVar32(0x38);
	HouseTypeClass* pThis = HouseTypeClass::Array->GetItem(n);

	if(Countries::CountryExt.find(pThis) != Countries::CountryExt.end())
	{
		R->set_EAX((DWORD)StringTable::LoadString(Countries::CountryExt[pThis].LSSpecialName));
		return 0x553B3B;
	}

	return 0;
}

//0x553D06
EXPORT Countries_GetLSBrief(REGISTERS* R)
{
	int n = R->get_StackVar32(0x38);
	HouseTypeClass* pThis = HouseTypeClass::Array->GetItem(n);

	if(Countries::CountryExt.find(pThis) != Countries::CountryExt.end())
	{
		R->set_ESI((DWORD)StringTable::LoadString(Countries::CountryExt[pThis].LSBrief));
		return 0x553E54;
	}

	return 0;
}

//0x4E3579
EXPORT Countries_DrawFlag(REGISTERS* R)
{
	int n = R->get_ECX();
	HouseTypeClass* pThis = HouseTypeClass::Array->GetItem(n);
	
	char* pFlagFile = NULL;

	if(Countries::CountryExt.find(pThis) != Countries::CountryExt.end())
		pFlagFile = Countries::CountryExt[pThis].FlagFile;
	else if(n == 0)
		pFlagFile = "usai.pcx";
	else
		return 0x4E3590;

	R->set_EAX((DWORD)PCX::GetSurface(pFlagFile));

	return 0x4E3686;
}

//0x72B690
EXPORT Countries_LSPAL(REGISTERS* R)
{
	int n = R->get_EDI();
	HouseTypeClass* pThis = HouseTypeClass::Array->GetItem(n);

	char* pPALFile = NULL;

	if(Countries::CountryExt.find(pThis) != Countries::CountryExt.end())
		pPALFile = Countries::CountryExt[pThis].LSPALFile;
	else if(n == 0)
		pPALFile = "mplsu.pal";	//need to recode cause I broke the code with the jump
	else
		return 0x72B6B6;

	//some ASM magic! =)
	PUSH_IMM(0xB0FB98);
	SET_REG32(edx, 0xB0FB94);
	SET_REG32(ecx, pPALFile);
	CALL(0x72ADE0);

	return 0x72B804;
}

//0x4E38D8
EXPORT Countries_GetSTT(REGISTERS* R)
{
	int n = R->get_ECX();
	HouseTypeClass* pThis = HouseTypeClass::Array->GetItem(n);

	char* pSTT = NULL;

	if(Countries::CountryExt.find(pThis) != Countries::CountryExt.end())
		pSTT = Countries::CountryExt[pThis].StatusText;
	else if(n == 0)
		pSTT = "STT:PlayerSideAmerica";
	else 
		return 0x4E38F3;

	R->set_EAX((DWORD)StringTable::LoadString(pSTT));
	return 0x4E39F1;
}

//0x553412
EXPORT Countries_LSFile(REGISTERS* R)
{
	int n = R->get_EBX();
	HouseTypeClass* pThis = HouseTypeClass::Array->GetItem(n);

	char* pLSFile = NULL;

	if(Countries::CountryExt.find(pThis) != Countries::CountryExt.end())
		pLSFile = Countries::CountryExt[pThis].LSFile;
	else if(n == 0)
		pLSFile = "ls%sustates.shp";
	else
		return 0x553421;

	R->set_EDX((DWORD)pLSFile);
	return 0x55342C;
}

//0x752BA1
EXPORT Countries_GetTaunt(REGISTERS* R)
{
	char* pFileName = (char*)R->get_ESP() + 0x04;
	int nTaunt = R->get_CL() & 0xF;
	int nCountry = (R->get_CL() >> 4) & 0xF;	//ARF 16-country-limit >.<

	HouseTypeClass* pThis = HouseTypeClass::Array->GetItem(nCountry);
	if(Countries::CountryExt.find(pThis) != Countries::CountryExt.end())
	{
		sprintf(pFileName, Countries::CountryExt[pThis].TauntFile, nTaunt);

		R->set_ECX(*((DWORD*)0xB1D4D8));
		return 0x752C54;
	}

	return 0;
}

//0x4E3792
EXPORT Countries_Unlimit1(REGISTERS* R)
{ return 0x4E37AD; }

//0x4E3A9C
EXPORT Countries_Unlimit2(REGISTERS* R)
{ return 0x4E3AA1; }

//0x4E3F31
EXPORT Countries_Unlimit3(REGISTERS* R)
{ return 0x4E3F4C; }

//0x4E412C
EXPORT Countries_Unlimit4(REGISTERS* R)
{ return 0x4E4147; }

//0x4E41A7
EXPORT Countries_Unlimit5(REGISTERS* R)
{ return 0x4E41C3; }

//Helper function
int Countries::PickRandomCountry()
{
	std::vector<int> vecLegible;
	HouseTypeClass* pCountry;

	for(int i = 0; i < HouseTypeClass::Array->get_Count(); i++)
	{
		pCountry = HouseTypeClass::Array->GetItem(i);
		if(pCountry->get_Multiplay())
		{
			if(Countries::CountryExt.find(pCountry) != Countries::CountryExt.end())
			{
				for(int k = 0; k < Countries::CountryExt[pCountry].RandomSelectionWeight; k++)
					vecLegible.push_back(i);
			}
			else
				vecLegible.push_back(i);
		}
	}

	if(vecLegible.size() > 0)
	{
		//Scenario->Random.Ranged(0, vecLegible.size() - 1)
		BYTE* pScen;
		MEM_READ32(pScen, 0xA8B230);

		BYTE* pRand = pScen + 0x218;
		int max = vecLegible.size() - 1;

		PUSH_VAR32(max);
		PUSH_IMM(0);
		THISCALL_EX(pRand, 0x65C7E0);

		int pick;
		GET_REG32(pick, eax);

		return vecLegible[pick];
	}
	else
		return 0;
}

//0x69B774
EXPORT Countries_PickRandom_Human(REGISTERS* R)
{
	R->set_EAX(Countries::PickRandomCountry());
	return 0x69B788;
}

//0x69B670
EXPORT Countries_PickRandom_AI(REGISTERS* R)
{
	R->set_EAX(Countries::PickRandomCountry());
	return 0x69B684;
}
