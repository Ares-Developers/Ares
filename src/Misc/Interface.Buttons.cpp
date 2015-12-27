#include "Interface.h"
#include "../Ares.h"

#include <StringTable.h>
#include <MessageBox.h>

#include <Strsafe.h>

#include <array>

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
bool Interface::invokeClickAction(Ares::UISettings::UIAction action, const char* name, int* pResult, int nextMenu) {
	// reset
	nextAction = -1;
	nextReturnMenu = -1;
	nextMessageText = nullptr;

	auto ret = [pResult, nextMenu](int _nextAction) -> bool {
		*pResult = nextAction = _nextAction;
		nextReturnMenu = nextMenu;
		return (_nextAction > -1);
	};

	if(action == Ares::UISettings::UIAction::Message) {
		// generate the label name
		char buffer[0x20];
		StringCchPrintfA(buffer, 0x20, "TXT_%s_MSG", name);
		nextMessageText = StringTable::LoadStringA(buffer);

		// hide dialog temporarily and show a message box
		return ret(6);
	}

	if(action == Ares::UISettings::UIAction::SneakPeek) {
		// show the preview video
		return ret(13);
	}

	if(action == Ares::UISettings::UIAction::Credits) {
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
void Interface::updateMenuItems(HWND hDlg, const MenuItem* items, size_t count) {
	// account for dialog nc size
	POINT ptDlg = {0, 0};
	ScreenToClient(hDlg, &ptDlg);

	constexpr size_t const MaxMenuItemCount = 9;

	size_t VisibleButtons = 0;
	count = std::min(count, MaxMenuItemCount);
	std::array<RECT, MaxMenuItemCount> Rects;
	for(size_t i = 0; i < count; ++i) {
		if(HWND hItem = GetDlgItem(hDlg, items[i].nIDDlgItem)) {
			GetWindowRect(hItem, &Rects[i]);

			if(items[i].uiaAction == Ares::UISettings::UIAction::Hide) {
				// hide the window
				ShowWindow(hItem, SW_HIDE);
			} else {
				if(items[i].uiaAction == Ares::UISettings::UIAction::Disable) {
					// disable the button
					EnableWindow(hItem, false);
				}

				if(i != VisibleButtons) {
					// move the button to the next free position
					moveItem(hItem, Rects[VisibleButtons], ptDlg);
				}
				++VisibleButtons;
			}
		}
	}
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

//! Parses an eUIAction from a string.
/*!
	Converts the string to an eUIAction.

	\param value The string to parse.
	\param def The eUIAction returned for invalid values.

	\returns Parsed eUIAction if value is valid, def otherwise.

	\author AlexB
	\date 2010-06-20
*/
Ares::UISettings::UIAction Interface::parseUIAction(const char* value, Ares::UISettings::UIAction def) {
	if(!_strcmpi(value, "message")) {
		return Ares::UISettings::UIAction::Message;
	} else if(!_strcmpi(value, "disable")) {
		return Ares::UISettings::UIAction::Disable;
	} else if(!_strcmpi(value, "hide")) {
		return Ares::UISettings::UIAction::Hide;
	} else if(!_strcmpi(value, "credits")) {
		return Ares::UISettings::UIAction::Credits;
	} else if(!_strcmpi(value, "sneakpeek")) {
		return Ares::UISettings::UIAction::SneakPeek;
	} else if(_strcmpi(value, "default")) {
		Debug::Log(Debug::Severity::Error, "Unrecognized UI action value: %s\n", value);
		Debug::RegisterParserError();
	}
	return def;
}

// show message
DEFINE_HOOK(52DDBA, Frontend_WndProc_MessageBox, 5) {
	if(Interface::nextMessageText) {
		const wchar_t* ok = StringTable::LoadStringA("TXT_OK");
		MessageBox::Show(Interface::nextMessageText, ok, nullptr);
		Interface::nextMessageText = nullptr;
		return 0x52DE39;
	}

	auto const quick = Ares::UISettings::QuickExit;
	return quick ? 0x52DE25u : 0u;
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
