#include "Interface.h"
#include "../Ares.h"

#include <StringTable.h>
#include <MessageBox.h>
#include <Strsafe.h>

int Interface::lastDialogTemplateID = 0;
int Interface::nextAction = -1;
int Interface::nextReturnMenu = -1;
const wchar_t* Interface::nextMessageText = NULL;

bool Interface::invokeClickAction(eUIAction action, char* name, int* pResult, int nextMenu) {
	nextAction = -1;
	nextReturnMenu = -1;
	nextMessageText = NULL;

	auto ret = [&](int _nextAction) -> bool {
		*pResult = nextAction = _nextAction;
		nextReturnMenu = nextMenu;
		return (_nextAction > -1);
	};

	if(action == Interface::uia_Message) {
		// generate the label name
		char *buffer = new char[0x20];
		StringCchPrintfA(buffer, 0x20, "TXT_%s_MSG", name);
		nextMessageText = StringTable::LoadStringA(buffer);
		delete [] &buffer;

		// hide dialog temporarily and show a message box
		return ret(6);
	}

	if(action == Interface::uia_SneakPeak) {
		// show the preview video
		return ret(13);
	}

	if(action == Interface::uia_Credits) {
		// show the credits
		return ret(15);
	}

	return false;
}

void Interface::updateMenuItems(HWND hDlg, MenuItem* items, int count) {
	// account for dialog nc size
	POINT ptDlg = {0, 0};
	ScreenToClient(hDlg, &ptDlg);

	int iButton = 0;
	RECT* rcOriginal = new RECT[count];
	for(int i=0; i<count; ++i) {
		if(HWND hItem = GetDlgItem(hDlg, items[i].nIDDlgItem)) {
			GetWindowRect(hItem, &rcOriginal[i]);

			if(items[i].uiaAction == Interface::uia_Hide) {
				// hide the window
				ShowWindow(hItem, SW_HIDE);
			} else {
				if(items[i].uiaAction == Interface::uia_Disable) {
					// disable the button
					EnableWindow(hItem, false);
				}

				if(i != iButton) {
					// move the button to the next free position
					moveItem(hItem, rcOriginal[iButton], ptDlg);
				}
				++iButton;
			}
		}
	}

	delete [] &rcOriginal;
}

void Interface::moveItem(HWND hItem, RECT rcItem, POINT ptOffset) {
	OffsetRect(&rcItem, ptOffset.x, ptOffset.y);
	MoveWindow(hItem, rcItem.left, rcItem.top, 
		rcItem.right - rcItem.left, rcItem.bottom - rcItem.top, false);
}

void Interface::swapItems(HWND hDlg, int nIDDlgItem1, int nIDDlgItem2) {
	HWND hFirst = GetDlgItem(hDlg, nIDDlgItem1);
	HWND hSecond = GetDlgItem(hDlg, nIDDlgItem2);

	if(hFirst && hSecond) {
		RECT rcFirst, rcSecond;
		if(GetWindowRect(hFirst, &rcFirst) && GetWindowRect(hSecond, &rcSecond)) {
			POINT ptDlg = {0, 0};
			ScreenToClient(hDlg, &ptDlg);
			moveItem(hFirst, rcSecond, ptDlg);
			moveItem(hSecond, rcFirst, ptDlg);
		}
	}
}

void Interface::updateMenu(HWND hDlg) {
	int iID = Interface::lastDialogTemplateID;

	// main menu
	if(iID == 226) {
		struct Interface::MenuItem items[] = {{0x683, Ares::UISettings::SinglePlayerButton},
			{0x684, Ares::UISettings::WWOnlineButton}, {0x578, Ares::UISettings::NetworkButton},
			{0x686, Ares::UISettings::MoviesAndCreditsButton}, {0x55C, Interface::uia_Default}};
		Interface::updateMenuItems(hDlg, items, 5);
	}

	// singleplayer menu
	if(iID == 256) {
		// swap skirmish and load buttons so load will not appear first
		if(Ares::UISettings::CampaignButton != Interface::uia_Hide) {
			struct Interface::MenuItem items[] = {{1672, Ares::UISettings::CampaignButton},
				{1673, Interface::uia_Default}, {1401, Ares::UISettings::SkirmishButton}};
			Interface::updateMenuItems(hDlg, items, 3);
		} else {
			Interface::swapItems(hDlg, 0x688, 0x579);
			struct Interface::MenuItem items[] = {{1401, Ares::UISettings::SkirmishButton},
				{1673, Interface::uia_Default}, {1672, Ares::UISettings::CampaignButton}};
			Interface::updateMenuItems(hDlg, items, 3);
		}
	}

	// movies and credits menu
	if(iID == 257) {
		struct Interface::MenuItem items[] = {{0x68D, Ares::UISettings::SneakPeaksButton},
			{0x68E, Ares::UISettings::PlayMoviesButton}, {0x68F, Ares::UISettings::ViewCreditsButton}};
		Interface::updateMenuItems(hDlg, items, 3);
	}

	// one-button message box
	if(iID == 206) {
		// more room for text
		if(HWND hItem = GetDlgItem(hDlg, 0x5B0)) {
			POINT ptDlg = {0, 0};
			ScreenToClient(hDlg, &ptDlg);

			RECT rcItem;
			GetWindowRect(hItem, &rcItem);

			rcItem.bottom = rcItem.top + 200;
			moveItem(hItem, rcItem, ptDlg);
		}
	}
}

