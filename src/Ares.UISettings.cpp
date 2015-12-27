#include "Ares.h"
#include "Ares.CRT.h"
#include "Misc/Interface.h"
#include "Utilities/Constructs.h"

#include <FileSystem.h>
#include <Checksummer.h>
#include <ColorScheme.h>
#include <StringTable.h>

#include <strsafe.h>

#include <algorithm>

bool Ares::UISettings::Initialized = false;
Ares::UISettings::UIAction Ares::UISettings::SinglePlayerButton = Ares::UISettings::UIAction::Default;
Ares::UISettings::UIAction Ares::UISettings::WWOnlineButton = Ares::UISettings::UIAction::Default;
Ares::UISettings::UIAction Ares::UISettings::NetworkButton = Ares::UISettings::UIAction::Default;
Ares::UISettings::UIAction Ares::UISettings::MoviesAndCreditsButton = Ares::UISettings::UIAction::Default;
Ares::UISettings::UIAction Ares::UISettings::CampaignButton = Ares::UISettings::UIAction::Default;
Ares::UISettings::UIAction Ares::UISettings::SkirmishButton = Ares::UISettings::UIAction::Default;
Ares::UISettings::UIAction Ares::UISettings::SneakPeeksButton = Ares::UISettings::UIAction::Default;
Ares::UISettings::UIAction Ares::UISettings::PlayMoviesButton = Ares::UISettings::UIAction::Default;
Ares::UISettings::UIAction Ares::UISettings::ViewCreditsButton = Ares::UISettings::UIAction::Default;
bool Ares::UISettings::QuickExit = false;
bool Ares::UISettings::AllowMultiEngineer = false;
bool Ares::UISettings::CampaignList = false;
bool Ares::UISettings::ShowDebugCampaigns = false;
bool Ares::UISettings::ShowSummary = true;
int Ares::UISettings::CampaignListSize = -1;
Ares::UISettings::CampaignData Ares::UISettings::Campaigns[4];

int Ares::UISettings::ColorCount = 8;
Ares::UISettings::ColorData Ares::UISettings::Colors[MaxColorCount + 1];

int Ares::UISettings::uiColorText;
int Ares::UISettings::uiColorTextButton = 0xFFFF; // #1644: needed for CD prompt
int Ares::UISettings::uiColorTextCheckbox;
int Ares::UISettings::uiColorTextRadio;
int Ares::UISettings::uiColorTextLabel = 0xFFFF; // #1644: needed for CD prompt
int Ares::UISettings::uiColorTextList;
int Ares::UISettings::uiColorTextCombobox;
int Ares::UISettings::uiColorTextGroupbox;
int Ares::UISettings::uiColorTextSlider;
int Ares::UISettings::uiColorTextEdit;
int Ares::UISettings::uiColorTextObserver;
int Ares::UISettings::uiColorCaret;
int Ares::UISettings::uiColorSelection;
int Ares::UISettings::uiColorSelectionCombobox;
int Ares::UISettings::uiColorSelectionList;
int Ares::UISettings::uiColorSelectionObserver;
int Ares::UISettings::uiColorBorder1;
int Ares::UISettings::uiColorBorder2;
int Ares::UISettings::uiColorDisabled;
int Ares::UISettings::uiColorDisabledLabel;
int Ares::UISettings::uiColorDisabledButton;
int Ares::UISettings::uiColorDisabledCombobox;
int Ares::UISettings::uiColorDisabledCheckbox;
int Ares::UISettings::uiColorDisabledSlider;
int Ares::UISettings::uiColorDisabledList;
int Ares::UISettings::uiColorDisabledObserver;

char Ares::UISettings::ModName[0x40] = "Yuri's Revenge";
char Ares::UISettings::ModVersion[0x40] = "1.001";
int Ares::UISettings::ModIdentifier = 0;

