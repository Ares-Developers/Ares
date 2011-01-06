#include "Ares.h"
#include "Ares.CRT.h"
#include "Misc/Interface.h"
#include "Utilities/Constructs.h"

#include <strsafe.h>

bool Ares::UISettings::Initialized = false;
Interface::eUIAction Ares::UISettings::SinglePlayerButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::WWOnlineButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::NetworkButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::MoviesAndCreditsButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::CampaignButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::SkirmishButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::SneakPeeksButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::PlayMoviesButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::ViewCreditsButton = Interface::uia_Default;
bool Ares::UISettings::AllowMultiEngineer = false;
bool Ares::UISettings::CampaignList = false;
bool Ares::UISettings::ShowDebugCampaigns = false;
Interface::CampaignData Ares::UISettings::Campaigns[4];

void Ares::UISettings::Load(CCINIClass *pINI) {
	if(pINI == NULL) {
		return;
	}

	const char* section = "UISettings";

	auto ReadUIAction = [&](const char* name, Interface::eUIAction &value) {
		if(pINI->ReadString(section, name, "default", Ares::readBuffer, Ares::readLength)) {
			value = Interface::parseUIAction(Ares::readBuffer, value);
		}
	};

	ReadUIAction("SinglePlayerButton", SinglePlayerButton);
	ReadUIAction("WWOnlineButton", WWOnlineButton);
	ReadUIAction("NetworkButton", NetworkButton);
	ReadUIAction("MoviesAndCreditsButton", MoviesAndCreditsButton);
	ReadUIAction("CampaignButton", CampaignButton);
	ReadUIAction("SkirmishButton", SkirmishButton);
	ReadUIAction("SneakPeeksButton", SneakPeeksButton);
	ReadUIAction("PlayMoviesButton", PlayMoviesButton);
	ReadUIAction("ViewCreditsButton", ViewCreditsButton);

	AllowMultiEngineer = pINI->ReadBool(section, "AllowMultiEngineer", AllowMultiEngineer);
	CampaignList = pINI->ReadBool(section, "CampaignList", CampaignList);
	ShowDebugCampaigns = pINI->ReadBool(section, "ShowDebugCampaigns", ShowDebugCampaigns);

	// read the campaigns that can be started from the default campaign selection menu
	auto ReadCampaign = [&](const char* name, Interface::CampaignData *value, char* defBattle, char* defImage, char* defPalette, char* defSubline) {
		if(pINI->ReadString(section, name, defBattle, Ares::readBuffer, Ares::readLength)) {
			AresCRT::strCopy(value->Battle, Ares::readBuffer, 0x18);
		}

		// load the image
		char buffer[0x20];
		StringCchPrintfA(buffer, 0x20, "%s.Image", name);
		if(pINI->ReadString(section, buffer, defImage, Ares::readBuffer, Ares::readLength)) {
			value->Image = FileSystem::LoadSHPFile(Ares::readBuffer);
		}

		// read the palette
		StringCchPrintfA(buffer, 0x20, "%s.Palette", name);
		if(value->Palette) {
			delete value->Palette;
		}
		value->Palette = new CustomPalette();
		value->Palette->LoadFromINI(pINI, section, buffer, defPalette);

		// get the subline and tooltip
		StringCchPrintfA(buffer, 0x20, "%s.Subline", name);
		if(pINI->ReadString(section, buffer, defSubline, Ares::readBuffer, Ares::readLength)) {
			AresCRT::strCopy(value->Subline, Ares::readBuffer, 0x1F);
		}

		StringCchPrintfA(buffer, 0x20, "%s.Tooltip", name);
		if(pINI->ReadString(section, buffer, value->Subline, Ares::readBuffer, Ares::readLength)) {
			AresCRT::strCopy(value->ToolTip, Ares::readBuffer, 0x1F);
		}

		// is this a valid campaign?
		value->Valid = (*value->Battle != 0) && value->Image && _strcmpi(value->Battle, "no");
	};

	ReadCampaign("Campaign1", &Campaigns[0], "ALL1", "FSALG.SHP", "FSALG.PAL", "STT:AlliedCampaignIcon");
	ReadCampaign("Campaign2", &Campaigns[1], "SOV1", "FSSLG.SHP", "FSSLG.PAL", "STT:SovietCampaignIcon");
	ReadCampaign("Campaign3", &Campaigns[2], "TUT1", "", "FSBCLG.PAL", "STT:CampaignAnimTutorial");
	ReadCampaign("Campaign4", &Campaigns[3], "", "", "", "");

	Initialized = true;
}

DEFINE_HOOK(5FACDF, _Options_LoadFromINI, 5)
{
	// open the rules file
	Debug::Log("--------- Loading Ares global settings -----------\n");
	CCINIClass *pINI = Ares::OpenConfig("uimd.ini");
	
	// load and output settings
	Ares::UISettings::Load(pINI);
	Debug::Log("AllowMultiEngineer is %s\n", (Ares::UISettings::AllowMultiEngineer ? "ON" : "OFF"));
	Debug::Log("CampaignList is %s\n", (Ares::UISettings::CampaignList ? "ON" : "OFF"));
	Debug::Log("ShowDebugCampaigns is %s\n", (Ares::UISettings::ShowDebugCampaigns ? "ON" : "OFF"));

	// clean up
	Ares::CloseConfig(&pINI);
	return 0;
}
