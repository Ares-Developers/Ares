#include "Interface.h"
#include "../Ares.h"
#include "../Ext/Campaign/Body.h"

#include <StringTable.h>
#include <MessageBox.h>
#include <VocClass.h>
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

	if((iID == 148) && Ares::UISettings::CampaignList) {
		auto hide = [hDlg](int nIDDlgItem) {
			if(HWND hItem = GetDlgItem(hDlg, nIDDlgItem)) {
				ShowWindow(hItem, SW_HIDE);
			}
		};

		auto show = [hDlg](int nIDDlgItem) {
			if(HWND hItem = GetDlgItem(hDlg, nIDDlgItem)) {
				ShowWindow(hItem, SW_SHOW);
			}
		};

		auto offset = [hDlg](int nIDDlgItem, int x, int y) {
			if(HWND hItem = GetDlgItem(hDlg, nIDDlgItem)) {
				POINT ptDlg = {0, 0};
				ScreenToClient(hDlg, &ptDlg);

				RECT rcItem;
				GetWindowRect(hItem, &rcItem);

				OffsetRect(&rcItem, x, y);
				moveItem(hItem, rcItem, ptDlg);
			}
		};

		if(HWND hItem = GetDlgItem(hDlg, 1109)) {
			// extensive stuff
			show(1109);
			show(1038);
			hide(1770);
			hide(1772);

			POINT ptDlg = {0, 0};
			ScreenToClient(hDlg, &ptDlg);

			// use the position of the Allied button to place the
			// new campaign selection list.
			RECT rcItem = {125, 34, 125 + 174, 34 + 87};
			if(HWND hAllImage = GetDlgItem(hDlg, 1770)) {
				GetWindowRect(hAllImage, &rcItem);
			}
			offset(1959, 0, -rcItem.bottom + rcItem.top);

			// center the list above the difficulty selection. the list may
			// contain seven items, after that, a scroll bar will appear.
			// acount for its width, too.
			int offList = (CampaignExt::countVisible() < 8 ? -2 : -12);
			OffsetRect(&rcItem, offList, 32);
			moveItem(hItem, rcItem, ptDlg);
			
			// let the Allied label be the caption
			if(HWND hAllLabel = GetDlgItem(hDlg, 1959)) {
				SendMessageA(hAllLabel, 0x4B2, 0, (LPARAM)StringTable::LoadStringA("GUI:SelectCampaign"));
			}

			// call the load button "Play"
			if(HWND hLoad = GetDlgItem(hDlg, 1038)) {
				SendMessageA(hLoad, 0x4B2, 0, (LPARAM)StringTable::LoadStringA("GUI:Play"));
			}

			// move the soviet label to a new location and reuse
			// it to show the selected campaigns summary.
			if(HWND hSovImage = GetDlgItem(hDlg, 1772)) {
				GetWindowRect(hSovImage, &rcItem);
				if(HWND hSovLabel = GetDlgItem(hDlg, 1960)) {
					// remove default text and move label
					SendMessageA(hSovLabel, 0x4B2, 0, (LPARAM)L"");
					moveItem(hSovLabel, rcItem, ptDlg);
					
					// left align text
					DWORD style = GetWindowLong(hSovLabel, GWL_STYLE);
					style = SS_LEFT | WS_CHILD | WS_VISIBLE;
					SetWindowLong(hSovLabel, GWL_STYLE, style);
				}
			}

			// reset the selection cache
			CampaignExt::lastSelectedCampaign = -1;
		}
	}

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

// Hook up the new campaign list. Instead of using the index in the list
// to load the selected campaign we use the item data to remember which
// index to use. This makes it possible to support a mixed list of
// debug/release campaigns.

// We do not select the first item in the list as the game would do. This
// allows us to play the campaign selection sound when the user clicks it
// and not when the dialog is shown or not at all.
DEFINE_HOOK(52F00B, CampaignMenu_hDlg_PopulateCampaignList, 5) {
	GET(HWND, hDlg, ESI);
	GET(HWND, hList, EBP);

	// use button selection screen?
	if(!Ares::UISettings::CampaignList) {
		return 0;
	}

	// fill in the campaigns list
	for(int i=0; i<CampaignExt::Array.Count; ++i) {
		CampaignExt::ExtData *pData = CampaignExt::Array.GetItem(i);
		if(pData && pData->isVisible()) {
			int newIndex = SendMessageA(hList, 0x4CD, 0, (WPARAM)pData->AttachedToObject->Description);
			SendMessageA(hList, LB_SETITEMDATA, newIndex, i);
		}
	}

	// disable the play button as there is nothing selected. we don't select
	// the first campaign here so the user will get the introduction sound.
	if(HWND hItem = GetDlgItem(hDlg, 1038)) {
		EnableWindow(hItem, false);
	}

	return 0x52F07F;
}

DEFINE_HOOK(52EC18, CampaignClass_hDlg_PreHandleGeneral, 5) {
	GET(HWND, hDlg, ESI);
	GET(int, msg, EBX);
	GET(int, wParam, EDI);
	GET(int, lParam, EBP);

	// catch the selection change event of the campaign list
	if(msg == WM_COMMAND) {
		int iID = LOWORD(lParam);
		int iCmd = HIWORD(lParam);
		if((iID == 1109) && (iCmd == LBN_SELCHANGE)) {
			int index = SendDlgItemMessageA(hDlg, 1109, LB_GETCURSEL, 0, 0);
			int idxCampaign = SendDlgItemMessageA(hDlg, 1109, LB_GETITEMDATA, index, 0);

			if(CampaignExt::lastSelectedCampaign != idxCampaign) {
				// play the hover sound
				CampaignExt::ExtData* pData = CampaignExt::Array.GetItem(idxCampaign);
				if(pData) {
					int idxSound = VocClass::FindIndex(pData->HoverSound);
					if(idxSound > -1) {
						VocClass::PlayGlobal(idxSound, 1.0f, 8192, 0);
					}

					// set the summary text
					if(HWND hSovLabel = GetDlgItem(hDlg, 1960)) {
						const wchar_t* summary = NULL;
						if(*pData->Summary) {
							summary = StringTable::LoadStringA(pData->Summary);
						}
						SendMessageA(hSovLabel, 0x4B2, 0, (LPARAM)summary);
					}
				}

				// cache the selected index
				CampaignExt::lastSelectedCampaign = idxCampaign;
			}

			// enable the play button
			if(HWND hItem = GetDlgItem(hDlg, 1038)) {
				EnableWindow(hItem, (index >= 0));
			}
		}
	}

	return 0;
}

DEFINE_HOOK(52ED21, CampaignClass_hDlg_ClickedPlay, 9) {
	GET(HWND, hDlg, ESI);

	// find out which campaign is selected
	int idxItem = SendDlgItemMessageA(hDlg, 1109, LB_GETCURSEL, 0, 0);
	int idxCampaign = SendDlgItemMessageA(hDlg, 1109, LB_GETITEMDATA, idxItem, 0);

	// set it ourselves
	R->EAX(idxCampaign);

	return 0x52ED2D;
}