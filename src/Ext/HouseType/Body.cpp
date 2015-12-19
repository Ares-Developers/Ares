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
HouseTypeExt::ExtContainer HouseTypeExt::ExtMap;

void HouseTypeExt::ExtData::InitializeConstants() {
	const char* pID = this->OwnerObject()->ID;

	//We assign default values by country ID rather than index so you simply add a new country
	//without having to specify all the tags for the old ones

	static char const* const Countries[] = {
		"Americans", "Alliance", "French", "Germans", "British", "Africans",
		"Arabs", "Confederation", "Russians", "YuriCountry" };
	auto const it = std::find_if(std::begin(Countries), std::end(Countries),
		[=](const char* pCountry) { return !_strcmpi(pID, pCountry); });
	auto const index = std::distance(std::begin(Countries), it);

	const char* pFlagFile = nullptr;
	const char* pLoadScreenBrief = nullptr;
	const char* pLoadScreenBackground = nullptr;
	const char* pLoadScreenName = nullptr;
	const char* pLoadScreenPalette = nullptr;
	const char* pLoadScreenSpecialName = nullptr;
	const char* pStatusText = nullptr;
	const char* pTauntFile = nullptr;
	const char* pObserverBackgroundSHP = nullptr;
	const char* pObserverFlagSHP = nullptr;
	bool bObserverFlagYuriPAL = false;

	switch(index) {
	case 0: // USA
		pFlagFile = "usai.pcx";
		pLoadScreenBrief = "LoadBrief:USA";
		pLoadScreenBackground = "ls%sustates.shp";
		pLoadScreenName = "Name:Americans";
		pLoadScreenPalette = "mplsu.pal";
		pLoadScreenSpecialName = "Name:Para";
		pStatusText = "STT:PlayerSideAmerica";
		pTauntFile = "taunts\\tauam%02i.wav";
		pObserverBackgroundSHP = "obsalli.shp";
		pObserverFlagSHP = "usai.shp";
		break;
	case 1: //Korea	
		pFlagFile = "japi.pcx";
		pLoadScreenBrief = "LoadBrief:Korea";
		pLoadScreenBackground = "ls%skorea.shp";
		pLoadScreenName = "Name:Alliance";
		pLoadScreenPalette = "mplsk.pal";
		pLoadScreenSpecialName = "Name:BEAGLE";
		pStatusText = "STT:PlayerSideKorea";
		pTauntFile = "taunts\\tauko%02i.wav";
		pObserverBackgroundSHP = "obsalli.shp";
		pObserverFlagSHP = "japi.shp";
		break;
	case 2: //France
		pFlagFile = "frai.pcx";
		pLoadScreenBrief = "LoadBrief:French";
		pLoadScreenBackground = "ls%sfrance.shp";
		pLoadScreenName = "Name:French";
		pLoadScreenPalette = "mplsf.pal";
		pLoadScreenSpecialName = "Name:GTGCAN";
		pStatusText = "STT:PlayerSideFrance";
		pTauntFile = "taunts\\taufr%02i.wav";
		pObserverBackgroundSHP = "obsalli.shp";
		pObserverFlagSHP = "frai.shp";
		break;
	case 3: //Germany
		pFlagFile = "geri.pcx";
		pLoadScreenBrief = "LoadBrief:Germans";
		pLoadScreenBackground = "ls%sgermany.shp";
		pLoadScreenName = "Name:Germans";
		pLoadScreenPalette = "mplsg.pal";
		pLoadScreenSpecialName = "Name:TNKD";
		pStatusText = "STT:PlayerSideGermany";
		pTauntFile = "taunts\\tauge%02i.wav";
		pObserverBackgroundSHP = "obsalli.shp";
		pObserverFlagSHP = "geri.shp";
		break;
	case 4: //United Kingdom
		pFlagFile = "gbri.pcx";
		pLoadScreenBrief = "LoadBrief:British";
		pLoadScreenBackground = "ls%sukingdom.shp";
		pLoadScreenName = "Name:British";
		pLoadScreenPalette = "mplsuk.pal";
		pLoadScreenSpecialName = "Name:SNIPE";
		pStatusText = "STT:PlayerSideBritain";
		pTauntFile = "taunts\\taubr%02i.wav";
		pObserverBackgroundSHP = "obsalli.shp";
		pObserverFlagSHP = "gbri.shp";
		break;
	case 5: //Libya
		pFlagFile = "djbi.pcx";
		pLoadScreenBrief = "LoadBrief:Lybia";
		pLoadScreenBackground = "ls%slibya.shp";
		pLoadScreenName = "Name:Africans";
		pLoadScreenPalette = "mplsl.pal";
		pLoadScreenSpecialName = "Name:DTRUCK";
		pStatusText = "STT:PlayerSideLibya";
		pTauntFile = "taunts\\tauli%02i.wav";
		pObserverBackgroundSHP = "obssovi.shp";
		pObserverFlagSHP = "djbi.shp";
		break;
	case 6: //Iraq
		pFlagFile = "arbi.pcx";
		pLoadScreenBrief = "LoadBrief:Iraq";
		pLoadScreenBackground = "ls%siraq.shp";
		pLoadScreenName = "Name:Arabs";
		pLoadScreenPalette = "mplsi.pal";
		pLoadScreenSpecialName = "Name:DESO";
		pStatusText = "STT:PlayerSideIraq";
		pTauntFile = "taunts\\tauir%02i.wav";
		pObserverBackgroundSHP = "obssovi.shp";
		pObserverFlagSHP = "arbi.shp";
		break;
	case 7: //Cuba
		pFlagFile = "lati.pcx";
		pLoadScreenBrief = "LoadBrief:Cuba";
		pLoadScreenBackground = "ls%scuba.shp";
		pLoadScreenName = "Name:Confederation";
		pLoadScreenPalette = "mplsc.pal";
		pLoadScreenSpecialName = "Name:TERROR";
		pStatusText = "STT:PlayerSideCuba";
		pTauntFile = "taunts\\taucu%02i.wav";
		pObserverBackgroundSHP = "obssovi.shp";
		pObserverFlagSHP = "lati.shp";
		break;
	case 8: //Russia
		pFlagFile = "rusi.pcx";
		pLoadScreenBrief = "LoadBrief:Russia";
		pLoadScreenBackground = "ls%srussia.shp";
		pLoadScreenName = "Name:Russians";
		pLoadScreenPalette = "mplsr.pal";
		pLoadScreenSpecialName = "Name:TTNK";
		pStatusText = "STT:PlayerSideRussia";
		pTauntFile = "taunts\\tauru%02i.wav";
		pObserverBackgroundSHP = "obssovi.shp";
		pObserverFlagSHP = "rusi.shp";
		break;
	case 9: //Yuri
		pFlagFile = "yrii.pcx";
		pLoadScreenBrief = "LoadBrief:YuriCountry";
		pLoadScreenBackground = "ls%syuri.shp";
		pLoadScreenName = "Name:YuriCountry";
		pLoadScreenPalette = "mpyls.pal";
		pLoadScreenSpecialName = "Name:YURI";
		pStatusText = "STT:PlayerSideYuriCountry";
		pTauntFile = "taunts\\tauyu%02i.wav";
		pObserverBackgroundSHP = "obsyuri.shp";
		pObserverFlagSHP = "yrii.shp";
		bObserverFlagYuriPAL = true;
		break;
	default: //Unknown
		pFlagFile = "rani.pcx";
		pLoadScreenBrief = "GUI:Unknown";
		pLoadScreenBackground = "ls%sobs.shp";
		pLoadScreenName = "GUI:Unknown";
		pLoadScreenPalette = "mplsobs.pal";
		pLoadScreenSpecialName = "GUI:Unknown";
		pStatusText = "GUI:Unknown";
		pTauntFile = "taunts\\tauam%02i.wav";
	}

	// apply the defaults
	this->FlagFile = pFlagFile;
	this->LoadScreenBrief = pLoadScreenBrief;
	this->LoadScreenBackground = pLoadScreenBackground;
	this->LoadScreenName = pLoadScreenName;
	this->LoadScreenPalette = pLoadScreenPalette;
	this->LoadScreenSpecialName = pLoadScreenSpecialName;
	this->StatusText = pStatusText;
	this->TauntFile = pTauntFile;

	if(pObserverBackgroundSHP) {
		this->ObserverBackgroundSHP = FileSystem::LoadSHPFile(pObserverBackgroundSHP);
	}

	if(pObserverFlagSHP) {
		this->ObserverFlagSHP = FileSystem::LoadSHPFile(pObserverFlagSHP);
	}

	this->ObserverFlagYuriPAL = bObserverFlagYuriPAL;
}

