#include "Ares.h"
#include "Misc/Interface.h"

bool Ares::UISettings::Initialized = false;
Interface::eUIAction Ares::UISettings::SinglePlayerButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::WWOnlineButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::NetworkButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::MoviesAndCreditsButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::CampaignButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::SkirmishButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::SneakPeaksButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::PlayMoviesButton = Interface::uia_Default;
Interface::eUIAction Ares::UISettings::ViewCreditsButton = Interface::uia_Default;
bool Ares::UISettings::AllowMultiEngineer = false;

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
	ReadUIAction("SneakPeaksButton", SneakPeaksButton);
	ReadUIAction("PlayMoviesButton", PlayMoviesButton);
	ReadUIAction("ViewCreditsButton", ViewCreditsButton);

	AllowMultiEngineer = pINI->ReadBool(section, "AllowMultiEngineer", AllowMultiEngineer);

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

	// clean up
	Ares::CloseConfig(&pINI);
	return 0;
}