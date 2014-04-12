#include "Body.h"
#include "../Side/Body.h"
#include "../../Ares.h"
#include "../../Ares.CRT.h"
#include "../../Utilities/TemplateDef.h"
#include <ScenarioClass.h>
#include <ColorScheme.h>
#include <DiscreteDistributionClass.h>
#include <PCX.h>

#include <iterator>
#include <algorithm>

template<> const DWORD Extension<HouseTypeClass>::Canary = 0xAFFEAFFE;
Container<HouseTypeExt> HouseTypeExt::ExtMap;

template<> HouseTypeExt::TT *Container<HouseTypeExt>::SavingObject = nullptr;
template<> IStream *Container<HouseTypeExt>::SavingStream = nullptr;

void HouseTypeExt::ExtData::InitializeConstants(HouseTypeClass *pThis) {
	char* pID = pThis->ID;

	//We assign default values by country ID rather than index so you simply add a new country
	//without having to specify all the tags for the old ones

	if (!_strcmpi(pID, "Americans")) //USA
	{
		strcpy(this->FlagFile, "usai.pcx");
		strcpy(this->LSBrief, "LoadBrief:USA");
		strcpy(this->LSFile, "ls%sustates.shp");
		strcpy(this->LSName, "Name:Americans");
		strcpy(this->LSPALFile, "mplsu.pal");
		strcpy(this->LSSpecialName, "Name:Para");
		strcpy(this->StatusText, "STT:PlayerSideAmerica");
		strcpy(this->TauntFile, "taunts\\tauam%02i.wav");
		strcpy(this->ObserverBackground, "obsalli.shp");
		strcpy(this->ObserverFlag, "usai.shp");
	} else if (!_strcmpi(pID, "Alliance")) //Korea
	{
		strcpy(this->FlagFile, "japi.pcx");
		strcpy(this->LSBrief, "LoadBrief:Korea");
		strcpy(this->LSFile, "ls%skorea.shp");
		strcpy(this->LSName, "Name:Alliance");
		strcpy(this->LSPALFile, "mplsk.pal");
		strcpy(this->LSSpecialName, "Name:BEAGLE");
		strcpy(this->StatusText, "STT:PlayerSideKorea");
		strcpy(this->TauntFile, "taunts\\tauko%02i.wav");
		strcpy(this->ObserverBackground, "obsalli.shp");
		strcpy(this->ObserverFlag, "japi.shp");
	} else if (!_strcmpi(pID, "French")) //France
	{
		strcpy(this->FlagFile, "frai.pcx");
		strcpy(this->LSBrief, "LoadBrief:French");
		strcpy(this->LSFile, "ls%sfrance.shp");
		strcpy(this->LSName, "Name:French");
		strcpy(this->LSPALFile, "mplsf.pal");
		strcpy(this->LSSpecialName, "Name:GTGCAN");
		strcpy(this->StatusText, "STT:PlayerSideFrance");
		strcpy(this->TauntFile, "taunts\\taufr%02i.wav");
		strcpy(this->ObserverBackground, "obsalli.shp");
		strcpy(this->ObserverFlag, "frai.shp");
	} else if (!_strcmpi(pID, "Germans")) //Germany
	{
		strcpy(this->FlagFile, "geri.pcx");
		strcpy(this->LSBrief, "LoadBrief:Germans");
		strcpy(this->LSFile, "ls%sgermany.shp");
		strcpy(this->LSName, "Name:Germans");
		strcpy(this->LSPALFile, "mplsg.pal");
		strcpy(this->LSSpecialName, "Name:TNKD");
		strcpy(this->StatusText, "STT:PlayerSideGermany");
		strcpy(this->TauntFile, "taunts\\tauge%02i.wav");
		strcpy(this->ObserverBackground, "obsalli.shp");
		strcpy(this->ObserverFlag, "geri.shp");
	} else if (!_strcmpi(pID, "British")) //United Kingdom
	{
		strcpy(this->FlagFile, "gbri.pcx");
		strcpy(this->LSBrief, "LoadBrief:British");
		strcpy(this->LSFile, "ls%sukingdom.shp");
		strcpy(this->LSName, "Name:British");
		strcpy(this->LSPALFile, "mplsuk.pal");
		strcpy(this->LSSpecialName, "Name:SNIPE");
		strcpy(this->StatusText, "STT:PlayerSideBritain");
		strcpy(this->TauntFile, "taunts\\taubr%02i.wav");
		strcpy(this->ObserverBackground, "obsalli.shp");
		strcpy(this->ObserverFlag, "gbri.shp");
	} else if (!_strcmpi(pID, "Africans")) //Libya
	{
		strcpy(this->FlagFile, "djbi.pcx");
		strcpy(this->LSBrief, "LoadBrief:Lybia");
		strcpy(this->LSFile, "ls%slibya.shp");
		strcpy(this->LSName, "Name:Africans");
		strcpy(this->LSPALFile, "mplsl.pal");
		strcpy(this->LSSpecialName, "Name:DTRUCK");
		strcpy(this->StatusText, "STT:PlayerSideLibya");
		strcpy(this->TauntFile, "taunts\\tauli%02i.wav");
		strcpy(this->ObserverBackground, "obssovi.shp");
		strcpy(this->ObserverFlag, "djbi.shp");
	} else if (!_strcmpi(pID, "Arabs")) //Iraq
	{
		strcpy(this->FlagFile, "arbi.pcx");
		strcpy(this->LSBrief, "LoadBrief:Iraq");
		strcpy(this->LSFile, "ls%siraq.shp");
		strcpy(this->LSName, "Name:Arabs");
		strcpy(this->LSPALFile, "mplsi.pal");
		strcpy(this->LSSpecialName, "Name:DESO");
		strcpy(this->StatusText, "STT:PlayerSideIraq");
		strcpy(this->TauntFile, "taunts\\tauir%02i.wav");
		strcpy(this->ObserverBackground, "obssovi.shp");
		strcpy(this->ObserverFlag, "arbi.shp");
	} else if (!_strcmpi(pID, "Confederation")) //Cuba
	{
		strcpy(this->FlagFile, "lati.pcx");
		strcpy(this->LSBrief, "LoadBrief:Cuba");
		strcpy(this->LSFile, "ls%scuba.shp");
		strcpy(this->LSName, "Name:Confederation");
		strcpy(this->LSPALFile, "mplsc.pal");
		strcpy(this->LSSpecialName, "Name:TERROR");
		strcpy(this->StatusText, "STT:PlayerSideCuba");
		strcpy(this->TauntFile, "taunts\\taucu%02i.wav");
		strcpy(this->ObserverBackground, "obssovi.shp");
		strcpy(this->ObserverFlag, "lati.shp");
	} else if (!_strcmpi(pID, "Russians")) //Russia
	{
		strcpy(this->FlagFile, "rusi.pcx");
		strcpy(this->LSBrief, "LoadBrief:Russia");
		strcpy(this->LSFile, "ls%srussia.shp");
		strcpy(this->LSName, "Name:Russians");
		strcpy(this->LSPALFile, "mplsr.pal");
		strcpy(this->LSSpecialName, "Name:TTNK");
		strcpy(this->StatusText, "STT:PlayerSideRussia");
		strcpy(this->TauntFile, "taunts\\tauru%02i.wav");
		strcpy(this->ObserverBackground, "obssovi.shp");
		strcpy(this->ObserverFlag, "rusi.shp");
	} else if (!_strcmpi(pID, "YuriCountry")) //Yuri
	{
		strcpy(this->FlagFile, "yrii.pcx");
		strcpy(this->LSBrief, "LoadBrief:YuriCountry");
		strcpy(this->LSFile, "ls%syuri.shp");
		strcpy(this->LSName, "Name:YuriCountry");
		strcpy(this->LSPALFile, "mpyls.pal");
		strcpy(this->LSSpecialName, "Name:YURI");
		strcpy(this->StatusText, "STT:PlayerSideYuriCountry");
		strcpy(this->TauntFile, "taunts\\tauyu%02i.wav");
		strcpy(this->ObserverBackground, "obsyuri.shp");
		strcpy(this->ObserverFlag, "yrii.shp");
		this->ObserverFlagYuriPAL = true;
	} else //Unknown
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
	this->CountryListIndex = 100;
}