void HouseTypeExt::ExtData::Initialize() {
	switch (this->OwnerObject()->SideIndex) {
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

void HouseTypeExt::ExtData::LoadFromRulesFile(CCINIClass *pINI) {
	const char* pID = this->OwnerObject()->ID;

	INI_EX exINI(pINI);

	// ppShp is optional. if not set, only PCX is supported
	auto ReadShpOrPcxImage = [pINI, pID](const char* key, AresPCXFile& Pcx, SHPStruct** ppShp) {
		// read the key and convert it to lower case
		if(pINI->ReadString(pID, key, Pcx.GetFilename(), Ares::readBuffer)) {

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

void HouseTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI) {
	auto pThis = this->OwnerObject();
	const char* pID = pThis->ID;

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

	this->RandomSelectionWeight.Read(exINI, pID, "RandomSelectionWeight");
	this->CountryListIndex.Read(exINI, pID, "ListIndex");

	this->VeteranBuildings.Read(exINI, pID, "VeteranBuildings");

	this->Degrades.Read(exINI, pID, "Degrades");
}

void HouseTypeExt::ExtData::InheritSettings(HouseTypeClass *pThis) {
	if(auto ParentCountry = pThis->FindParentCountry()) {
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

			this->ParaDropPlane = ParentData->ParaDropPlane;
			this->Parachute_Anim = ParentData->Parachute_Anim;

			this->Powerplants = ParentData->Powerplants;
			this->ParaDropTypes = ParentData->ParaDropTypes;
			this->ParaDropNum = ParentData->ParaDropNum;

			this->VeteranBuildings = ParentData->VeteranBuildings;
			this->Degrades = ParentData->Degrades;
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
	int iSide = this->OwnerObject()->SideIndex;
	if(auto pSide = SideClass::Array->GetItemOrDefault(iSide)) {
		if(SideExt::ExtData *pData = SideExt::ExtMap.Find(pSide)) {
			if(AnimTypeClass* pAnimType = pData->GetParachuteAnim()) {
				return pAnimType;
			}
		} else {
			Debug::Log("[GetParachuteAnim] House %s and its side have no valid parachute defined. Rules fallback failed.\n", this->OwnerObject()->ID);
		}
	}
	
	// this should not happen for non-civilian sides
	return AnimTypeClass::Find("PARACH");
}

AircraftTypeClass* HouseTypeExt::ExtData::GetParadropPlane() {
	// tries to get the house's default plane and falls back to
	// the sides default plane.
	int iPlane = this->ParaDropPlane;

	int iSide = this->OwnerObject()->SideIndex;
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
		Debug::Log("[GetParadropPlane] House %s and its side have no valid paradrop plane defined. Rules fallback failed.\n", this->OwnerObject()->ID);
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
		SideClass* pSide = SideClass::Array->GetItem(this->OwnerObject()->SideIndex);
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
	switch(this->OwnerObject()->SideIndex) {
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
// load / save

template <typename T>
void HouseTypeExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->FlagFile)
		.Process(this->LoadScreenBackground)
		.Process(this->LoadScreenPalette)
		.Process(this->TauntFile)
		.Process(this->LoadScreenName)
		.Process(this->LoadScreenSpecialName)
		.Process(this->LoadScreenBrief)
		.Process(this->StatusText)
		.Process(this->LoadTextColor)
		.Process(this->RandomSelectionWeight)
		.Process(this->CountryListIndex)
		.Process(this->Powerplants)
		.Process(this->ParaDropTypes)
		.Process(this->ParaDropNum)
		.Process(this->ParaDropPlane)
		.Process(this->Parachute_Anim)
		.Process(this->VeteranBuildings)
		.Process(this->ObserverBackground)
		.Process(this->ObserverBackgroundSHP)
		.Process(this->ObserverFlag)
		.Process(this->ObserverFlagSHP)
		.Process(this->ObserverFlagYuriPAL)
		.Process(this->SettingsInherited)
		.Process(this->Degrades);
}

void HouseTypeExt::ExtData::LoadFromStream(AresStreamReader &Stm) {
	Extension<HouseTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void HouseTypeExt::ExtData::SaveToStream(AresStreamWriter &Stm) {
	Extension<HouseTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

HouseTypeExt::ExtContainer::ExtContainer() : Container("HouseTypeClass") {
}

HouseTypeExt::ExtContainer::~ExtContainer() = default;

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
	GET_STACK(HouseTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	HouseTypeExt::ExtMap.PrepareStream(pItem, pStm);

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