void Ares::UISettings::Load(CCINIClass *pINI) {
	if(pINI == nullptr) {
		return;
	}

	auto const section = "UISettings";

	auto const ReadUIAction = [pINI, section](const char* name, Ares::UISettings::UIAction &value) {
		if(pINI->ReadString(section, name, "default", Ares::readBuffer)) {
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

	QuickExit = pINI->ReadBool(section, "QuickExit", QuickExit);
	AllowMultiEngineer = pINI->ReadBool(section, "AllowMultiEngineer", AllowMultiEngineer);
	CampaignList = pINI->ReadBool(section, "CampaignList", CampaignList);
	ShowDebugCampaigns = pINI->ReadBool(section, "ShowDebugCampaigns", ShowDebugCampaigns);
	ShowSummary = pINI->ReadBool(section, "ShowSummary", ShowSummary);
	CampaignListSize = pINI->ReadInteger(section, "CampaignListSize", CampaignListSize);

	// read the campaigns that can be started from the default campaign selection menu
	auto const ReadCampaign = [pINI, section](const char* name, CampaignData& value,
		const char* defBattle, const char* defImage, const char* defPalette,
		const char* defSubline)
	{
		if(pINI->ReadString(section, name, defBattle, Ares::readBuffer)) {
			AresCRT::strCopy(value.Battle, Ares::readBuffer);
		}

		// load the image
		char buffer[0x20];
		sprintf_s(buffer, 0x20, "%s.Image", name);
		if(pINI->ReadString(section, buffer, defImage, Ares::readBuffer)) {
			value.Image = FileSystem::LoadSHPFile(Ares::readBuffer);
		}

		// read the palette
		sprintf_s(buffer, 0x20, "%s.Palette", name);
		if(value.Palette) {
			delete value.Palette;
		}
		value.Palette = new CustomPalette();
		value.Palette->LoadFromINI(pINI, section, buffer, defPalette);

		// get the subline and tooltip
		sprintf_s(buffer, 0x20, "%s.Subline", name);
		if(pINI->ReadString(section, buffer, defSubline, Ares::readBuffer)) {
			AresCRT::strCopy(value.Subline, Ares::readBuffer);
		}

		sprintf_s(buffer, 0x20, "%s.Tooltip", name);
		if(pINI->ReadString(section, buffer, value.Subline, Ares::readBuffer)) {
			AresCRT::strCopy(value.ToolTip, Ares::readBuffer);
		}

		// is this a valid campaign?
		value.Valid = (value.Battle[0] != 0) && value.Image && _strcmpi(value.Battle, "no");
	};

	ReadCampaign("Campaign1", Campaigns[0], "ALL1", "FSALG.SHP", "FSALG.PAL", "STT:AlliedCampaignIcon");
	ReadCampaign("Campaign2", Campaigns[1], "SOV1", "FSSLG.SHP", "FSSLG.PAL", "STT:SovietCampaignIcon");
	ReadCampaign("Campaign3", Campaigns[2], "TUT1", "", "FSBCLG.PAL", "STT:CampaignAnimTutorial");
	ReadCampaign("Campaign4", Campaigns[3], "", "", "", "");

	// color settings. these are the selectable colors used in the user interface
	// and the mapping to link to the corresponding in-game color schemes.
	auto const section2 = "Colors";

	auto const ParseColorInt = [pINI](const char* section, const char* key, int defColor) -> int {
		ColorStruct default(defColor & 0xFF, (defColor >> 8) & 0xFF, (defColor >> 16) & 0xFF);
		auto const color = pINI->ReadColor(section, key, default);
		return color.R | color.G << 8 | color.B << 16;
	};

	auto const ReadColor = [pINI, section2, ParseColorInt](const char* name, ColorData& value, int colorRGB, const char* defTooltip, const char* defColorScheme) {
		// load the tooltip string
		char buffer[0x20];
		sprintf_s(buffer, 0x20, "%s.Tooltip", name);
		if(pINI->ReadString(section2, buffer, defTooltip, Ares::readBuffer)) {
			value.sttToolTipSublineText = StringTable::LoadString(Ares::readBuffer);
		}

		sprintf_s(buffer, 0x20, "%s.ColorScheme", name);
		if(pINI->ReadString(section2, buffer, defColorScheme, Ares::readBuffer)) {
			AresCRT::strCopy(value.colorScheme, Ares::readBuffer);
		}

		sprintf_s(buffer, 0x20, "%s.DisplayColor", name);
		value.colorRGB = ParseColorInt(section2, buffer, colorRGB);

		value.colorSchemeIndex = -1;
		value.selectedIndex = -1;
	};

	ColorCount = Math::clamp(pINI->ReadInteger(section2, "Count", ColorCount), 8, MaxColorCount);

	// original color schemes
	auto const defColors = reinterpret_cast<int const*>(0x8316A8);
	ReadColor("Observer", Colors[0], defColors[8], "STT:PlayerColorObserver", "LightGrey");
	ReadColor("Slot1", Colors[1], defColors[0], "STT:PlayerColorGold", "Gold");
	ReadColor("Slot2", Colors[2], defColors[1], "STT:PlayerColorRed", "DarkRed");
	ReadColor("Slot3", Colors[3], defColors[2], "STT:PlayerColorBlue", "DarkBlue");
	ReadColor("Slot4", Colors[4], defColors[3], "STT:PlayerColorGreen", "DarkGreen");
	ReadColor("Slot5", Colors[5], defColors[4], "STT:PlayerColorOrange", "Orange");
	ReadColor("Slot6", Colors[6], defColors[5], "STT:PlayerColorSkyBlue", "DarkSky");
	ReadColor("Slot7", Colors[7], defColors[6], "STT:PlayerColorPurple", "Purple");
	ReadColor("Slot8", Colors[8], defColors[7], "STT:PlayerColorPink", "Magenta");

	// additional color schemes so just increasing Count will produce nice colors
	ReadColor("Slot9", Colors[9], 0xEF5D94, "STT:PlayerColorLilac", "NeonBlue");
	ReadColor("Slot10", Colors[10], 0xE7FF73, "STT:PlayerColorLightBlue", "LightBlue");
	ReadColor("Slot11", Colors[11], 0x63EFFF, "STT:PlayerColorLime", "Yellow");
	ReadColor("Slot12", Colors[12], 0x5AC308, "STT:PlayerColorTeal", "Green");
	ReadColor("Slot13", Colors[13], 0x0055BD, "STT:PlayerColorBrown", "Red");
	ReadColor("Slot14", Colors[14], 0x808080, "STT:PlayerColorCharcoal", "Grey");

	// blunt stuff
	char key[0x10];
	for(auto i = 15; i <= ColorCount; ++i) {
		sprintf_s(key, 0x10, "Slot%d", i);
		ReadColor(key, Colors[i], 0xFFFFFF, "NOSTR:", "LightGrey");
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
	uiColorTextObserver = ParseColorInt(section, "Color.Observer.Text", 0xEEEEEE);
	uiColorCaret = ParseColorInt(section, "Color.Caret", 0xFFFF);
	uiColorSelection = ParseColorInt(section, "Color.Selection", 0xFF);
	uiColorSelectionCombobox = ParseColorInt(section, "Color.Combobox.Selection", uiColorSelection);
	uiColorSelectionList = ParseColorInt(section, "Color.List.Selection", uiColorSelection);
	uiColorSelectionObserver = ParseColorInt(section, "Color.Observer.Selection", 0x626262);
	uiColorBorder1 = ParseColorInt(section, "Color.Border1", 0xC5BEA7);
	uiColorBorder2 = ParseColorInt(section, "Color.Border2", 0x807A68);
	uiColorDisabled = ParseColorInt(section, "Color.Disabled", 0x9F);
	uiColorDisabledLabel = ParseColorInt(section, "Color.Label.Disabled", uiColorDisabled);
	uiColorDisabledCombobox = ParseColorInt(section, "Color.Combobox.Disabled", uiColorDisabled);
	uiColorDisabledSlider = ParseColorInt(section, "Color.Slider.Disabled", uiColorDisabled);
	uiColorDisabledButton = ParseColorInt(section, "Color.Button.Disabled", 0xA7);
	uiColorDisabledCheckbox = ParseColorInt(section, "Color.Checkbox.Disabled", uiColorDisabled);
	uiColorDisabledList = ParseColorInt(section, "Color.List.Disabled", uiColorDisabled);
	uiColorDisabledObserver = ParseColorInt(section, "Color.Observer.Disabled", 0x8F8F8F);

	// read the mod's version info
	if(pINI->ReadString("VersionInfo", "Name", Ares::readDefval, Ares::readBuffer, std::size(ModName))) {
		AresCRT::strCopy(ModName, Ares::readBuffer);
	}
	if(pINI->ReadString("VersionInfo", "Version", Ares::readDefval, Ares::readBuffer, std::size(ModVersion))) {
		AresCRT::strCopy(ModVersion, Ares::readBuffer);
	}

	SafeChecksummer crc;
	crc.Add(ModName);
	crc.Commit();
	crc.Add(ModVersion);
	ModIdentifier = pINI->ReadInteger("VersionInfo", "Identifier", static_cast<int>(crc.GetValue()));

	Initialized = true;
}

DEFINE_HOOK(5FACDF, _Options_LoadFromINI, 5)
{
	// open the rules file
	Debug::Log("--------- Loading Ares global settings -----------\n");
	CCINIClass *pINI = Ares::OpenConfig("uimd.ini");
	
	// load and output settings
	Ares::UISettings::Load(pINI);
	Debug::Log("QuickExit is %s\n", (Ares::UISettings::QuickExit ? "ON" : "OFF"));
	Debug::Log("AllowMultiEngineer is %s\n", (Ares::UISettings::AllowMultiEngineer ? "ON" : "OFF"));
	Debug::Log("CampaignList is %s\n", (Ares::UISettings::CampaignList ? "ON" : "OFF"));
	Debug::Log("ShowDebugCampaigns is %s\n", (Ares::UISettings::ShowDebugCampaigns ? "ON" : "OFF"));
	Debug::Log("Color count is %d\n", Ares::UISettings::ColorCount);
	Debug::Log("Mod is %s (%s) with %X\n", Ares::UISettings::ModName,
		Ares::UISettings::ModVersion, Ares::UISettings::ModIdentifier);

	// clean up
	Ares::CloseConfig(pINI);
	return 0;
}
