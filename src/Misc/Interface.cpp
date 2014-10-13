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

using namespace DialogConstants;

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
		using namespace CampaignDialog;

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
			if(HWND hItem = GetDlgItem(hDlg, CampaignList)) {
				// extensive stuff
				show(CampaignList);
				show(LoadButton);
				hide(AlliedImage);
				hide(SovietImage);
				hide(ThirdImage);
				hide(FourthImage);

				// use the position of the Allied button to place the
				// new campaign selection list.
				RECT rcItem = {125, 34, 125 + 174, 34 + 87};
				if(HWND hAllImage = GetDlgItem(hDlg, AlliedImage)) {
					GetWindowRect(hAllImage, &rcItem);
				}
				offset(AlliedLabel, 0, -rcItem.bottom + rcItem.top);

				// center the list above the difficulty selection. the list may
				// contain seven items, after that, a scroll bar will appear.
				// acount for its width, too.
				int offList = (CampaignExt::CountVisible() < 8) ? -2 : -12;
				OffsetRect(&rcItem, offList, 32);
				moveItem(hItem, rcItem, ptDlg);
			
				// let the Allied label be the caption
				if(HWND hAllLabel = GetDlgItem(hDlg, AlliedLabel)) {
					SendMessageA(hAllLabel, 0x4B2, 0, reinterpret_cast<LPARAM>(StringTable::LoadStringA("GUI:SelectCampaign")));
				}

				// call the load button "Play"
				if(HWND hLoad = GetDlgItem(hDlg, LoadButton)) {
					SendMessageA(hLoad, 0x4B2, 0, reinterpret_cast<LPARAM>(StringTable::LoadStringA("GUI:PlayMission")));
				}

				// move the soviet label to a new location and reuse
				// it to show the selected campaigns summary.
				if(HWND hSovImage = GetDlgItem(hDlg, SovietImage)) {
					GetWindowRect(hSovImage, &rcItem);
					if(HWND hSovLabel = GetDlgItem(hDlg, SovietLabel)) {
						// remove default text and move label
						SendMessageA(hSovLabel, 0x4B2, 0, reinterpret_cast<LPARAM>(L""));
						moveItem(hSovLabel, rcItem, ptDlg);
					
						// left align text
						auto style = GetWindowLong(hSovLabel, GWL_STYLE);
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
			if(HWND hAllImage = GetDlgItem(hDlg, AlliedImage)) {
				GetWindowRect(hAllImage, &rcItem);
			}
			SIZE szImage;
			szImage.cx = static_cast<int>((rcItem.right - rcItem.left) * .8);
			szImage.cy = static_cast<int>((rcItem.bottom - rcItem.top) * 1.0);

			// the soviet image's top is used for the second row
			RECT rcSovImage = {0, 216, 0, 0};
			if(HWND hSovImage = GetDlgItem(hDlg, SovietImage)) {
				GetWindowRect(hSovImage, &rcSovImage);
			}
			int row2Offset = rcSovImage.top - rcItem.top;

			// call the load button "Play"
			if(HWND hLoad = GetDlgItem(hDlg, LoadButton)) {
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
			moveToPlace(AlliedImage, 0, 1, 2, 0, AlliedLabel);
			moveToPlace(SovietImage, 1, 0, 3, 0, SovietLabel);
			moveToPlace(ThirdImage, 2, 3, 4, 1, ThirdLabel);
			moveToPlace(FourthImage, 3, 2, 5, 1, FourthLabel);
		}
	}

	// main menu
	if(iID == 226) {
		using namespace MainDialog;

		static const Interface::MenuItem items[] = {
			{SinglePlayerButton, Ares::UISettings::SinglePlayerButton},
			{WestwoodOnlineButton, Ares::UISettings::WWOnlineButton},
			{NetworkButton, Ares::UISettings::NetworkButton},
			{MoviesAndCreditsButton, Ares::UISettings::MoviesAndCreditsButton},
			{OptionsButton, Ares::UISettings::UIAction::Default}};
		Interface::updateMenuItems(hDlg, items, _countof(items));
	}

	// singleplayer menu
	if(iID == 256) {
		using namespace SinglePlayerDialog;

		// swap skirmish and load buttons so load will not appear first
		if(Ares::UISettings::CampaignButton != Ares::UISettings::UIAction::Hide) {
			static const Interface::MenuItem items[] = {
				{NewCampaignButton, Ares::UISettings::CampaignButton},
				{LoadSavedGameButton, Ares::UISettings::UIAction::Default},
				{SkirmishButton, Ares::UISettings::SkirmishButton}};
			Interface::updateMenuItems(hDlg, items, _countof(items));
		} else {
			static const  Interface::MenuItem items[] = {
				{SkirmishButton, Ares::UISettings::SkirmishButton},
				{LoadSavedGameButton, Ares::UISettings::UIAction::Default},
				{NewCampaignButton, Ares::UISettings::CampaignButton}};
			Interface::swapItems(hDlg, NewCampaignButton, SkirmishButton);
			Interface::updateMenuItems(hDlg, items, _countof(items));
		}
	}

	// movies and credits menu
	if(iID == 257) {
		using namespace MoviesAndCreditsDialog;

		static const Interface::MenuItem items[] = {
			{SneakPeeksButton, Ares::UISettings::SneakPeeksButton},
			{PlayMoviesButton, Ares::UISettings::PlayMoviesButton},
			{ViewCreditsButton, Ares::UISettings::ViewCreditsButton}};
		Interface::updateMenuItems(hDlg, items, _countof(items));
	}

	// one-button message box
	if(iID == 206) {
		using namespace OneButtonMessageBox;

		// more room for text
		if(HWND hItem = GetDlgItem(hDlg, MessageText)) {
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
	using namespace CampaignDialog;
	if(iID == AlliedImage) {
		return 0;
	} else if (iID == SovietImage) {
		return 1;
	} else if (iID == ThirdImage) {
		return 2;
	} else if (iID == FourthImage) {
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
		if(pData && pData->IsVisible()) {
			auto newIndex = SendMessageA(hList, 0x4CD, 0, reinterpret_cast<LPARAM>(pData->OwnerObject()->Description));
			SendMessageA(hList, LB_SETITEMDATA, static_cast<WPARAM>(newIndex), i);
		}
	}

	// disable the play button as there is nothing selected. we don't select
	// the first campaign here so the user will get the introduction sound.
	if(HWND hItem = GetDlgItem(hDlg, CampaignDialog::LoadButton)) {
		EnableWindow(hItem, false);
	}

	return 0x52F07F;
}

// catch selecting a new campaign from the list
DEFINE_HOOK(52EC18, CampaignMenu_hDlg_PreHandleGeneral, 5) {
	GET(HWND, hDlg, ESI);
	GET(int, msg, EBX);
	GET(int, lParam, EBP);

	using namespace CampaignDialog;

	// catch the selection change event of the campaign list
	if(msg == WM_COMMAND) {
		int iID = LOWORD(lParam);
		int iCmd = HIWORD(lParam);
		if((iID == CampaignList) && (iCmd == LBN_SELCHANGE)) {
			auto index = SendDlgItemMessageA(hDlg, CampaignList, LB_GETCURSEL, 0, 0);
			int idxCampaign = SendDlgItemMessageA(hDlg, CampaignList, LB_GETITEMDATA, static_cast<WPARAM>(index), 0);

			if(CampaignExt::lastSelectedCampaign != idxCampaign) {
				// play the hover sound
				CampaignExt::ExtData* pData = CampaignExt::Array.GetItem(idxCampaign);
				if(pData) {
					int idxSound = VocClass::FindIndex(pData->HoverSound);
					if(idxSound > -1) {
						VocClass::PlayGlobal(idxSound, 8192, 1.0f);
					}

					// set the summary text
					if(HWND hSovLabel = GetDlgItem(hDlg, SovietLabel)) {
						const wchar_t* summary = pData->Summary.Get();
						SendMessageA(hSovLabel, 0x4B2, 0, reinterpret_cast<LPARAM>(summary));
					}
				}

				// cache the selected index
				CampaignExt::lastSelectedCampaign = idxCampaign;
			}

			// enable the play button
			if(HWND hItem = GetDlgItem(hDlg, LoadButton)) {
				EnableWindow(hItem, (index >= 0));
			}
		}
	}

	return 0;
}

// start the mission
DEFINE_HOOK(52ED21, CampaignMenu_hDlg_ClickedPlay, 9) {
	GET(HWND, hDlg, ESI);

	using namespace CampaignDialog;

	// find out which campaign is selected
	auto idxItem = SendDlgItemMessageA(hDlg, CampaignList, LB_GETCURSEL, 0, 0);
	int idxCampaign = SendDlgItemMessageA(hDlg, CampaignList, LB_GETITEMDATA, static_cast<WPARAM>(idxItem), 0);

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
				auto& SoundToPlay = *reinterpret_cast<int*>(0x825C20);
				SendDlgItemMessageA(hDlg, iID, 0x4D3, 0, 0);
				SoundToPlay = sound;

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

	return (Interface::getSlotIndex(reinterpret_cast<int>(iID)) > -1) ? 0x60A982u : 0x60A9EDu;
}

// animation duration
DEFINE_HOOK(60357E, CampaignMenu_SetAnimationDuration, 5) {
	GET(HWND, iID, EAX);

	return (Interface::getSlotIndex(reinterpret_cast<int>(iID)) > -1) ? 0x6035C5u : 0x6035E6u;
}

// initialize stuff like the order and images
DEFINE_HOOK(52F191, CampaignMenu_InitializeMoreButtons, 5) {
	GET(HWND, hDlg, ESI);

	using namespace CampaignDialog;

	if(!Ares::UISettings::CampaignList) {
		// register buttons as campaign buttons
		// (FourthImage being used twice here is no error)
		SendDlgItemMessageA(hDlg, FourthImage, 0x4D5u, 0, 0);
		SendDlgItemMessageA(hDlg, FourthImage, 0x4D4u, 0, 0);

		if(HWND hItem = GetDlgItem(hDlg, ThirdImage)) {
			PostMessageA(hItem, 0x4D7u, 0, reinterpret_cast<LPARAM>(hDlg));
		}

		if(HWND hItem = GetDlgItem(hDlg, FourthImage)) {
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
					if(HWND hItem = GetDlgItem(hDlg, AlliedLabel + i)) {
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

	using namespace GameOptionsDialog;

	for(int item=LoadGameButton; item<=DeleteGameButton; ++item) {
		if(HWND hItem = GetDlgItem(hDlg, item)) {
			EnableWindow(hItem, FALSE);
		}
	}

	return 0x4F1834;
}
