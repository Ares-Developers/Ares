#include "Skirmish.h"
#include "../UI/registered.h"
#include "../Ares.h"

#include "../Ext/Rules/Body.h"
#include <GameModeOptionsClass.h>

// user clicked on the multi engineer checkbox
DEFINE_HOOK(6ACEE0, Skirmish_DialogFunc_UpdateMultiEngineer, 6) {
	GET(HWND, hDlg, ECX);
	GET(int, nIDDlgItem, EDX);
	
	// update the current state.
	if(nIDDlgItem == ARES_CHK_MULTIENGINEER) {
		HWND hDlgItem = GetDlgItem(hDlg, ARES_CHK_MULTIENGINEER);
		if(hDlgItem) {
			bool enabled = (SendMessageA(hDlgItem, BM_GETCHECK, 0, 0) != 0);
			RulesExt::Global()->MultiEngineer[0] = enabled;
		}
	}

	// copy the setting to a location the game actually uses
	GameModeOptionsClass::Instance->MultiEngineer =
		!Ares::UISettings::AllowMultiEngineer || RulesExt::Global()->MultiEngineer[0];
	return 0;
}

// prepare the game to use or to not use the multi engineer logic
DEFINE_HOOK(6AD8A4, Skirmish_DialogFunc_MultiEngineer, 7) {
	GET(HWND, hDlg, EBP);

	if(Ares::UISettings::AllowMultiEngineer) {
		HWND hDlgItem = GetDlgItem(hDlg, ARES_CHK_MULTIENGINEER);
		if(hDlgItem) {
			bool enabled = (SendMessageA(hDlgItem, BM_GETCHECK, 0, 0) != 0);
			RulesExt::Global()->MultiEngineer[0] = enabled;
			GameModeOptionsClass::Instance->MultiEngineer = enabled;
		}
	} else {
		GameModeOptionsClass::Instance->MultiEngineer = false;
	}

	// recreate the MapPreviewDrawer destructor check
	GET(void*, pMPD, ESI);
	return (pMPD ? 0x6AD8AD : 0x6AD8C7);
}

// initialize the skirmish dialog to check or uncheck the multi engineer checkbox
DEFINE_HOOK(6AEE6A, Skirmish_InitializeDialog_MultiEngineer, 5) {
	GET(HWND, hDlg, EBP);

	// turn on multi engineer
	HWND hDlgItem = GetDlgItem(hDlg, ARES_CHK_MULTIENGINEER);
	if (hDlgItem) {
		// show or hide the menu and update the checkbox
		ShowWindow(hDlgItem, (Ares::UISettings::AllowMultiEngineer ? SW_SHOW : SW_HIDE));
		SendMessageA(hDlgItem, BM_SETCHECK, RulesExt::Global()->MultiEngineer[0] != 0, 0);
	}

	return 0;
}

// save the current multi engineer setting
DEFINE_HOOK(699043, GameMode_SaveGameSettings_MultiEngineer, 5) {
	GET(CCINIClass*, pINI, EDI);
	GET(char*, pSection, ESI);

	if(!_strcmpi(pSection, "Skirmish")) {
		pINI->WriteBool(pSection, "MultiEngineer", RulesExt::Global()->MultiEngineer[0]);
	}

	return 0;
}

// load the multi engineer setting from ini
DEFINE_HOOK(69801A, Game_GetGameTypePrefs_MultiEngineer, 6) {
	//GET(GameModeOptionsClass*, pOptions, EDI);
	GET(CCINIClass*, pINI, EBX);
	GET(char*, pSection, EBP);

	if(!_strcmpi(pSection, "Skirmish")) {
		RulesExt::Global()->MultiEngineer[0] = pINI->ReadBool(pSection, "MultiEngineer", RulesClass::Global()->MultiEngineer);
	}

	return 0;
}