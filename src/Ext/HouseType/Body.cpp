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

void HouseTypeExt::ExtData::InitializeConstants() {
	const char* pID = this->OwnerObject()->ID;

	//We assign default values by country ID rather than index so you simply add a new country
	//without having to specify all the tags for the old ones

	if (!_strcmpi(pID, "Americans")) //USA
	{
		this->FlagFile = "usai.pcx";
		this->LoadScreenBrief = CSFText("LoadBrief:USA");
		this->LoadScreenBackground = "ls%sustates.shp";
		this->LoadScreenName = CSFText("Name:Americans");
		this->LoadScreenPalette = "mplsu.pal";
		this->LoadScreenSpecialName = CSFText("Name:Para");
		this->StatusText = CSFText("STT:PlayerSideAmerica");
		this->TauntFile = "taunts\\tauam%02i.wav";
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsalli.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("usai.shp");
	} else if (!_strcmpi(pID, "Alliance")) //Korea
	{
		this->FlagFile = "japi.pcx";
		this->LoadScreenBrief = CSFText("LoadBrief:Korea");
		this->LoadScreenBackground = "ls%skorea.shp";
		this->LoadScreenName = CSFText("Name:Alliance");
		this->LoadScreenPalette = "mplsk.pal";
		this->LoadScreenSpecialName = CSFText("Name:BEAGLE");
		this->StatusText = CSFText("STT:PlayerSideKorea");
		this->TauntFile = "taunts\\tauko%02i.wav";
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsalli.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("japi.shp");
	} else if (!_strcmpi(pID, "French")) //France
	{
		this->FlagFile = "frai.pcx";
		this->LoadScreenBrief = CSFText("LoadBrief:French");
		this->LoadScreenBackground = "ls%sfrance.shp";
		this->LoadScreenName = CSFText("Name:French");
		this->LoadScreenPalette = "mplsf.pal";
		this->LoadScreenSpecialName = CSFText("Name:GTGCAN");
		this->StatusText = CSFText("STT:PlayerSideFrance");
		this->TauntFile = "taunts\\taufr%02i.wav";
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsalli.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("frai.shp");
	} else if (!_strcmpi(pID, "Germans")) //Germany
	{
		this->FlagFile = "geri.pcx";
		this->LoadScreenBrief = CSFText("LoadBrief:Germans");
		this->LoadScreenBackground = "ls%sgermany.shp";
		this->LoadScreenName = CSFText("Name:Germans");
		this->LoadScreenPalette = "mplsg.pal";
		this->LoadScreenSpecialName = CSFText("Name:TNKD");
		this->StatusText = CSFText("STT:PlayerSideGermany");
		this->TauntFile = "taunts\\tauge%02i.wav";
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsalli.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("geri.shp");
	} else if (!_strcmpi(pID, "British")) //United Kingdom
	{
		this->FlagFile = "gbri.pcx";
		this->LoadScreenBrief = CSFText("LoadBrief:British");
		this->LoadScreenBackground = "ls%sukingdom.shp";
		this->LoadScreenName = CSFText("Name:British");
		this->LoadScreenPalette = "mplsuk.pal";
		this->LoadScreenSpecialName = CSFText("Name:SNIPE");
		this->StatusText = CSFText("STT:PlayerSideBritain");
		this->TauntFile = "taunts\\taubr%02i.wav";
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsalli.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("gbri.shp");
	} else if (!_strcmpi(pID, "Africans")) //Libya
	{
		this->FlagFile = "djbi.pcx";
		this->LoadScreenBrief = CSFText("LoadBrief:Lybia");
		this->LoadScreenBackground = "ls%slibya.shp";
		this->LoadScreenName = CSFText("Name:Africans");
		this->LoadScreenPalette = "mplsl.pal";
		this->LoadScreenSpecialName = CSFText("Name:DTRUCK");
		this->StatusText = CSFText("STT:PlayerSideLibya");
		this->TauntFile = "taunts\\tauli%02i.wav";
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obssovi.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("djbi.shp");
	} else if (!_strcmpi(pID, "Arabs")) //Iraq
	{
		this->FlagFile = "arbi.pcx";
		this->LoadScreenBrief = CSFText("LoadBrief:Iraq");
		this->LoadScreenBackground = "ls%siraq.shp";
		this->LoadScreenName = CSFText("Name:Arabs");
		this->LoadScreenPalette = "mplsi.pal";
		this->LoadScreenSpecialName = CSFText("Name:DESO");
		this->StatusText = CSFText("STT:PlayerSideIraq");
		this->TauntFile = "taunts\\tauir%02i.wav";
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obssovi.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("arbi.shp");
	} else if (!_strcmpi(pID, "Confederation")) //Cuba
	{
		this->FlagFile = "lati.pcx";
		this->LoadScreenBrief = CSFText("LoadBrief:Cuba");
		this->LoadScreenBackground = "ls%scuba.shp";
		this->LoadScreenName = CSFText("Name:Confederation");
		this->LoadScreenPalette = "mplsc.pal";
		this->LoadScreenSpecialName = CSFText("Name:TERROR");
		this->StatusText = CSFText("STT:PlayerSideCuba");
		this->TauntFile = "taunts\\taucu%02i.wav";
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obssovi.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("lati.shp");
	} else if (!_strcmpi(pID, "Russians")) //Russia
	{
		this->FlagFile = "rusi.pcx";
		this->LoadScreenBrief = CSFText("LoadBrief:Russia");
		this->LoadScreenBackground = "ls%srussia.shp";
		this->LoadScreenName = CSFText("Name:Russians");
		this->LoadScreenPalette = "mplsr.pal";
		this->LoadScreenSpecialName = CSFText("Name:TTNK");
		this->StatusText = CSFText("STT:PlayerSideRussia");
		this->TauntFile = "taunts\\tauru%02i.wav";
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obssovi.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("rusi.shp");
	} else if (!_strcmpi(pID, "YuriCountry")) //Yuri
	{
		this->FlagFile = "yrii.pcx";
		this->LoadScreenBrief = CSFText("LoadBrief:YuriCountry");
		this->LoadScreenBackground = "ls%syuri.shp";
		this->LoadScreenName = CSFText("Name:YuriCountry");
		this->LoadScreenPalette = "mpyls.pal";
		this->LoadScreenSpecialName = CSFText("Name:YURI");
		this->StatusText = CSFText("STT:PlayerSideYuriCountry");
		this->TauntFile = "taunts\\tauyu%02i.wav";
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile("obsyuri.shp");
		this->ObserverFlagSHP = FileSystem::LoadSHPFile("yrii.shp");
		this->ObserverFlagYuriPAL = true;
	} else //Unknown
	{
		this->FlagFile = "rani.pcx";
		this->LoadScreenBrief = CSFText("GUI:Unknown");
		this->LoadScreenBackground = "ls%sobs.shp";
		this->LoadScreenName = CSFText("GUI:Unknown");
		this->LoadScreenPalette = "mplsobs.pal";
		this->LoadScreenSpecialName = CSFText("GUI:Unknown");
		this->StatusText = CSFText("GUI:Unknown");
		this->TauntFile = "taunts\\tauam%02i.wav";
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

	this->InitializeConstants();

	INI_EX exINI(pINI);

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
	this->TauntFile.Read(pINI, pID, "File.Taunt");

	this->LoadScreenName.Read(exINI, pID, "LoadScreenText.Name");
	this->LoadScreenSpecialName.Read(exINI, pID, "LoadScreenText.SpecialName");
	this->LoadScreenBrief.Read(exINI, pID, "LoadScreenText.Brief");
	this->StatusText.Read(exINI, pID, "MenuText.Status");

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

void HouseTypeExt::ExtData::InheritSettings(HouseTypeClass *pThis) {
	if(auto ParentCountry = HouseTypeClass::Find(pThis->ParentCountry)) {
		if(const auto ParentData = HouseTypeExt::ExtMap.Find(ParentCountry)) {
			this->LoadScreenBackground = ParentData->LoadScreenBackground;
			this->LoadScreenPalette = ParentData->LoadScreenPalette;
			this->TauntFile = ParentData->TauntFile;
			this->LoadScreenName = ParentData->LoadScreenName;
			this->LoadScreenSpecialName = ParentData->LoadScreenSpecialName;
			this->LoadScreenBrief = ParentData->LoadScreenBrief;
			this->StatusText = ParentData->StatusText;
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

			this->VeteranBuildings = ParentData->VeteranBuildings;
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

	unsigned int count = (ppPower && *ppPower) ? 1u : 0u;
	return Iterator<BuildingTypeClass*>(ppPower, count);
}

int HouseTypeExt::PickRandomCountry() {
	DiscreteDistributionClass<int> items;

	for (int i = 0; i < HouseTypeClass::Array->Count; i++) {
		HouseTypeClass* pCountry = HouseTypeClass::Array->Items[i];
		if (pCountry->Multiplay) {
			if (auto pData = HouseTypeExt::ExtMap.Find(pCountry)) {
				items.Add(i, static_cast<unsigned int>(pData->RandomSelectionWeight));
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