void HouseTypeExt::ExtData::Initialize(HouseTypeClass *pThis) {
	switch (pThis->SideIndex) {
	case 0:
		this->LoadTextColor = ColorScheme::FindIndex("AlliedLoad");
		break;
	case 1:
		this->LoadTextColor = ColorScheme::FindIndex("SovietLoad");
		break;
	case 2:
		this->LoadTextColor = ColorScheme::FindIndex("YuriLoad");
		if(this->LoadTextColor == -1) {
			// there is no YuriLoad in the original game. fall
			// back to a decent value.
			this->LoadTextColor = ColorScheme::FindIndex("Purple");
		}
		break;
	}
}

void HouseTypeExt::ExtData::LoadFromRulesFile(HouseTypeClass *pThis, CCINIClass *pINI) {
	char* pID = pThis->ID;

	this->InitializeConstants(pThis);

	// ppShp is optional. if not set, only PCX is supported
	auto ReadShpOrPcxImage = [&](const char* key, char* pBuffer, size_t cbBuffer, SHPStruct** ppShp) {
		// read the key and convert it to lower case
		if(pINI->ReadString(pID, key, pBuffer, Ares::readBuffer, Ares::readLength)) {
			AresCRT::strCopy(pBuffer, Ares::readBuffer, cbBuffer);
			_strlwr_s(pBuffer, cbBuffer);

			// parse the value
			if(INIClass::IsBlank(pBuffer)) {
				// explicitly set to no image
				if(ppShp) {
					*ppShp = nullptr;
				}
				pBuffer[0] = 0;
			} else if(!ppShp || strstr(pBuffer, ".pcx")) {
				// clear shp and load pcx
				if(ppShp) {
					*ppShp = nullptr;
				}
				if(!PCX::Instance->LoadFile(pBuffer)) {
					// log error and clear invalid name
					Debug::INIParseFailed(pID, key, pBuffer);
					pBuffer[0] = 0;
				}
			} else if(ppShp) {
				// allowed to load as shp
				*ppShp = FileSystem::LoadSHPFile(pBuffer);
				if(!*ppShp) {
					// log error and clear invalid name
					Debug::INIParseFailed(pID, key, pBuffer);
					pBuffer[0] = 0;
				}
			} else {
				// disallowed file type
				Debug::INIParseFailed(pID, key, pBuffer, "File type not allowed.");
			}
		}
	};

	ReadShpOrPcxImage("File.Flag", this->FlagFile, 0x20, nullptr);
	ReadShpOrPcxImage("File.ObserverFlag", this->ObserverFlag, 0x20, &this->ObserverFlagSHP);
	ReadShpOrPcxImage("File.ObserverBackground", this->ObserverBackground, 0x20, &this->ObserverBackgroundSHP);

	if (pINI->ReadString(pID, "File.LoadScreen", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->LSFile, Ares::readBuffer);
	}

	if (pINI->ReadString(pID, "File.LoadScreenPAL", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->LSPALFile, Ares::readBuffer);
	}

	if (pINI->ReadString(pID, "File.Taunt", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->TauntFile, Ares::readBuffer);
	}

	if (pINI->ReadString(pID, "LoadScreenText.Name", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->LSName, Ares::readBuffer);
	}

	if (pINI->ReadString(pID, "LoadScreenText.SpecialName", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->LSSpecialName, Ares::readBuffer);
	}

	if (pINI->ReadString(pID, "LoadScreenText.Brief", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->LSBrief, Ares::readBuffer);
	}

	if (pINI->ReadString(pID, "MenuText.Status", "", Ares::readBuffer, Ares::readLength)) {
		AresCRT::strCopy(this->StatusText, Ares::readBuffer);
	}

	INI_EX exINI(pINI);
	this->ObserverFlagYuriPAL.Read(exINI, pID, "File.ObserverFlagAltPalette");
}

