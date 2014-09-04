#include "Interface.h"
#include "../Ares.h"
#include "../Ares.CRT.h"
#include "../Ext/Campaign/Body.h"
#include "../Utilities/Constructs.h"

#include <StringTable.h>
#include <VocClass.h>

int Interface::lastDialogTemplateID = 0;
int Interface::nextAction = -1;
int Interface::nextReturnMenu = -1;
const wchar_t* Interface::nextMessageText = nullptr;

int Interface::slots[4]; // holds index-of-campaign+1

//! Updates the dialogs after creation to better fit the user's needs.
/*!
	This function does dialog dependent stuff. Some controls are moved,
	or disabled, new Ares controls are hidden if not explicitly enabled.

	\param hDlg The dialog to update.
	
	\author AlexB
	\date 2010-06-20
*/
void Interface::updateMenu(HWND hDlg, int iID) {

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
					SendMessageA(hAllLabel, 0x4B2, 0, reinterpret_cast<LPARAM>(StringTable::LoadStringA("GUI:SelectCampaign")));
				}

				// call the load button "Play"
				if(HWND hLoad = GetDlgItem(hDlg, 1038)) {
					SendMessageA(hLoad, 0x4B2, 0, reinterpret_cast<LPARAM>(StringTable::LoadStringA("GUI:PlayMission")));
				}

				// move the soviet label to a new location and reuse
				// it to show the selected campaigns summary.
				if(HWND hSovImage = GetDlgItem(hDlg, 1772)) {
					GetWindowRect(hSovImage, &rcItem);
					if(HWND hSovLabel = GetDlgItem(hDlg, 1960)) {
						// remove default text and move label
						SendMessageA(hSovLabel, 0x4B2, 0, reinterpret_cast<LPARAM>(L""));
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
			szImage.cx = static_cast<int>((rcItem.right - rcItem.left) * .8);
			szImage.cy = static_cast<int>((rcItem.bottom - rcItem.top) * 1.0);

			// the soviet image's top is used for the second row
			RECT rcSovImage = {0, 216, 0, 0};
			if(HWND hSovImage = GetDlgItem(hDlg, 1772)) {
				GetWindowRect(hSovImage, &rcSovImage);
			}
			int row2Offset = rcSovImage.top - rcItem.top;

			// call the load button "Play"
			if(HWND hLoad = GetDlgItem(hDlg, 1038)) {
				SendMessageA(hLoad, 0x4B2, 0, reinterpret_cast<LPARAM>(StringTable::LoadStringA("GUI:PlayMission")));
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

			RECT rcSlots[7];
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
						int widen = static_cast<int>(slot < 2 ? (width - rcLabel.right + rcLabel.left) / 2 : 15);
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
	Interface::updateMenu(hDlg, Interface::lastDialogTemplateID);
	return 0;
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
			int newIndex = SendMessageA(hList, 0x4CD, 0, reinterpret_cast<WPARAM>(pData->AttachedToObject->Description));
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
						VocClass::PlayGlobal(idxSound, 8192, 1.0f);
					}

					// set the summary text
					if(HWND hSovLabel = GetDlgItem(hDlg, 1960)) {
						const wchar_t* summary = nullptr;
						if(*pData->Summary) {
							summary = StringTable::LoadStringA(pData->Summary);
						}
						SendMessageA(hSovLabel, 0x4B2, 0, reinterpret_cast<LPARAM>(summary));
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

			char* campaignID = nullptr;
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
				*reinterpret_cast<DWORD*>(0x825C20) = sound;

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
	char* campaignID = nullptr;
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
		if(idxCampaign > -1) {
			if(auto pConvert = Ares::UISettings::Campaigns[idxCampaign].Palette->GetConvert()) {
				R->EAX(pConvert);
				return 0x603798;
			}
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

	return ((Interface::getSlotIndex(reinterpret_cast<int>(iID)) > -1) ? 0x60A982 : 0x60A9ED);
}

// animation duration
DEFINE_HOOK(60357E, CampaignMenu_SetAnimationDuration, 5) {
	GET(HWND, iID, EAX);

	return ((Interface::getSlotIndex(reinterpret_cast<int>(iID)) > -1) ? 0x6035C5 : 0x6035E6);
}

// initialize stuff like the order and images
DEFINE_HOOK(52F191, CampaignMenu_InitializeMoreButtons, 5) {
	GET(HWND, hDlg, ESI);

	if(!Ares::UISettings::CampaignList) {
		// register buttons as campaign buttons
		SendDlgItemMessageA(hDlg, 1773, 0x4D5u, 0, 0);
		SendDlgItemMessageA(hDlg, 1773, 0x4D4u, 0, 0);

		if(HWND hItem = GetDlgItem(hDlg, 1771)) {
			PostMessageA(hItem, 0x4D7u, 0, reinterpret_cast<LPARAM>(hDlg));
		}

		if(HWND hItem = GetDlgItem(hDlg, 1773)) {
			PostMessageA(hItem, 0x4D7u, 0, reinterpret_cast<LPARAM>(hDlg));
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
			if(Interface::slots[i] > 0) {
				int idxCampaign = Interface::slots[i] - 1;
				if(!Ares::UISettings::Campaigns[idxCampaign].Valid) {
					// disable slot
					Interface::slots[i] = 0;
				} else {
					// update the subline text
					if(HWND hItem = GetDlgItem(hDlg, i + 1959)) {
						SendMessageA(hItem, 0x4B2u, 0, reinterpret_cast<LPARAM>(StringTable::LoadStringA(Ares::UISettings::Campaigns[idxCampaign].Subline)));
					}
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

// disable the loading button in the single player menu
DEFINE_HOOK(52D6C2, Singleplayer_hDlg_DisableSaves, A)
{
	R->ECX(FALSE);
	return 0x52D6CC;
}

// disable load, save and delete buttons on the ingame menu
DEFINE_HOOK(4F17F6, sub_4F1720_DisableSaves, 6)
{
	GET(HWND, hDlg, EBP);

	for(int item=1310; item<=1312; ++item) {
		if(HWND hItem = GetDlgItem(hDlg, item)) {
			EnableWindow(hItem, FALSE);
		}
	}

	return 0x4F1834;
}