Interface::eUIAction Interface::parseUIAction(char* value, Interface::eUIAction def) {
	if(!_strcmpi(value, "message")) {
		return Interface::uia_Message;
	} else if(!_strcmpi(value, "disable")) {
		return Interface::uia_Disable;
	} else if(!_strcmpi(value, "hide")) {
		return Interface::uia_Hide;
	} else if(!_strcmpi(value, "credits")) {
		return Interface::uia_Credits;
	} else if(!_strcmpi(value, "sneakpeak")) {
		return Interface::uia_SneakPeak;
	}
	return def;
}

// cache the template id for later use
DEFINE_HOOK(62267F, Dialog_Show_GetTemplate, 6) {
	GET(int, iID, EBP);
	Interface::lastDialogTemplateID = iID;
	return 0;
}

// manipulate the dialog after creation
DEFINE_HOOK(6226EE, Dialog_Show_UpdateControls, 6) {
	GET(HWND, hDlg, ESI);
	Interface::updateMenu(hDlg);
	return 0;
}

// show message
DEFINE_HOOK(52DDBA, Frontend_WndProc_MessageBox, 5) {
	if(Interface::nextMessageText) {
		const wchar_t* ok = StringTable::LoadStringA("TXT_OK");
		MessageBox::Show(Interface::nextMessageText, ok, NULL);
		Interface::nextMessageText = NULL;
		return 0x52DE39;
	}

	return 0;
}

// go to the return menu instead of the menu Westwood would like to see here.
DEFINE_HOOK(52E446, Frontend_WndProc_JustAfterAction, 6) {
	GET(int, thisAction, ESI);
	if((Interface::nextAction != thisAction) && (Interface::nextReturnMenu >= 0)) {
		R->ESI(Interface::nextReturnMenu);
		Interface::nextReturnMenu = -1;
	}

	return 0;
}

// hooks for different buttons
DEFINE_HOOK(53208D, Main_hDlg_SinglePlayerButtonClick, 6) {
	GET(int*, pAction, EAX);
	return (Interface::invokeClickAction(Ares::UISettings::SinglePlayerButton,
		"SINGLEPLAYERBUTTON", pAction, 18) ? 0x532093 : 0);
}

DEFINE_HOOK(5320C2, Main_hDlg_WWOnlineButtonClick, 6) {
	GET(int*, pAction, EAX);
	return (Interface::invokeClickAction(Ares::UISettings::WWOnlineButton,
		"WWONLINEBUTTON", pAction, 18) ? 0x5320C8 : 0);
}

DEFINE_HOOK(532051, Main_hDlg_NetworkButtonClick, 6) {
	GET(int*, pAction, EAX);
	return (Interface::invokeClickAction(Ares::UISettings::NetworkButton,
		"NETWORKBUTTON", pAction, 18) ? 0x532057 : 0);
}

DEFINE_HOOK(5320AE, Main_hDlg_MoviesAndCreditsButtonClick, 6) {
	GET(int*, pAction, EAX);
	return (Interface::invokeClickAction(Ares::UISettings::MoviesAndCreditsButton,
		"MOVIESANDCREDITSBUTTON", pAction, 18) ? 0x5320B4 : 0);
}

DEFINE_HOOK(52D724, SinglePlayer_hDlg_CampaignButtonClick, 6) {
	GET(int*, pAction, EAX);
	return (Interface::invokeClickAction(Ares::UISettings::CampaignButton,
		"CAMPAIGNBUTTON", pAction, 1) ? 0x52D72A : 0);
}

DEFINE_HOOK(52D713, SinglePlayer_hDlg_SkirmishButtonClick, 6) {
	GET(int*, pAction, EAX);
	return (Interface::invokeClickAction(Ares::UISettings::SkirmishButton,
		"SKIRMISHBUTTON", pAction, 1) ? 0x52D719 : 0);
}

DEFINE_HOOK(52D7ED, MoviesAndCredits_hDlg_SneakPeaksButtonClick, 6) {
	GET(int*, pAction, EAX);
	return (Interface::invokeClickAction(Ares::UISettings::SneakPeaksButton,
		"SNEAKPEAKSBUTTON", pAction, 4) ? 0x52D7F3 : 0);
}

DEFINE_HOOK(52D7FB, MoviesAndCredits_hDlg_PlayMoviesButtonClick, 6) {
	GET(int*, pAction, EAX);
	return (Interface::invokeClickAction(Ares::UISettings::PlayMoviesButton,
		"PLAYMOVIESBUTTON", pAction, 4) ? 0x52D801 : 0);
}

DEFINE_HOOK(52D809, MoviesAndCredits_hDlg_ViewCreditsButtonClick, 6) {
	GET(int*, pAction, EAX);
	return (Interface::invokeClickAction(Ares::UISettings::ViewCreditsButton,
		"VIEWCREDITSBUTTON", pAction, 4) ? 0x52D80F : 0);
}
