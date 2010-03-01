#include "Body.h"
#include "../HouseType/Body.h"
#include "../../Ares.h"
#include <ScenarioClass.h>

template<> const DWORD Extension<HouseTypeClass>::Canary = 0xAFFEAFFE;
Container<HouseTypeExt> HouseTypeExt::ExtMap;

template<> HouseTypeExt::TT *Container<HouseTypeExt>::SavingObject = NULL;
template<> IStream *Container<HouseTypeExt>::SavingStream = NULL;

void HouseTypeExt::ExtData::InitializeConstants(HouseTypeClass *pThis)
{
	char* pID = pThis->get_ID();

	//We assign default values by country ID rather than index so you simply add a new country
	//without having to specify all the tags for the old ones

	if(!_strcmpi(pID, "Americans"))	//USA
	{
		strcpy(this->FlagFile, "usai.pcx");
		strcpy(this->LSBrief, "LoadBrief:USA");
		strcpy(this->LSFile, "ls%sustates.shp");
		strcpy(this->LSName, "Name:Americans");
		strcpy(this->LSPALFile, "mplsu.pal");
		strcpy(this->LSSpecialName, "Name:Para");
		strcpy(this->StatusText, "STT:PlayerSideAmerica");
		strcpy(this->TauntFile, "taunts\\tauam%02i.wav");
	}
	else if(!_strcmpi(pID, "Alliance"))	//Korea
	{
		strcpy(this->FlagFile, "japi.pcx");
		strcpy(this->LSBrief, "LoadBrief:Korea");
		strcpy(this->LSFile, "ls%skorea.shp");
		strcpy(this->LSName, "Name:Alliance");
		strcpy(this->LSPALFile, "mplsk.pal");
		strcpy(this->LSSpecialName, "Name:BEAGLE");
		strcpy(this->StatusText, "STT:PlayerSideKorea");
		strcpy(this->TauntFile, "taunts\\tauko%02i.wav");
	}
	else if(!_strcmpi(pID, "French"))	//France
	{
		strcpy(this->FlagFile, "frai.pcx");
		strcpy(this->LSBrief, "LoadBrief:French");
		strcpy(this->LSFile, "ls%sfrance.shp");
		strcpy(this->LSName, "Name:French");
		strcpy(this->LSPALFile, "mplsf.pal");
		strcpy(this->LSSpecialName, "Name:GTGCAN");
		strcpy(this->StatusText, "STT:PlayerSideFrance");
		strcpy(this->TauntFile, "taunts\\taufr%02i.wav");
	}
	else if(!_strcmpi(pID, "Germans"))	//Germany
	{
		strcpy(this->FlagFile, "geri.pcx");
		strcpy(this->LSBrief, "LoadBrief:Germans");
		strcpy(this->LSFile, "ls%sgermany.shp");
		strcpy(this->LSName, "Name:Germans");
		strcpy(this->LSPALFile, "mplsg.pal");
		strcpy(this->LSSpecialName, "Name:TNKD");
		strcpy(this->StatusText, "STT:PlayerSideGermany");
		strcpy(this->TauntFile, "taunts\\tauge%02i.wav");
	}
	else if(!_strcmpi(pID, "British"))	//United Kingdom
	{
		strcpy(this->FlagFile, "gbri.pcx");
		strcpy(this->LSBrief, "LoadBrief:British");
		strcpy(this->LSFile, "ls%sukingdom.shp");
		strcpy(this->LSName, "Name:British");
		strcpy(this->LSPALFile, "mplsuk.pal");
		strcpy(this->LSSpecialName, "Name:SNIPE");
		strcpy(this->StatusText, "STT:PlayerSideBritain");
		strcpy(this->TauntFile, "taunts\\taubr%02i.wav");
	}
	else if(!_strcmpi(pID, "Africans"))	//Libya
	{
		strcpy(this->FlagFile, "djbi.pcx");
		strcpy(this->LSBrief, "LoadBrief:Lybia");
		strcpy(this->LSFile, "ls%slibya.shp");
		strcpy(this->LSName, "Name:Africans");
		strcpy(this->LSPALFile, "mplsl.pal");
		strcpy(this->LSSpecialName, "Name:DTRUCK");
		strcpy(this->StatusText, "STT:PlayerSideLibya");
		strcpy(this->TauntFile, "taunts\\tauli%02i.wav");
	}
	else if(!_strcmpi(pID, "Arabs"))	//Iraq
	{
		strcpy(this->FlagFile, "arbi.pcx");
		strcpy(this->LSBrief, "LoadBrief:Iraq");
		strcpy(this->LSFile, "ls%siraq.shp");
		strcpy(this->LSName, "Name:Arabs");
		strcpy(this->LSPALFile, "mplsi.pal");
		strcpy(this->LSSpecialName, "Name:DESO");
		strcpy(this->StatusText, "STT:PlayerSideIraq");
		strcpy(this->TauntFile, "taunts\\tauir%02i.wav");
	}
	else if(!_strcmpi(pID, "Confederation"))	//Cuba
	{
		strcpy(this->FlagFile, "lati.pcx");
		strcpy(this->LSBrief, "LoadBrief:Cuba");
		strcpy(this->LSFile, "ls%scuba.shp");
		strcpy(this->LSName, "Name:Confederation");
		strcpy(this->LSPALFile, "mplsc.pal");
		strcpy(this->LSSpecialName, "Name:TERROR");
		strcpy(this->StatusText, "STT:PlayerSideCuba");
		strcpy(this->TauntFile, "taunts\\taucu%02i.wav");
	}
	else if(!_strcmpi(pID, "Russians"))	//Russia
	{
		strcpy(this->FlagFile, "rusi.pcx");
		strcpy(this->LSBrief, "LoadBrief:Russia");
		strcpy(this->LSFile, "ls%srussia.shp");
		strcpy(this->LSName, "Name:Russians");
		strcpy(this->LSPALFile, "mplsr.pal");
		strcpy(this->LSSpecialName, "Name:TTNK");
		strcpy(this->StatusText, "STT:PlayerSideRussia");
		strcpy(this->TauntFile, "taunts\\tauru%02i.wav");
	}
	else if(!_strcmpi(pID, "YuriCountry"))	//Yuri
	{
		strcpy(this->FlagFile, "yrii.pcx");
		strcpy(this->LSBrief, "LoadBrief:YuriCountry");
		strcpy(this->LSFile, "ls%syuri.shp");
		strcpy(this->LSName, "Name:YuriCountry");
		strcpy(this->LSPALFile, "mpyls.pal");
		strcpy(this->LSSpecialName, "Name:YURI");
		strcpy(this->StatusText, "STT:PlayerSideYuriCountry");
		strcpy(this->TauntFile, "taunts\\tauyu%02i.wav");
	}
	else	//Unknown
	{
		strcpy(this->FlagFile, "rani.pcx");
		strcpy(this->LSBrief, "GUI:Unknown");
		strcpy(this->LSFile, "ls%sobs.shp");
		strcpy(this->LSName, "GUI:Unknown");
		strcpy(this->LSPALFile, "mplsobs.pal");
		strcpy(this->LSSpecialName, "GUI:Unknown");
		strcpy(this->StatusText, "GUI:Unknown");
		strcpy(this->TauntFile, "taunts\\tauam%02i.wav");
	}
	this->RandomSelectionWeight = 1;

	this->_Initialized = is_Constanted;
}

