#include "Ares.h"
#include "Ares.CRT.h"
#include "Misc/Interface.h"
#include "Utilities/Constructs.h"

#include <StringTable.h>
#include <ColorScheme.h>
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

int Ares::UISettings::ColorCount = 8;
Interface::ColorData Ares::UISettings::Colors[maxColorCount+1];

int Ares::UISettings::uiColorText;
int Ares::UISettings::uiColorTextButton;
int Ares::UISettings::uiColorTextCheckbox;
int Ares::UISettings::uiColorTextRadio;
int Ares::UISettings::uiColorTextLabel;
int Ares::UISettings::uiColorTextList;
int Ares::UISettings::uiColorTextCombobox;
int Ares::UISettings::uiColorTextGroupbox;
int Ares::UISettings::uiColorTextSlider;
int Ares::UISettings::uiColorTextEdit;
int Ares::UISettings::uiColorCaret;
int Ares::UISettings::uiColorSelection;
int Ares::UISettings::uiColorSelectionCombobox;
int Ares::UISettings::uiColorSelectionList;
int Ares::UISettings::uiColorBorder1;
int Ares::UISettings::uiColorBorder2;
int Ares::UISettings::uiColorObserverSide;
int Ares::UISettings::uiColorObserverOpen;
int Ares::UISettings::uiColorDisabled;
int Ares::UISettings::uiColorDisabledLabel;
int Ares::UISettings::uiColorDisabledButton;
int Ares::UISettings::uiColorDisabledCombobox;
int Ares::UISettings::uiColorDisabledCheckbox;
int Ares::UISettings::uiColorDisabledSlider;
int Ares::UISettings::uiColorDisabledList;

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
		sprintf_s(buffer, 0x20, "%s.Image", name);
		if(pINI->ReadString(section, buffer, defImage, Ares::readBuffer, Ares::readLength)) {
			value->Image = FileSystem::LoadSHPFile(Ares::readBuffer);
		}

		// read the palette
		sprintf_s(buffer, 0x20, "%s.Palette", name);
		if(value->Palette) {
			delete value->Palette;
		}
		value->Palette = new CustomPalette();
		value->Palette->LoadFromINI(pINI, section, buffer, defPalette);

		// get the subline and tooltip
		sprintf_s(buffer, 0x20, "%s.Subline", name);
		if(pINI->ReadString(section, buffer, defSubline, Ares::readBuffer, Ares::readLength)) {
			AresCRT::strCopy(value->Subline, Ares::readBuffer, 0x1F);
		}

		sprintf_s(buffer, 0x20, "%s.Tooltip", name);
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

	// color settings. these are the selectable colors used in the user interface
	// and the mapping to link to the corresponding in-game color schemes.
	const char* section2 = "Colors";

	auto ParseColorInt = [&](const char* section, const char* key, int defColor) -> int {
		ColorStruct defCS;
		ColorStruct bufCS;
		defCS.R = defColor & 0xFF;
		defCS.G = (defColor >> 8) & 0xFF;
		defCS.B = (defColor >> 16) & 0xFF;
		if(pINI->ReadColor(&bufCS, section, key, &defCS)) {
			return bufCS.R | bufCS.G << 8 | bufCS.B << 16;
		}
		return defColor;
	};

	auto ReadColor = [&](const char* name, Interface::ColorData *value, int colorRGB, char* defTooltip, char* defColorScheme) {
		// load the tooltip string
		char buffer[0x20];
		sprintf_s(buffer, 0x20, "%s.Tooltip", name);
		if(pINI->ReadString(section2, buffer, defTooltip, Ares::readBuffer, Ares::readLength)) {
			value->sttToolTipSublineText = StringTable::LoadStringA(Ares::readBuffer);
		}

		sprintf_s(buffer, 0x20, "%s.ColorScheme", name);
		if(pINI->ReadString(section2, buffer, defColorScheme, Ares::readBuffer, Ares::readLength)) {
			AresCRT::strCopy(value->colorScheme, Ares::readBuffer, 0x20);
		}

		sprintf_s(buffer, 0x20, "%s.DisplayColor", name);
		value->colorRGB = ParseColorInt(section, buffer, colorRGB);

		value->colorSchemeIndex = -1;
		value->selectedIndex = -1;
	};

	int* defColors = (int*)0x8316A8;
	ColorCount = std::max(std::min(pINI->ReadInteger(section2, "Count", ColorCount), maxColorCount), 8);

	// original color schemes
	ReadColor("Observer", &Colors[0], defColors[8], "STT:PlayerColorObserver", "LightGrey");
	ReadColor("Slot1", &Colors[1], defColors[0], "STT:PlayerColorGold", "Gold");
	ReadColor("Slot2", &Colors[2], defColors[1], "STT:PlayerColorRed", "DarkRed");
	ReadColor("Slot3", &Colors[3], defColors[2], "STT:PlayerColorBlue", "DarkBlue");
	ReadColor("Slot4", &Colors[4], defColors[3], "STT:PlayerColorGreen", "DarkGreen");
	ReadColor("Slot5", &Colors[5], defColors[4], "STT:PlayerColorOrange", "Orange");
	ReadColor("Slot6", &Colors[6], defColors[5], "STT:PlayerColorSkyBlue", "DarkSky");
	ReadColor("Slot7", &Colors[7], defColors[6], "STT:PlayerColorPurple", "Purple");
	ReadColor("Slot8", &Colors[8], defColors[7], "STT:PlayerColorPink", "Magenta");

	// additional color schemes so just increasing Count will produce nice colors
	ReadColor("Slot9", &Colors[9], 0xEF5D94, "STT:PlayerColorLilac", "NeonBlue");
	ReadColor("Slot10", &Colors[10], 0xE7FF73, "STT:PlayerColorLightBlue", "LightBlue");
	ReadColor("Slot11", &Colors[11], 0x63EFFF, "STT:PlayerColorLime", "Yellow");
	ReadColor("Slot12", &Colors[12], 0x5AC308, "STT:PlayerColorTeal", "Green");
	ReadColor("Slot13", &Colors[13], 0x0055BD, "STT:PlayerColorBrown", "Red");
	ReadColor("Slot14", &Colors[14], 0x808080, "STT:PlayerColorCharcoal", "Grey");

	// blunt stuff
	char key[0x10];
	for(int i=15; i<=ColorCount; ++i) {
		sprintf_s(key, 0x10, "Slot%d", i);
		ReadColor(key, &Colors[i], 0xFFFFFF, "NOSTR:", "LightGrey");
	}

	// menu colors. the color of labels, button texts, list items, stuff and others
	uiColorText = ParseColorInt(section, "Color.Text", 0xFFFF);
	uiColorTextButton = ParseColorInt(section, "Color.Button.Text", uiColorText);
	uiColorTextRadio = ParseColorInt(section, "Color.Radio.Text", uiColorText);
	uiColorTextCheckbox = ParseColorInt(section, "Color.Checkbox.Text", uiColorText);
	uiColorTextLabel = ParseColorInt(section, "Color.Label.Text", uiColorText);
	uiColorTextList = ParseColorInt(section, "Color.List.Text", uiColorText);
	uiColorTextCombobox = ParseColorInt(section, "Color.Combobox.Text", uiColorText);
	uiColorTextGroupbox = ParseColorInt(section, "Color.Groupbox.Text", uiColorText);
	uiColorTextSlider = ParseColorInt(section, "Color.Slider.Text", uiColorText);
	uiColorTextEdit = ParseColorInt(section, "Color.Edit.Text", uiColorText);
	uiColorCaret = ParseColorInt(section, "Color.Caret", 0xFFFF);
	uiColorSelection = ParseColorInt(section, "Color.Selection", 0xFF);
	uiColorSelectionCombobox = ParseColorInt(section, "Color.Combobox.Selection", uiColorSelection);
	uiColorSelectionList = ParseColorInt(section, "Color.List.Selection", uiColorSelection);
	uiColorBorder1 = ParseColorInt(section, "Color.Border1", 0xC5BEA7);
	uiColorBorder2 = ParseColorInt(section, "Color.Border2", 0x807A68);
	uiColorObserverSide = ParseColorInt(section, "Color.ObserverSide", 0x8F8F8F);
	uiColorObserverOpen = ParseColorInt(section, "Color.ObserverOpen", 0xEEEEEE);
	uiColorDisabled = ParseColorInt(section, "Color.Disabled", 0x9F);
	uiColorDisabledLabel = ParseColorInt(section, "Color.Label.Disabled", uiColorDisabled);
	uiColorDisabledCombobox = ParseColorInt(section, "Color.Combobox.Disabled", uiColorDisabled);
	uiColorDisabledSlider = ParseColorInt(section, "Color.Slider.Disabled", uiColorDisabled);
	uiColorDisabledButton = ParseColorInt(section, "Color.Button.Disabled", 0xA7);
	uiColorDisabledCheckbox = ParseColorInt(section, "Color.Checkbox.Disabled", uiColorDisabled);
	uiColorDisabledList = ParseColorInt(section, "Color.List.Disabled", uiColorDisabled);

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
	Debug::Log("Color count is %d\n", Ares::UISettings::ColorCount);

	// clean up
	Ares::CloseConfig(&pINI);
	return 0;
}