void HouseTypeExt::ExtData::LoadFromINIFile(HouseTypeClass *pThis, CCINIClass *pINI) {
	char* pID = pThis->ID;

	if(!this->SettingsInherited && *pThis->ParentCountry && _strcmpi(pThis->ParentCountry, pThis->ID)) {
		this->InheritSettings(pThis);
	}

	INI_EX exINI(pINI);

	this->Powerplants.Read(exINI, pID, "AI.PowerPlants");

	this->Parachute_Anim.Read(exINI, pID, "Parachute.Anim");
	
	this->ParaDropPlane.Read(exINI, pID, "ParaDrop.Aircraft");

	this->ParaDropTypes.Read(exINI, pID, "ParaDrop.Types");

	// remove all types that aren't either infantry or unit types
	this->ParaDropTypes.erase(std::remove_if(this->ParaDropTypes.begin(), this->ParaDropTypes.end(), [pID](TechnoTypeClass* pItem) -> bool {
		auto abs = pItem->WhatAmI();
		if(abs == InfantryTypeClass::AbsID || abs == UnitTypeClass::AbsID) {
			return false;
		}

		Debug::INIParseFailed(pID, "ParaDrop.Types", pItem->ID, "Only InfantryTypes and UnitTypes are supported.");
		return true;
	}), this->ParaDropTypes.end());

	this->ParaDropNum.Read(exINI, pID, "ParaDrop.Num");

	this->LoadTextColor.Read(exINI, pID, "LoadScreenText.Color");

	this->RandomSelectionWeight = pINI->ReadInteger(pID, "RandomSelectionWeight", this->RandomSelectionWeight);
	this->CountryListIndex = pINI->ReadInteger(pID, "ListIndex", this->CountryListIndex);

	this->VeteranBuildings.Read(exINI, pID, "VeteranBuildings");
}