void HouseTypeExt::ExtData::Initialize(HouseTypeClass *pThis)
{
	this->Powerplants.Clear();

	BuildingTypeClass * pPower = NULL;

	switch(pThis->SideIndex) {
		case 0:
			pPower = RulesClass::Global()->GDIPowerPlant;
			break;
		case 1:
			pPower = RulesClass::Global()->NodRegularPower;
			break;
		case 2:
			pPower = RulesClass::Global()->ThirdPowerPlant;
			break;
	}
	if(pPower) {
		this->Powerplants.AddItem(pPower);
	}

	this->_Initialized = is_Inited;
}

void HouseTypeExt::ExtData::LoadFromRulesFile(HouseTypeClass *pThis, CCINIClass *pINI)
{
	char* pID = pThis->ID;

	this->InitializeConstants(pThis);

	if(pINI->ReadString(pID, "File.Flag", "", Ares::readBuffer, Ares::readLength)) {
		strncpy(this->FlagFile, Ares::readBuffer, 0x20);
//		Debug::Log("Got [%s]File.Flag = %s\n", pThis->get_ID(), this->FlagFile);
		//Load PCX File so it can be drawn
		PCX::LoadFile(this->FlagFile);
	}

	if(pINI->ReadString(pID, "File.LoadScreen", "", Ares::readBuffer, Ares::readLength)) {
		strncpy(this->LSFile, Ares::readBuffer, 0x20);
	}

	if(pINI->ReadString(pID, "File.LoadScreenPAL", "", Ares::readBuffer, Ares::readLength)) {
		strncpy(this->LSPALFile, Ares::readBuffer, 0x20);
	}

	if(pINI->ReadString(pID, "File.Taunt", "", Ares::readBuffer, Ares::readLength)) {
		strncpy(this->TauntFile, Ares::readBuffer, 0x20);
	}

	if(pINI->ReadString(pID, "LoadScreenText.Name", "", Ares::readBuffer, Ares::readLength)) {
		strncpy(this->LSName, Ares::readBuffer, 0x20);
	}

	if(pINI->ReadString(pID, "LoadScreenText.SpecialName", "", Ares::readBuffer, Ares::readLength)) {
		strncpy(this->LSSpecialName, Ares::readBuffer, 0x20);
	}

	if(pINI->ReadString(pID, "LoadScreenText.Brief", "", Ares::readBuffer, Ares::readLength)) {
		strncpy(this->LSBrief, Ares::readBuffer, 0x20);
	}

	if(pINI->ReadString(pID, "MenuText.Status", "", Ares::readBuffer, Ares::readLength)) {
		strncpy(this->StatusText, Ares::readBuffer, 0x20);
	}
}

