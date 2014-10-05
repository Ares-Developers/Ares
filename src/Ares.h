#ifndef ARES_H
#define ARES_H

//ifndef str
#define str(x) str_(x)
#define str_(x) #x
//endif

#include <bitset>

//include <YRPP.h>
#include <Helpers/Macro.h>

#include "Misc/Interface.h"
#include "Misc/Debug.h"

#define GFX_DX_HW 0x01l
#define GFX_DX_EM 0x02l

#define GFX_SU_VRAM 0x00l
#define GFX_SU_SYSTEM 0x01l

#define GFX_SU_NF3D 0x00l
#define GFX_SU_F3D 0x01l

class AircraftTypeClass;
class TechnoTypeClass;
class CellClass;
class HouseClass;
class CCINIClass;
class MixFileClass;
class CustomPalette;
struct SHPStruct;

class Ares
{
public:
	enum class ExceptionHandlerMode {
		Default = 0,
		Full = 1,
		NoRemove = 2
	};

	/**
	* called before any saving takes place
	*/
	static void SaveGame();
	/**
	* called after the game (and extdata) has saved its data
	*/
	static HRESULT SaveGameData(IStream *pStm);

	/**
	* called before any loading takes place
	*/
	static void LoadGame();

	/**
	* called after the game (and extdata) has loaded its data
	*/
	static void LoadGameData(IStream *pStm);

	//Global Options
	static HANDLE	hInstance;

	static bool bNoLogo;
	static bool bNoCD;

	static bool bTestingRun;

	static bool bLog;
	static FILE* pLogFile;

	static bool bStrictParser;

	static bool bAllowAIControl;

	static bool bFPSCounter;

	static bool bOutputMissingStrings;

	static int TrackIndex;

	static PVOID pExceptionHandler;
	static ExceptionHandlerMode ExceptionMode;

	static bool bShuttingDown;

	static bool bStable;
	static bool bStableNotification;
	static void UpdateStability();

	static const wchar_t StabilityWarning[];

	static const size_t readLength = 2048;
	static char readBuffer[readLength];
	static const char readDelims[4];
	static const char readDefval[4];

	static MixFileClass *aresMIX;
	static void InitOwnResources();
	static void UninitOwnResources();

	static CCINIClass* OpenConfig(const char*);
	static void CloseConfig(CCINIClass*&);

	static void InitNoCDMode();
	static void CheckProcessorFeatures();

	//Callbacks
	static void __stdcall CmdLineParse(char**,int);

	static void __stdcall ExeRun();
	static void __stdcall ExeTerminate();

	static void __stdcall PostGameInit();

	static void __stdcall RegisterCommands();

	//General functions

	class GlobalControls {
	private:
		GlobalControls() {};
	public:
		static CCINIClass * INI;

		static void LoadConfig();

		static void Load(CCINIClass*);
		static void LoadFromRules(CCINIClass*);

		static bool Initialized;
		static bool AllowParallelAIQueues;

		static bool DebugKeysEnabled;

		static byte GFX_DX_Force;

		static std::bitset<3> AllowBypassBuildLimit;

		class SurfaceConfig {
		public:
			byte Force3D;
			byte Memory;
		};
	};

	class UISettings {
	private:
		UISettings() {};
	public:
		struct CampaignData {
			char Battle[0x18];
			char Subline[0x1F];
			char ToolTip[0x1F];
			CustomPalette* Palette;
			SHPStruct* Image;
			bool Valid;
		};

		struct ColorData {
			wchar_t* id;
			int colorRGB;
			int selectedIndex;
			char colorSchemeIndex;
			char colorScheme[0x20];
			const wchar_t* sttToolTipSublineText;
		};

		static void Load(CCINIClass*);

		static bool Initialized;
		static Interface::eUIAction SinglePlayerButton;
		static Interface::eUIAction WWOnlineButton;
		static Interface::eUIAction NetworkButton;
		static Interface::eUIAction MoviesAndCreditsButton;
		static Interface::eUIAction CampaignButton;
		static Interface::eUIAction SkirmishButton;
		static Interface::eUIAction SneakPeeksButton;
		static Interface::eUIAction PlayMoviesButton;
		static Interface::eUIAction ViewCreditsButton;
		static bool AllowMultiEngineer;
		static bool CampaignList;
		static bool ShowDebugCampaigns;
		static CampaignData Campaigns[4];

		static const int maxColorCount = 16;
		static int ColorCount;
		static ColorData Colors[maxColorCount+1];

		static int uiColorText;
		static int uiColorTextButton;
		static int uiColorTextCheckbox;
		static int uiColorTextRadio;
		static int uiColorTextLabel;
		static int uiColorTextList;
		static int uiColorTextCombobox;
		static int uiColorTextGroupbox;
		static int uiColorTextEdit;
		static int uiColorTextSlider;
		static int uiColorTextObserver;
		static int uiColorCaret;
		static int uiColorSelection;
		static int uiColorSelectionCombobox;
		static int uiColorSelectionList;
		static int uiColorSelectionObserver;
		static int uiColorBorder1;
		static int uiColorBorder2;
		static int uiColorDisabled;
		static int uiColorDisabledLabel;
		static int uiColorDisabledButton;
		static int uiColorDisabledCombobox;
		static int uiColorDisabledCheckbox;
		static int uiColorDisabledList;
		static int uiColorDisabledSlider;
		static int uiColorDisabledObserver;

		static char ModName[0x40];
		static char ModVersion[0x40];
		static int ModIdentifier;
	};
};

#endif
