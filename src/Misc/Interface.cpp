#include "Interface.h"
#include "../Ares.h"
#include "../Ares.CRT.h"
#include "../Ext/Campaign/Body.h"
#include "../Utilities/Constructs.h"

#include <StringTable.h>
#include <MessageBox.h>
#include <VocClass.h>
#include <Strsafe.h>

int Interface::lastDialogTemplateID = 0;
int Interface::nextAction = -1;
int Interface::nextReturnMenu = -1;
const wchar_t* Interface::nextMessageText = NULL;

int Interface::slots[4]; // holds index-of-campaign+1

//! Overrides the default action and queue the user defined one.
/*!
	Called from within the button click event to override the game's
	default action. Sets the user defined action and/or the message.

	\param action The eUIAction to perform.
	\param name The button name to get the message string of.
	\param pResult A pointer to the game dialog's user info.
	\param nextMenu The menu to go to after showing the message.

	\returns True if action has been overridden, false otherwise.

	\author AlexB
	\date 2010-06-20
*/
bool Interface::invokeClickAction(eUIAction action, char* name, int* pResult, int nextMenu) {
	// reset
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

	if(action == Interface::uia_SneakPeek) {
		// show the preview video
		return ret(13);
	}

	if(action == Interface::uia_Credits) {
		// show the credits
		return ret(15);
	}

	// do default stuff
	return false;
}

//! Disables, hides and moves menu items so there are no gaps in the menu navigation.
/*!
	Menu items will be disabled or hidden. For the latter, all succeeding
	items are moved to the next free position, closing all gaps.

	\param hDlg The dialog to update.
	\param items An array of MenuItems to update.
	\param count Length of items.

	\author AlexB
	\date 2010-06-20
*/
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

//! Moves a menu item to a new location using an optional offset.
/*!
	Helper function to move an dialog item and offset in one go.
	The offset is needed to account for non-client areas of the
	windowed game.

	\param hItem Handle of the item to update.
	\param rcItem The item's new bounds.
	\param ptOffset Special offset.

	\author AlexB
	\date 2010-06-20
*/
void Interface::moveItem(HWND hItem, RECT rcItem, POINT ptOffset) {
	OffsetRect(&rcItem, ptOffset.x, ptOffset.y);
	MoveWindow(hItem, rcItem.left, rcItem.top, 
		rcItem.right - rcItem.left, rcItem.bottom - rcItem.top, false);
}

//! Swaps the bounds of two dialog items.
/*!
	Both button's bounds are swapped. This can be used to fix unintuitive
	menu button order if some items are hidden.

	\param hDlg The dialog to update.
	\param nIDDlgItem1 Item one.
	\param nIDDlgItem2 Item two.

	\author AlexB
	\date 2010-06-20
*/
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