template<size_t Len>
void CopyString(char (HouseTypeExt::ExtData::* prop)[Len], const HouseTypeExt::ExtData *src, HouseTypeExt::ExtData *dst) {
	AresCRT::strCopy(dst->*prop, src->*prop);
}

template<typename T>
void CopyVector(T HouseTypeExt::ExtData::* prop, const HouseTypeExt::ExtData *src, HouseTypeExt::ExtData *dst) {
	auto &sp = src->*prop;
	auto &dp = dst->*prop;
	dp.SetCapacity(sp.Capacity, nullptr);
	dp.Count = sp.Count;
	for(unsigned ix = sp.Count; ix > 0; --ix) {
		auto idx = ix - 1;
		dp.Items[idx] = sp.Items[idx];
	}
}

template<typename T>
void CopyStdVector(T HouseTypeExt::ExtData::* prop, const HouseTypeExt::ExtData *src, HouseTypeExt::ExtData *dst) {
	auto &sp = src->*prop;
	auto &dp = dst->*prop;
	dp.clear();
	dp.reserve(sp.size());
	std::copy(sp.begin(), sp.end(), std::back_inserter(dp));
}

void HouseTypeExt::ExtData::InheritSettings(HouseTypeClass *pThis) {
	if(auto ParentCountry = HouseTypeClass::Find(pThis->ParentCountry)) {
		if(const auto ParentData = HouseTypeExt::ExtMap.Find(ParentCountry)) {
			CopyString(&HouseTypeExt::ExtData::FlagFile, ParentData, this);
			CopyString(&HouseTypeExt::ExtData::ObserverFlag, ParentData, this);
			CopyString(&HouseTypeExt::ExtData::ObserverBackground, ParentData, this);
			CopyString(&HouseTypeExt::ExtData::LSFile, ParentData, this);
			CopyString(&HouseTypeExt::ExtData::LSPALFile, ParentData, this);
			CopyString(&HouseTypeExt::ExtData::TauntFile, ParentData, this);
			CopyString(&HouseTypeExt::ExtData::LSName, ParentData, this);
			CopyString(&HouseTypeExt::ExtData::LSSpecialName, ParentData, this);
			CopyString(&HouseTypeExt::ExtData::LSBrief, ParentData, this);
			CopyString(&HouseTypeExt::ExtData::StatusText, ParentData, this);
			this->LoadTextColor = ParentData->LoadTextColor;
			this->RandomSelectionWeight = ParentData->RandomSelectionWeight;
			this->CountryListIndex = ParentData->CountryListIndex + 1;
			this->ObserverBackgroundSHP = ParentData->ObserverBackgroundSHP;
			this->ObserverFlagSHP = ParentData->ObserverFlagSHP;
			this->ObserverFlagYuriPAL = ParentData->ObserverFlagYuriPAL;

			this->ParaDropPlane.Set(ParentData->ParaDropPlane);
			this->Parachute_Anim.Set(ParentData->Parachute_Anim);

			this->Powerplants = ParentData->Powerplants;
			this->ParaDropTypes = ParentData->ParaDropTypes;
			this->ParaDropNum = ParentData->ParaDropNum;

			CopyStdVector(&HouseTypeExt::ExtData::VeteranBuildings, ParentData, this);
		}
	}
	this->SettingsInherited = true;
}

AnimTypeClass* HouseTypeExt::ExtData::GetParachuteAnim() {
	// country-specific parachute
	if(AnimTypeClass* pAnimType = this->Parachute_Anim) {
		return pAnimType;
	}

	// side-specific with fallback to rules
	int iSide = this->AttachedToObject->SideIndex;
	if(auto pSide = SideClass::Array->GetItemOrDefault(iSide)) {
		if(SideExt::ExtData *pData = SideExt::ExtMap.Find(pSide)) {
			if(AnimTypeClass* pAnimType = pData->GetParachuteAnim()) {
				return pAnimType;
			}
		} else {
			Debug::Log("[GetParachuteAnim] House %s and its side have no valid parachute defined. Rules fallback failed.\n", this->AttachedToObject->ID);
		}
	}
	
	// this should not happen for non-civilian sides
	return AnimTypeClass::Find("PARACH");
}