void HouseTypeExt::ExtData::LoadFromINIFile(HouseTypeClass *pThis, CCINIClass *pINI)
{
	char* pID = pThis->ID;

/*
	discarding this - special case, needs to load things even before the rules is done
	if(this->_Initialized != is_Inited) {
		return;
	}
*/

	if(!this->Powerplants.Count) {
		switch (pThis->SideIndex) {
			case 0:
				pINI->ReadString("General", "GDIPowerPlant", "", Ares::readBuffer, Ares::readLength);
				break;
			case 1:
				pINI->ReadString("General", "NodRegularPower", "", Ares::readBuffer, Ares::readLength);
				break;
			case 2:
				pINI->ReadString("General", "ThirdPowerPlant", "", Ares::readBuffer, Ares::readLength);
				break;
		}
		if(strlen(Ares::readBuffer)) {
			if(BuildingTypeClass *pBld = BuildingTypeClass::Find(Ares::readBuffer)) {
				this->Powerplants.AddItem(pBld);
			}
		}
	}

	if(pINI->ReadString(pID, "AI.PowerPlants", "", Ares::readBuffer, Ares::readLength)) {
		this->Powerplants.Clear();
		for(char *bld = strtok(Ares::readBuffer, Ares::readDelims); bld; bld = strtok(NULL, Ares::readDelims)) {
			if(BuildingTypeClass *pBld = BuildingTypeClass::Find(bld)) {
				this->Powerplants.AddItem(pBld);
			}
		}
	}

	this->RandomSelectionWeight = pINI->ReadInteger(pID, "RandomSelectionWeight", this->RandomSelectionWeight);
}

int HouseTypeExt::PickRandomCountry()
{
	std::vector<int> vecLegible;
	HouseTypeClass* pCountry;

	for(int i = 0; i < HouseTypeClass::Array->Count; i++) {
		pCountry = HouseTypeClass::Array->Items[i];
		if(pCountry->Multiplay) {
			if(HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pCountry)) {
				for(int k = 0; k < pData->RandomSelectionWeight; k++) {
					vecLegible.push_back(i);
				}
			}
		}
	}

	if(vecLegible.size() > 0) {
		int pick = ScenarioClass::Instance->Random.RandomRanged(0, vecLegible.size() - 1);

		return vecLegible.at(pick);
	} else {
		Debug::FatalError("No countries eligible for random selection!");
	}
	return 0;
}

// =============================
// load/save

void Container<HouseTypeExt>::Save(HouseTypeClass *pThis, IStream *pStm) {
	HouseTypeExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if(pData) {
		pData->Powerplants.Save(pStm);
	}
}

void Container<HouseTypeExt>::Load(HouseTypeClass *pThis, IStream *pStm) {
	HouseTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	pData->Powerplants.Load(pStm, 1);
}

// =============================
// container hooks

DEFINE_HOOK(511635, HouseTypeClass_CTOR_1, 5)
{
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(511643, HouseTypeClass_CTOR_2, 5)
{
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(512760, HouseTypeClass_DTOR, 6)
{
	GET(HouseTypeClass*, pItem, ECX);

	HouseTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(512290, HouseTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK_AGAIN(512480, HouseTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(HouseTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<HouseTypeExt>::SavingObject = pItem;
	Container<HouseTypeExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(51246D, HouseTypeClass_Load_Suffix, 5)
{
	HouseTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(51255C, HouseTypeClass_Save_Suffix, 5)
{
	HouseTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(51214F, HouseTypeClass_LoadFromINI, 5)
DEFINE_HOOK_AGAIN(51215A, HouseTypeClass_LoadFromINI, 5)
{
	GET(HouseTypeClass*, pItem, EBX);
	GET_BASE(CCINIClass*, pINI, 0x8);

	HouseTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