//! Updates the dialogs after creation to better fit the user's needs.
/*!
	This function does dialog dependent stuff. Some controls are moved,
	or disabled, new Ares controls are hidden if not explicitly enabled.

	\param hDlg The dialog to update.
	
	\author AlexB
	\date 2010-06-20
*/
void Interface::updateMenu(HWND hDlg) {
	int iID = Interface::lastDialogTemplateID;

	// campaign selection
	if(iID == 148) {
		// hide item by iID
		auto hide = [hDlg](int nIDDlgItem) {
			if(HWND hItem = GetDlgItem(hDlg, nIDDlgItem)) {
				ShowWindow(hItem, SW_HIDE);
			}
		};

		// show item by iID
		auto show = [hDlg](int nIDDlgItem) {
			if(HWND hItem = GetDlgItem(hDlg, nIDDlgItem)) {
				ShowWindow(hItem, SW_SHOW);
			}
		};

		// move item by some pixels
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

		POINT ptDlg = {0, 0};
		ScreenToClient(hDlg, &ptDlg);

		// new campaign list versus default click selection
		if(Ares::UISettings::CampaignList) {
			if(HWND hItem = GetDlgItem(hDlg, 1109)) {
				// extensive stuff
				show(1109);
				show(1038);
				hide(1770);
				hide(1772);
				hide(1771);
				hide(1773);

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
		} else {
			// default way with starting a campaign by clicking on its image.

			// The allied image defines the image size.
			RECT rcItem = {125, 34, 125 + 174, 34 + 87};
			if(HWND hAllImage = GetDlgItem(hDlg, 1770)) {
				GetWindowRect(hAllImage, &rcItem);
			}
			SIZE szImage;
			szImage.cx = (int)((rcItem.right - rcItem.left) * .8);
			szImage.cy = (int)((rcItem.bottom - rcItem.top) * 1.0);

			// the soviet image's top is used for the second row
			RECT rcSovImage = {0, 216, 0, 0};
			if(HWND hSovImage = GetDlgItem(hDlg, 1772)) {
				GetWindowRect(hSovImage, &rcSovImage);
			}
			int row2Offset = rcSovImage.top - rcItem.top;

			// call the load button "Play"
			if(HWND hLoad = GetDlgItem(hDlg, 1038)) {
				SendMessageA(hLoad, 0x4B2, 0, (LPARAM)StringTable::LoadStringA("GUI:PlayMission"));
			}

			// position values
			RECT rcWidth = rcItem;
			OffsetRect(&rcWidth, ptDlg.x, ptDlg.y);
			int width = rcWidth.left + rcWidth.right;

			int lefts[3];
			lefts[0] = (width - szImage.cx) / 2;
			lefts[1] = (width - 2 * szImage.cx) / 3;
			lefts[2] = width - lefts[1] - szImage.cx;

			// create seven slot rects
			auto setRect = [&](RECT *rcRect, int x, int y) {
				rcRect->left = x - ptDlg.x;
				rcRect->top = y;
				rcRect->right = rcRect->left + szImage.cx;
				rcRect->bottom = rcRect->top + szImage.cy;
			};

			RECT *rcSlots = new RECT[7];
			setRect(&rcSlots[0], lefts[0], rcItem.top);
			setRect(&rcSlots[1], lefts[0], rcItem.top + row2Offset);
			setRect(&rcSlots[2], lefts[1], rcItem.top);
			setRect(&rcSlots[3], lefts[2], rcItem.top);
			setRect(&rcSlots[4], lefts[1], rcItem.top + row2Offset);
			setRect(&rcSlots[5], lefts[2], rcItem.top + row2Offset);
			setRect(&rcSlots[6], -szImage.cx, -szImage.cy);

			// move the images to their new locations
			auto fillSlot = [&](int iIDDlgItem, int slot, int iIDLabel) {
				if(HWND hItem = GetDlgItem(hDlg, iIDDlgItem)) {
					moveItem(hItem, rcSlots[slot], ptDlg);
				}

				if(HWND hLabel = GetDlgItem(hDlg, iIDLabel)) {
					if(slot < 6) {
						RECT rcLabel = rcSlots[slot];
						OffsetRect(&rcLabel, 0, (rcLabel.bottom - rcLabel.top));
						rcLabel.bottom = rcLabel.top + 20;

						// make the subtitle a little wider
						int widen = (int)(slot < 2 ? (width - rcLabel.right + rcLabel.left) / 2 : 15);
						rcLabel.left -= widen;
						rcLabel.right += widen;

						moveItem(hLabel, rcLabel, ptDlg);
						ShowWindow(hLabel, SW_SHOW);
					}
					else {
						ShowWindow(hLabel, SW_HIDE);
					}
				}
			};

			// move image and label. auto-center, if there is no neighbour.
			auto moveToPlace = [&](int iID, int index, int neighbour, int slot, int center, int iIDLabel) {
				if(slots[index]) {
					fillSlot(iID, (slots[neighbour] ? slot : center), iIDLabel);
				} else {
					fillSlot(iID, 6, iIDLabel);
				}
			};

			// move click zones and labels
			moveToPlace(1770, 0, 1, 2, 0, 1959);
			moveToPlace(1772, 1, 0, 3, 0, 1960);
			moveToPlace(1771, 2, 3, 4, 1, 1961);
			moveToPlace(1773, 3, 2, 5, 1, 1962);

			delete [] &rcSlots;
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
		struct Interface::MenuItem items[] = {{0x68D, Ares::UISettings::SneakPeeksButton},
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

//! Parses an eUIAction from a string.
/*!
	Converts the string to an eUIAction.

	\param value The string to parse.
	\param def The eUIAction returned for invalid values.

	\returns Parsed eUIAction if value is valid, def otherwise.

	\author AlexB
	\date 2010-06-20
*/
Interface::eUIAction Interface::parseUIAction(char* value, Interface::eUIAction def) {
	if(!_strcmpi(value, "message")) {
		return Interface::uia_Message;
	} else if(!_strcmpi(value, "disable")) {
		return Interface::uia_Disable;
	} else if(!_strcmpi(value, "hide")) {
		return Interface::uia_Hide;
	} else if(!_strcmpi(value, "credits")) {
		return Interface::uia_Credits;
	} else if(!_strcmpi(value, "sneakpeek")) {
		return Interface::uia_SneakPeek;
	} else if(_strcmpi(value, "default")) {
		Debug::DevLog(Debug::Warning, "Unrecognized UI action value: %s\n", value);
	}
	return def;
}

//! Converts a control ID to its corresponding index in slots.
/*!
	Converts an control ID to a slot index.

	\param iID The control ID.

	\returns The index in the slot array.

	\author AlexB
	\date 2010-06-20
*/
int Interface::getSlotIndex(int iID) {
	if(iID == 1770) {
		return 0;
	} else if (iID == 1772) {
		return 1;
	} else if (iID == 1771) {
		return 2;
	} else if (iID == 1773) {
		return 3;
	}
	return -1;
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

DEFINE_HOOK(52D7ED, MoviesAndCredits_hDlg_SneakPeeksButtonClick, 6) {
	GET(int*, pAction, EAX);
	return (Interface::invokeClickAction(Ares::UISettings::SneakPeeksButton,
		"SNEAKPEEKSBUTTON", pAction, 4) ? 0x52D7F3 : 0);
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

// catch selecting a new campaign from the list
DEFINE_HOOK(52EC18, CampaignMenu_hDlg_PreHandleGeneral, 5) {
	GET(HWND, hDlg, ESI);
	GET(int, msg, EBX);
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

// start the mission
DEFINE_HOOK(52ED21, CampaignMenu_hDlg_ClickedPlay, 9) {
	GET(HWND, hDlg, ESI);

	// find out which campaign is selected
	int idxItem = SendDlgItemMessageA(hDlg, 1109, LB_GETCURSEL, 0, 0);
	int idxCampaign = SendDlgItemMessageA(hDlg, 1109, LB_GETITEMDATA, idxItem, 0);

	// set it ourselves
	R->EAX(idxCampaign);

	return 0x52ED2D;
}

// the extended default campaign selection menu.
// Red Alert 2 supported three campaigns in its campaign selection menu.
// Yuri's Revenge removed the tutorial. Ares adds two new buttons which
// can have their own images and color palettes.

// play mouse click sound
DEFINE_HOOK(52EF39, CampaignMenu_hDlg_ImageMouseDown, 5) {
	GET(int, iID, EAX);

	// this is an image button. play click sound.
	int idxSlot = Interface::getSlotIndex(iID);
	if(idxSlot > -1) {
		int idxCampaign = Interface::slots[idxSlot]-1;
		if((idxCampaign > -1) && *Ares::UISettings::Campaigns[idxCampaign].Battle) {
			return 0x52EF52;
		}
	}

	return 0x52F3DC;
}

// select the hover sound to be played
DEFINE_HOOK(52EE04, CampaignMenu_hDlg_SelectHoverSound, 6) {
	GET(HWND, hDlg, ESI);
	GET(int, iID, EBX);
	GET(int, lastiID, EAX);

	int idxSlot = Interface::getSlotIndex(iID);
	if(idxSlot > -1) {
		if(iID != lastiID) {
			// get the campaign hovered above
			int idxCampaign = Interface::slots[idxSlot]-1;

			char* campaignID = NULL;
			if(idxCampaign > -1) {
				campaignID = Ares::UISettings::Campaigns[idxCampaign].Battle;
			}

			// find the campaign hover sound
			if(campaignID && *campaignID) {
				int sound = -1;
				int idxBattle = CampaignClass::FindIndex(campaignID);
				if(idxBattle > -1) {
					if(CampaignExt::ExtData *pData = CampaignExt::Array.GetItem(idxBattle)) {
						sound = VocClass::FindIndex(pData->HoverSound);
					}
				}

				// the actual book keeping
				SendDlgItemMessageA(hDlg, iID, 0x4D3, 0, 0);
				*(DWORD*)(0x825C20) = sound;

				return 0x52EE2D;
			}
		}
		return 0x52F3DC;
	}

	return 0;
}

// override the campaign the game would choose
DEFINE_HOOK(52F232, CampaignMenu_hDlg_StartCampaign, 6) {
	GET(int, iID, EBP);

	// get the campaign hovered above
	int idxSlot = Interface::getSlotIndex(iID);
	char* campaignID = NULL;
	if(idxSlot > -1) {
		int idxCampaign = Interface::slots[idxSlot]-1;
		if(idxCampaign > -1) {
			campaignID = Ares::UISettings::Campaigns[idxCampaign].Battle;
		}
	}

	R->ECX(campaignID);
	return 0x52F260;
}

// converter
DEFINE_HOOK(60378B, CampaignMenu_ChooseButtonPalette, 6) {
	GET(int, iID, EDI);

	int idxSlot = Interface::getSlotIndex(iID);

	if(idxSlot > -1) {
		int idxCampaign = Interface::slots[idxSlot]-1;
		if(idxCampaign > -1 && Ares::UISettings::Campaigns[idxCampaign].Palette->Convert) {
			R->EAX(Ares::UISettings::Campaigns[idxCampaign].Palette->Convert);
			return 0x603798;
		}
	}
	return 0x6037FE;
}

// buttom image
DEFINE_HOOK(603A2E, CampaignMenu_ChooseButtonImage, 6) {
	GET(int, iID, EAX);

	int idxSlot = Interface::getSlotIndex(iID);

	if(idxSlot > -1) {
		int idxCampaign = Interface::slots[idxSlot]-1;
		if(idxCampaign > -1) {
			R->EAX(Ares::UISettings::Campaigns[idxCampaign].Image);
			return 0x603A3A;
		}
	}

	return 0x603A35;
}

// support button background images for every button
DEFINE_HOOK(60A90A, CampaignMenu_StaticButtonImage, 5) {
	GET(HWND, iID, EAX);

	return ((Interface::getSlotIndex((int)iID) > -1) ? 0x60A982 : 0x60A9ED);
}

// animation duration
DEFINE_HOOK(60357E, CampaignMenu_SetAnimationDuration, 5) {
	GET(HWND, iID, EAX);

	return ((Interface::getSlotIndex((int)iID) > -1) ? 0x6035C5 : 0x6035E6);
}

// initialize stuff like the order and images
DEFINE_HOOK(52F191, CampaignMenu_InitializeMoreButtons, 5) {
	GET(HWND, hDlg, ESI);

	if(!Ares::UISettings::CampaignList) {
		// register buttons as campaign buttons
		SendDlgItemMessageA(hDlg, 1773, 0x4D5u, 0, 0);
		SendDlgItemMessageA(hDlg, 1773, 0x4D4u, 0, 0);

		if(HWND hItem = GetDlgItem(hDlg, 1771)) {
			PostMessageA(hItem, 0x4D7u, 0, (LPARAM)hDlg);
		}

		if(HWND hItem = GetDlgItem(hDlg, 1773)) {
			PostMessageA(hItem, 0x4D7u, 0, (LPARAM)hDlg);
		}

		// create the order in which the campaigns will appear.
		// this matters for three campaigns as it seems the
		// usual item count should be 1-2 and not 2-1.
		if(Ares::UISettings::Campaigns[3].Valid) {
			// in order of appearance
			for(int i=0; i<4; ++i) {
				Interface::slots[i] = i + 1;
			}
		} else {
			// at most three campaigns. special order.
			int order[4] = {1, 0, 2, 3};
			for(int i=0; i<4; ++i) {
				Interface::slots[i] = order[i];
			}
		}

		// remove the ones that are not there
		for(int i=0; i<4; ++i) {
			int idxCampaign = Interface::slots[i] - 1;
			if(!Ares::UISettings::Campaigns[idxCampaign].Valid) {
				// disable slot
				Interface::slots[i] = 0;
			} else {
				// update the subline text
				if(HWND hItem = GetDlgItem(hDlg, i + 1959)) {
				  SendMessageA(hItem, 0x4B2u, 0, (LPARAM)StringTable::LoadStringA(Ares::UISettings::Campaigns[idxCampaign].Subline));
				}
			}
		}
	}

	return 0;
}

// if this is a campaign button, handle the tooltip ourselves
DEFINE_HOOK(6041ED, DialogFunc_SubText_CampaignIconA, 5) {
	GET(int, iID, EAX);

	int idxSlot = Interface::getSlotIndex(iID);
	if(idxSlot > -1) {
		int idxCampaign = Interface::slots[idxSlot]-1;
		if((idxCampaign > -1) && *Ares::UISettings::Campaigns[idxCampaign].ToolTip) {
			R->EAX(Ares::UISettings::Campaigns[idxCampaign].ToolTip);
			return 0x6041F4;
		}
	}

	return 0x60421D;
}

// already set. skip this.
DEFINE_HOOK(6041F5, DialogFunc_CampaignMenu_CampaignIconB, 5) {
	return 0x6041FA;
}

// convert player color slot index to color scheme index
DEFINE_HOOK(69A310, Game_GetLinkedColor, 7) {
	GET_STACK(int, idx, 0x4);

	// get the slot
	Interface::ColorData* slot = NULL;
	if(idx == -2 || idx == Ares::UISettings::ColorCount) {
		// observer color
		slot = &Ares::UISettings::Colors[0];
	} else if(idx < Ares::UISettings::ColorCount) {
		// house color
		slot = &Ares::UISettings::Colors[idx+1];
	}

	// retrieve the color scheme index
	int ret = 0;
	if(slot) {
		if(slot->colorSchemeIndex == -1) {
			slot->colorSchemeIndex = ColorScheme::FindIndex(slot->colorScheme);
			if(slot->colorSchemeIndex == -1) {
				Debug::Log("Color scheme \"%s\" not found.\n", slot->colorScheme ? slot->colorScheme : "");
				slot->colorSchemeIndex = 4;
			}
		}
		ret = slot->colorSchemeIndex;
	}
	
	R->EAX(ret + 1);
	return 0x69A334;
}

// return the tool tip describing this color
DEFINE_HOOK(4E42A0, GameSetup_GetColorTooltip, 5) {
	GET(int, idx, ECX);

	const wchar_t* ret = NULL;
	if(idx == -2) {
		// random
		ret = StringTable::LoadStringA("STT:PlayerColorRandom");
	} else if(idx <= Ares::UISettings::ColorCount) {
		// houses and observer
		ret = Ares::UISettings::Colors[(idx+1)%(Ares::UISettings::ColorCount+1)].sttToolTipSublineText;
	}

	R->EAX(ret);
	return 0x4E43B9;
}

// handle adding colors to combo box
DEFINE_HOOK(4E46BB, hWnd_PopulateWithColors, 7) {
	GET(HWND, hWnd, ESI);
	GET_STACK(int, idxSlot, 0x14);

	// add all colors
	int curSel = 0;
	for(int i=0; i<Ares::UISettings::ColorCount; ++i) {
		Interface::ColorData* pSlot = &Ares::UISettings::Colors[i+1];
		bool isCurrent = pSlot->selectedIndex == idxSlot;

		if(isCurrent || pSlot->selectedIndex == -1) {
			int idx = SendMessageA(hWnd, WW_CB_ADDITEM, 0, 0x822B78);
			SendMessageA(hWnd, WW_SETCOLOR, idx, pSlot->colorRGB);
			SendMessageA(hWnd, CB_SETITEMDATA, idx, i);

			if(isCurrent) {
				curSel = idx;
			}
		}
	}

	SendMessageA(hWnd, CB_SETCURSEL, curSel, 0);
	SendMessageA(hWnd, 0x4F1, 0, 0);

	return 0x4E4749;
}

// update the color in the combo drop-down lists
DEFINE_HOOK(4E4A41, hWnd_DrawColors_A, 7) {
	GET(int, curSel, EAX);

	int idx = -1;
	for(int i=0; i<Ares::UISettings::ColorCount; ++i) {
		if(Ares::UISettings::Colors[i+1].selectedIndex == curSel) {
			idx = i;
			break;
		}
	}

	if(idx != -1) {
		Ares::UISettings::Colors[idx+1].selectedIndex = -1;
	}

	return 0x4E4A6D;
}

DEFINE_HOOK(4E4B47, hWnd_DrawColors_B, 7) {
	GET(int, idx, EBP);
	GET(int, value, ESI);

	Ares::UISettings::Colors[idx+1].selectedIndex = value;

	return 0x4E4B4E;
}

DEFINE_HOOK(4E4556, hWnd_GetSlotColorIndex, 7) {
	GET(int, slot, ECX);

	int ret = -1;
	for(int i=0; i<Ares::UISettings::ColorCount; ++i) {
		if(Ares::UISettings::Colors[i+1].selectedIndex == slot) {
			ret = i+1;
			break;
		}
	}

	R->EAX(ret);
	return 0x4E4570;
}

DEFINE_HOOK(4E4580, hWnd_IsAvailableColor, 5) {
	GET(int, slot, ECX);

	R->AL(Ares::UISettings::Colors[slot+1].selectedIndex == -1);
	return 0x4E4592;
}

DEFINE_HOOK(4E4C9D, hWnd_WhateverColors_A, 7) {
	GET(int, curSel, EAX);

	int idx = -1;
	for(int i=0; i<Ares::UISettings::ColorCount; ++i) {
		if(Ares::UISettings::Colors[i+1].selectedIndex == curSel) {
			idx = i;
			break;
		}
	}
  
	if(idx != -1) {
		Ares::UISettings::Colors[idx+1].selectedIndex = -1;
	}

	return 0x4E4CC9;
}

DEFINE_HOOK(4E4D67, hWnd_WhateverColors_B, 7) {
	GET(int, idx, EAX);
	GET(int, value, ESI);

	Ares::UISettings::Colors[idx+1].selectedIndex = value;

	return 0x4E4D6E;
}

DEFINE_HOOK(60FAD7, Ownerdraw_PostProcessColors, A) {
	// copy original instruction
	*(int*)0xAC1B90 = 0x443716;

	// update colors
	*(int*)0xAC18A4 = Ares::UISettings::uiColorText;
	*(int*)0xAC184C = Ares::UISettings::uiColorCaret;
	*(int*)0xAC4604 = Ares::UISettings::uiColorSelection;
	*(int*)0xAC1B98 = Ares::UISettings::uiColorBorder1;
	*(int*)0xAC1B94 = Ares::UISettings::uiColorBorder2;
	*(int*)0xAC1AF8 = Ares::UISettings::uiColorDisabledObserver;
	*(int*)0xAC1CB0 = Ares::UISettings::uiColorTextObserver;
	*(int*)0xAC4880 = Ares::UISettings::uiColorSelectionObserver;
	*(int*)0xAC1CB4 = Ares::UISettings::uiColorDisabled;

	// skip initialization
	bool inited = *(bool*)0xAC48D4;
	return inited ? 0x60FB5D : 0x60FAE3;
}

DEFINE_HOOK(612DA9, Handle_Button_Messages_Color, 6) {
	R->EDI(Ares::UISettings::uiColorTextButton);
	return 0x612DAF;
}

DEFINE_HOOK(613072, Handle_Button_Messages_DisabledColor, 7) {
	R->EDI(Ares::UISettings::uiColorDisabledButton);
	return 0x613138;
}

DEFINE_HOOK(61664C, Handle_Checkbox_Messages_Color, 5) {
	R->EAX(Ares::UISettings::uiColorTextCheckbox);
	return 0x616651;
}

DEFINE_HOOK(616655, Handle_Checkbox_Messages_Disabled, 5) {
	R->EAX(Ares::UISettings::uiColorDisabledCheckbox);
	return 0x61665A;
}

DEFINE_HOOK(616AF0, Handle_RadioButton_Messages_Color, 6) {
	R->ECX(Ares::UISettings::uiColorTextRadio);
	return 0x616AF6;
}

DEFINE_HOOK(615DF7, Handle_Static_Messages_Color, 6) {
	R->ECX(Ares::UISettings::uiColorTextLabel);
	return 0x615DFD;
}

DEFINE_HOOK(615AB7, Handle_Static_Messages_Disabled, 6) {
	R->ECX(Ares::UISettings::uiColorDisabledLabel);
	return 0x615ABD;
}

DEFINE_HOOK(619A4F, Handle_Listbox_Messages_Color, 6) {
	R->ESI(Ares::UISettings::uiColorTextList);
	return 0x619A55;
}

DEFINE_HOOK(6198D3, Handle_Listbox_Messages_DisabledA, 6) {
	R->EBX(Ares::UISettings::uiColorDisabledList);
	return 0x6198D9;
}

DEFINE_HOOK(619A5F, Handle_Listbox_Messages_DisabledB, 6) {
	R->ESI(Ares::UISettings::uiColorDisabledList);
	return 0x619A65;
}

DEFINE_HOOK(619270, Handle_Listbox_Messages_SelectionA, 5) {
	R->EAX(Ares::UISettings::uiColorSelectionList);
	return 0x619275;
}

DEFINE_HOOK(619288, Handle_Listbox_Messages_SelectionB, 6) {
	R->DL(Ares::UISettings::uiColorSelectionList >> 16);
	return 0x61928E;
}

DEFINE_HOOK(617A2B, Handle_Combobox_Messages_Color, 6) {
	R->EBX(Ares::UISettings::uiColorTextCombobox);
	return 0x617A31;
}

DEFINE_HOOK(617A57, Handle_Combobox_Messages_Disabled, 6) {
	R->EBX(Ares::UISettings::uiColorDisabledCombobox);
	return 0x617A5D;
}

DEFINE_HOOK(60DDA6, Handle_Combobox_Dropdown_Messages_SelectionA, 5) {
	R->EAX(Ares::UISettings::uiColorSelectionCombobox);
	return 0x60DDAB;
}

DEFINE_HOOK(60DDB6, Handle_Combobox_Dropdown_Messages_SelectionB, 6) {
	R->DL(Ares::UISettings::uiColorSelectionCombobox >> 16);
	return 0x60DDBC;
}

DEFINE_HOOK(61E2A5, Handle_Slider_Messages_Color, 5) {
	R->EAX(Ares::UISettings::uiColorTextSlider);
	return 0x61E2AA;
}

DEFINE_HOOK(61E2B1, Handle_Slider_Messages_Disabled, 5) {
	R->EAX(Ares::UISettings::uiColorDisabledSlider);
	return 0x61E2B6;
}

DEFINE_HOOK(61E8A0, Handle_GroupBox_Messages_Color, 6) {
	R->ECX(Ares::UISettings::uiColorTextGroupbox);
	return 0x61E8A6;
}

DEFINE_HOOK(614FF2, Handle_NewEdit_Messages_Color, 6) {
	R->EDX(Ares::UISettings::uiColorTextEdit);
	return 0x614FF8;
}

DEFINE_HOOK(69B949, Game_ProcessRandomPlayers_ColorsA, 6) {
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, Ares::UISettings::ColorCount - 1));
	return 0x69B95E;
}

DEFINE_HOOK(69BA13, Game_ProcessRandomPlayers_ColorsB, 6) {
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, Ares::UISettings::ColorCount - 1));
	return 0x69BA28;
}

DEFINE_HOOK(69B69B, GameModeClass_PickRandomColor_Unlimited, 6) {
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, Ares::UISettings::ColorCount - 1));
	return 0x69B6AF;
}

DEFINE_HOOK(69B7FF, GameModeClass_SetColor_Unlimited, 6) {
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, Ares::UISettings::ColorCount - 1));
	return 0x69B813;
}