AircraftTypeClass* HouseTypeExt::ExtData::GetParadropPlane() {
	// tries to get the house's default plane and falls back to
	// the sides default plane.
	int iPlane = this->ParaDropPlane;

	int iSide = this->AttachedToObject->SideIndex;
	if((iPlane < 0) && (iSide >= 0)) {
		if(SideExt::ExtData *pData = SideExt::ExtMap.Find(SideClass::Array->GetItem(iSide))) {
			iPlane = pData->ParaDropPlane;
		}
	}

	// didn't help. default to the PDPlane like the game does.
	if(iPlane < 0) {
		iPlane = AircraftTypeClass::FindIndex("PDPLANE");
	}

	if(AircraftTypeClass::Array->ValidIndex(iPlane)) {
		return AircraftTypeClass::Array->GetItem(iPlane);
	} else {
		Debug::Log("[GetParadropPlane] House %s and its side have no valid paradrop plane defined. Rules fallback failed.\n", this->AttachedToObject->ID);
		return nullptr;
	}
}

bool HouseTypeExt::ExtData::GetParadropContent(Iterator<TechnoTypeClass*> &Types, Iterator<int> &Num) {
	// tries to get the house's default contents and falls back to
	// the sides default contents.
	if(this->ParaDropTypes.size()) {
		Types = this->ParaDropTypes;
		Num = this->ParaDropNum;
	}

	// fall back to side specific para drop
	if(!Types) {
		SideClass* pSide = SideClass::Array->GetItem(this->AttachedToObject->SideIndex);
		if(SideExt::ExtData *pData = SideExt::ExtMap.Find(pSide)) {
			Types = pData->GetParaDropTypes();
			Num = pData->GetParaDropNum();
		}
	}

	return (Types && Num);
}

Iterator<BuildingTypeClass*> HouseTypeExt::ExtData::GetPowerplants() const {
	if(this->Powerplants.size()) {
		return this->Powerplants;
	}

	return this->GetDefaultPowerplants();
}

Iterator<BuildingTypeClass*> HouseTypeExt::ExtData::GetDefaultPowerplants() const {
	BuildingTypeClass** ppPower = nullptr;
	switch(this->AttachedToObject->SideIndex) {
	case 0:
		ppPower = &RulesClass::Instance->GDIPowerPlant;
		break;
	case 1:
		ppPower = &RulesClass::Instance->NodRegularPower;
		break;
	case 2:
		ppPower = &RulesClass::Instance->ThirdPowerPlant;
		break;
	}

	int count = (ppPower && *ppPower) ? 1 : 0;
	return Iterator<BuildingTypeClass*>(ppPower, count);
}

int HouseTypeExt::PickRandomCountry() {
	DiscreteDistributionClass<int> items;

	for (int i = 0; i < HouseTypeClass::Array->Count; i++) {
		HouseTypeClass* pCountry = HouseTypeClass::Array->Items[i];
		if (pCountry->Multiplay) {
			if (HouseTypeExt::ExtData *pData = HouseTypeExt::ExtMap.Find(pCountry)) {
				items.Add(i, pData->RandomSelectionWeight);
			}
		}
	}

	int ret = 0;
	if(!items.Select(ScenarioClass::Instance->Random, &ret)) {
		Debug::FatalErrorAndExit("No countries eligible for random selection!");
	}

	return ret;
}

// =============================
// load/save

bool Container<HouseTypeExt>::Save(HouseTypeClass *pThis, IStream *pStm) {
	HouseTypeExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if (pData) {
		//pData->Powerplants.Save(pStm);
	}

	return pData != nullptr;
}

bool Container<HouseTypeExt>::Load(HouseTypeClass *pThis, IStream *pStm) {
	HouseTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	//pData->Powerplants.Load(pStm, 1);

	return pData != nullptr;
}

// =============================
// container hooks

DEFINE_HOOK(511635, HouseTypeClass_CTOR_1, 5) {
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(511643, HouseTypeClass_CTOR_2, 5) {
	GET(HouseTypeClass*, pItem, EAX);

	HouseTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(512760, HouseTypeClass_DTOR, 6) {
	GET(HouseTypeClass*, pItem, ECX);

	HouseTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(512480, HouseTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(512290, HouseTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(HouseTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<HouseTypeExt>::PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(51246D, HouseTypeClass_Load_Suffix, 5) {
	HouseTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(51255C, HouseTypeClass_Save_Suffix, 5) {
	HouseTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(51215A, HouseTypeClass_LoadFromINI, 5)
DEFINE_HOOK(51214F, HouseTypeClass_LoadFromINI, 5)
{
	GET(HouseTypeClass*, pItem, EBX);
	GET_BASE(CCINIClass*, pINI, 0x8);

	HouseTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}

