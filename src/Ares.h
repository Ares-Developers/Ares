#ifndef ARES_H
#define ARES_H

//ifndef str
#define str(x) str_(x)
#define str_(x) #x
//endif

#include <bitset>

#include <xcompile.h>
//include <YRPP.h>
#include <Helpers/Macro.h>

#include <CCINIClass.h>
#include <MixFileClass.h>

#include "Misc/Interface.h"
#include "Misc/Debug.h"

#include <AircraftTypeClass.h>
#include <CellClass.h>
#include <HouseClass.h>
#include <InfantryTypeClass.h>
#include <UnitTypeClass.h>

#define GFX_DX_HW 0x01l
#define GFX_DX_EM 0x02l

#define GFX_SU_VRAM 0x00l
#define GFX_SU_SYSTEM 0x01l

#define GFX_SU_NF3D 0x00l
#define GFX_SU_F3D 0x01l

template <typename T>
class Iterator;

class Ares
{
public:
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

	static bool bStable;
	static bool bStableNotification;
	static void UpdateStability();

	static const wchar_t StabilityWarning[BUFLEN];

	static DWORD readLength;
	static char readBuffer[BUFLEN];
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
	static void SendPDPlane(
		HouseClass* pOwner,
		CellClass* pDestination,
		AircraftTypeClass* pPlaneType,
		const Iterator<TechnoTypeClass*> &Types,
		const Iterator<int> &Nums);

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
		static Interface::CampaignData Campaigns[4];

		static const int maxColorCount = 16;
		static int ColorCount;
		static Interface::ColorData Colors[maxColorCount+1];

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
	};
};

class MemMap {
public:
	typedef hash_map <DWORD, size_t> memmap;
	static hash_map <DWORD, size_t> AllocMap;
	static size_t Total;

	static void Add(void * _addr, size_t amount) {
		DWORD addr = (DWORD)_addr;
		memmap::iterator i = AllocMap.find(addr);
		if(i != AllocMap.end()) {
#ifdef MEMORY_LOGGING
			Debug::Log("Reallocated a used block of 0x%X bytes @ 0x%X!\n", amount, addr);
#endif
		}
		AllocMap[addr] = amount;
		Total += amount;
	}

	static size_t Remove(void * _addr) {
		DWORD addr = (DWORD)_addr;
		memmap::iterator i = AllocMap.find(addr);
		if(i == AllocMap.end()) {
#ifdef MEMORY_LOGGING
			Debug::Log("Deallocated a dud block @ 0x%X!\n", addr);
#endif
			return 0;
		} else {
			size_t amount = AllocMap[addr];
			Total -= amount;
			AllocMap.erase(addr);
			return amount;
		}
	}
};

struct GameDeleter {
	template <typename T>
	void operator ()(T* ptr) {
		if(ptr) {
			GameDelete(ptr);
		}
	}
};

#endif
