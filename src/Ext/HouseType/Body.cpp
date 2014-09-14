#include "Body.h"
#include "../Side/Body.h"
#include "../../Ares.h"
#include "../../Ares.CRT.h"
#include "../../Utilities/Helpers.Alex.h"
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
		this->FlagFile = "usai.pcx";
		AresCRT::strCopy(this->LSBrief, "LoadBrief:USA");
		this->LoadScreenBackground = "ls%sustates.shp";
		AresCRT::strCopy(this->LSName, "Name:Americans");
		this->LoadScreenPalette = "mplsu.pal";
		AresCRT::strCopy(this->LSSpecialName, "Name:Para");
		AresCRT::strCopy(this->StatusText, "STT:PlayerSideAmerica");
		AresCRT::strCopy(this->TauntFile, "taunts\\tauam%02i.wav");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsalli.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("usai.shp");
	} else if (!_strcmpi(pID, "Alliance")) //Korea
	{
		this->FlagFile = "japi.pcx";
		AresCRT::strCopy(this->LSBrief, "LoadBrief:Korea");
		this->LoadScreenBackground = "ls%skorea.shp";
		AresCRT::strCopy(this->LSName, "Name:Alliance");
		this->LoadScreenPalette = "mplsk.pal";
		AresCRT::strCopy(this->LSSpecialName, "Name:BEAGLE");
		AresCRT::strCopy(this->StatusText, "STT:PlayerSideKorea");
		AresCRT::strCopy(this->TauntFile, "taunts\\tauko%02i.wav");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsalli.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("japi.shp");
	} else if (!_strcmpi(pID, "French")) //France
	{
		this->FlagFile = "frai.pcx";
		AresCRT::strCopy(this->LSBrief, "LoadBrief:French");
		this->LoadScreenBackground = "ls%sfrance.shp";
		AresCRT::strCopy(this->LSName, "Name:French");
		this->LoadScreenPalette = "mplsf.pal";
		AresCRT::strCopy(this->LSSpecialName, "Name:GTGCAN");
		AresCRT::strCopy(this->StatusText, "STT:PlayerSideFrance");
		AresCRT::strCopy(this->TauntFile, "taunts\\taufr%02i.wav");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsalli.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("frai.shp");
	} else if (!_strcmpi(pID, "Germans")) //Germany
	{
		this->FlagFile = "geri.pcx";
		AresCRT::strCopy(this->LSBrief, "LoadBrief:Germans");
		this->LoadScreenBackground = "ls%sgermany.shp";
		AresCRT::strCopy(this->LSName, "Name:Germans");
		this->LoadScreenPalette = "mplsg.pal";
		AresCRT::strCopy(this->LSSpecialName, "Name:TNKD");
		AresCRT::strCopy(this->StatusText, "STT:PlayerSideGermany");
		AresCRT::strCopy(this->TauntFile, "taunts\\tauge%02i.wav");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsalli.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("geri.shp");
	} else if (!_strcmpi(pID, "British")) //United Kingdom
	{
		this->FlagFile = "gbri.pcx";
		AresCRT::strCopy(this->LSBrief, "LoadBrief:British");
		this->LoadScreenBackground = "ls%sukingdom.shp";
		AresCRT::strCopy(this->LSName, "Name:British");
		this->LoadScreenPalette = "mplsuk.pal";
		AresCRT::strCopy(this->LSSpecialName, "Name:SNIPE");
		AresCRT::strCopy(this->StatusText, "STT:PlayerSideBritain");
		AresCRT::strCopy(this->TauntFile, "taunts\\taubr%02i.wav");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsalli.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("gbri.shp");
	} else if (!_strcmpi(pID, "Africans")) //Libya
	{
		this->FlagFile = "djbi.pcx";
		AresCRT::strCopy(this->LSBrief, "LoadBrief:Lybia");
		this->LoadScreenBackground = "ls%slibya.shp";
		AresCRT::strCopy(this->LSName, "Name:Africans");
		this->LoadScreenPalette = "mplsl.pal";
		AresCRT::strCopy(this->LSSpecialName, "Name:DTRUCK");
		AresCRT::strCopy(this->StatusText, "STT:PlayerSideLibya");
		AresCRT::strCopy(this->TauntFile, "taunts\\tauli%02i.wav");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obssovi.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("djbi.shp");
	} else if (!_strcmpi(pID, "Arabs")) //Iraq
	{
		this->FlagFile = "arbi.pcx";
		AresCRT::strCopy(this->LSBrief, "LoadBrief:Iraq");
		this->LoadScreenBackground = "ls%siraq.shp";
		AresCRT::strCopy(this->LSName, "Name:Arabs");
		this->LoadScreenPalette = "mplsi.pal";
		AresCRT::strCopy(this->LSSpecialName, "Name:DESO");
		AresCRT::strCopy(this->StatusText, "STT:PlayerSideIraq");
		AresCRT::strCopy(this->TauntFile, "taunts\\tauir%02i.wav");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obssovi.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("arbi.shp");
	} else if (!_strcmpi(pID, "Confederation")) //Cuba
	{
		this->FlagFile = "lati.pcx";
		AresCRT::strCopy(this->LSBrief, "LoadBrief:Cuba");
		this->LoadScreenBackground = "ls%scuba.shp";
		AresCRT::strCopy(this->LSName, "Name:Confederation");
		this->LoadScreenPalette = "mplsc.pal";
		AresCRT::strCopy(this->LSSpecialName, "Name:TERROR");
		AresCRT::strCopy(this->StatusText, "STT:PlayerSideCuba");
		AresCRT::strCopy(this->TauntFile, "taunts\\taucu%02i.wav");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obssovi.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("lati.shp");
	} else if (!_strcmpi(pID, "Russians")) //Russia
	{
		this->FlagFile = "rusi.pcx";
		AresCRT::strCopy(this->LSBrief, "LoadBrief:Russia");
		this->LoadScreenBackground = "ls%srussia.shp";
		AresCRT::strCopy(this->LSName, "Name:Russians");
		this->LoadScreenPalette = "mplsr.pal";
		AresCRT::strCopy(this->LSSpecialName, "Name:TTNK");
		AresCRT::strCopy(this->StatusText, "STT:PlayerSideRussia");
		AresCRT::strCopy(this->TauntFile, "taunts\\tauru%02i.wav");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obssovi.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("rusi.shp");
	} else if (!_strcmpi(pID, "YuriCountry")) //Yuri
	{
		this->FlagFile = "yrii.pcx";
		AresCRT::strCopy(this->LSBrief, "LoadBrief:YuriCountry");
		this->LoadScreenBackground = "ls%syuri.shp";
		AresCRT::strCopy(this->LSName, "Name:YuriCountry");
		this->LoadScreenPalette = "mpyls.pal";
		AresCRT::strCopy(this->LSSpecialName, "Name:YURI");
		AresCRT::strCopy(this->StatusText, "STT:PlayerSideYuriCountry");
		AresCRT::strCopy(this->TauntFile, "taunts\\tauyu%02i.wav");
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsyuri.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("yrii.shp");
		this->ObserverFlagYuriPAL = true;
	} else //Unknown
	{
		this->FlagFile = "rani.pcx";
		AresCRT::strCopy(this->LSBrief, "GUI:Unknown");
		this->LoadScreenBackground = "ls%sobs.shp";
		AresCRT::strCopy(this->LSName, "GUI:Unknown");
		this->LoadScreenPalette = "mplsobs.pal";
		AresCRT::strCopy(this->LSSpecialName, "GUI:Unknown");
		AresCRT::strCopy(this->StatusText, "GUI:Unknown");
		AresCRT::strCopy(this->TauntFile, "taunts\\tauam%02i.wav");
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
	auto ReadShpOrPcxImage = [&](const char* key, AresPCXFile& Pcx, SHPStruct** ppShp) {
		// read the key and convert it to lower case
		if(pINI->ReadString(pID, key, Pcx.GetFilename(), Ares::readBuffer, Ares::readLength)) {

			// parse the value
			if(INIClass::IsBlank(Ares::readBuffer)) {
				// explicitly set to no image
				if(ppShp) {
					*ppShp = nullptr;
				}
				Pcx = nullptr;
			} else if(!ppShp || strstr(Ares::readBuffer, ".pcx")) {
				// clear shp and load pcx
				if(ppShp) {
					*ppShp = nullptr;
				}
				Pcx = Ares::readBuffer;
				if(!Pcx.Exists()) {
					// log error and clear invalid name
					Debug::INIParseFailed(pID, key, Ares::readBuffer);
					Pcx = nullptr;
				}
			} else if(ppShp) {
				// allowed to load as shp
				*ppShp = FileSystem::LoadSHPFile(Ares::readBuffer);
				if(!*ppShp) {
					// log error and clear invalid name
					Debug::INIParseFailed(pID, key, Ares::readBuffer);
					Pcx = nullptr;
				}
			} else {
				// disallowed file type
				Debug::INIParseFailed(pID, key, Ares::readBuffer, "File type not allowed.");
			}
		}
	};

	ReadShpOrPcxImage("File.Flag", this->FlagFile, nullptr);
	ReadShpOrPcxImage("File.ObserverFlag", this->ObserverFlag, &this->ObserverFlagSHP);
	ReadShpOrPcxImage("File.ObserverBackground", this->ObserverBackground, &this->ObserverBackgroundSHP);

	this->LoadScreenBackground.Read(pINI, pID, "File.LoadScreen");
	this->LoadScreenPalette.Read(pINI, pID, "File.LoadScreenPAL");

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

	// remove all types that cannot paradrop
	Helpers::Alex::remove_non_paradroppables(this->ParaDropTypes, pID, "ParaDrop.Types");

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
			this->LoadScreenBackground = ParentData->LoadScreenBackground;
			this->LoadScreenPalette = ParentData->LoadScreenPalette;
			CopyString(&HouseTypeExt::ExtData::TauntFile, ParentData, this);
			CopyString(&HouseTypeExt::ExtData::LSName, ParentData, this);
			CopyString(&HouseTypeExt::ExtData::LSSpecialName, ParentData, this);
			CopyString(&HouseTypeExt::ExtData::LSBrief, ParentData, this);
			CopyString(&HouseTypeExt::ExtData::StatusText, ParentData, this);
			this->LoadTextColor = ParentData->LoadTextColor;
			this->RandomSelectionWeight = ParentData->RandomSelectionWeight;
			this->CountryListIndex = ParentData->CountryListIndex + 1;
			this->FlagFile = ParentData->FlagFile;
			this->ObserverBackground = ParentData->ObserverBackground;
			this->ObserverBackgroundSHP = ParentData->ObserverBackgroundSHP;
			this->ObserverFlag = ParentData->ObserverFlag;
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

DEFINE_HOOK(5127CF, HouseTypeClass_DTOR, 6) {
	GET(HouseTypeClass*, pItem, ESI);

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
