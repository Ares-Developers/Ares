#include "Ares.h"

bool Ares::UISettings::Initialized = false;
bool Ares::UISettings::AllowMultiEngineer = false;

void Ares::UISettings::Load(CCINIClass *pINI) {
	if(pINI == NULL) {
		return;
	}

	const char* section = "UISettings";

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