#include "../Ares.h"

#include <ColorScheme.h>
#include <ScenarioClass.h>
#include <SessionClass.h>
#include <StringTable.h>

// reset the colors
DEFINE_HOOK(4E43C0, Game_InitDropdownColors, 5)
{
	// mark all colors as unused (+1 for the  observer)
	for(auto i = 0; i < Ares::UISettings::ColorCount + 1; ++i) {
		Ares::UISettings::Colors[i].selectedIndex = -1;
	}

	return 0;
}

// convert player color slot index to color scheme index
DEFINE_HOOK(69A310, SessionClass_GetPlayerColorScheme, 7) {
	GET_STACK(int const, idx, 0x4);

	// get the slot
	Ares::UISettings::ColorData* slot = nullptr;
	if(idx == -2 || idx == Ares::UISettings::ColorCount) {
		// observer color
		slot = &Ares::UISettings::Colors[0];
	} else if(idx < Ares::UISettings::ColorCount) {
		// house color
		slot = &Ares::UISettings::Colors[idx + 1];
	}

	// retrieve the color scheme index
	auto ret = 0;
	if(slot) {
		if(slot->colorSchemeIndex == -1) {
			auto const pScheme = slot->colorScheme;
			slot->colorSchemeIndex = ColorScheme::FindIndex(pScheme);
			if(slot->colorSchemeIndex == -1) {
				Debug::Log("Color scheme \"%s\" not found.\n", pScheme);
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
	GET(int const, idxColor, ECX);

	const wchar_t* ret = nullptr;
	if(idxColor == -2) {
		// random
		ret = StringTable::LoadString("STT:PlayerColorRandom");
	} else if(idxColor <= Ares::UISettings::ColorCount) {
		// houses and observer
		auto const index = (idxColor + 1) % (Ares::UISettings::ColorCount + 1);
		ret = Ares::UISettings::Colors[index].sttToolTipSublineText;
	}

	R->EAX(ret);
	return 0x4E43B9;
}

// handle adding colors to combo box
DEFINE_HOOK(4E46BB, hWnd_PopulateWithColors, 7) {
	GET(HWND const, hWnd, ESI);
	GET_STACK(int const, idxPlayer, 0x14);

	// add all colors
	auto curSel = 0;
	for(auto i = 0; i < Ares::UISettings::ColorCount; ++i) {
		auto const& Color = Ares::UISettings::Colors[i + 1];
		auto const isCurrent = Color.selectedIndex == idxPlayer;

		if(isCurrent || Color.selectedIndex == -1) {
			int idx = SendMessageA(hWnd, WW_CB_ADDITEM, 0, 0x822B78);
			SendMessageA(hWnd, WW_SETCOLOR, idx, Color.colorRGB);
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
DEFINE_HOOK(4E4A41, hWnd_SetPlayerColor_A, 7) {
	GET(int const, idxPlayer, EAX);

	auto const count = Ares::UISettings::ColorCount;
	for(auto i = 0; i < count; ++i) {
		auto& Color = Ares::UISettings::Colors[i + 1];
		if(Color.selectedIndex == idxPlayer) {
			Color.selectedIndex = -1;
			break;
		}
	}

	return 0x4E4A6D;
}

DEFINE_HOOK(4E4B47, hWnd_SetPlayerColor_B, 7) {
	GET(int const, idxColor, EBP);
	GET(int const, idxPlayer, ESI);

	Ares::UISettings::Colors[idxColor + 1].selectedIndex = idxPlayer;

	return 0x4E4B4E;
}

DEFINE_HOOK(4E4556, hWnd_GetSlotColorIndex, 7) {
	GET(int const, idxPlayer, ECX);

	auto ret = -1;
	auto const count = Ares::UISettings::ColorCount;
	for(auto i = 0; i < count; ++i) {
		auto const& Color = Ares::UISettings::Colors[i + 1];
		if(Color.selectedIndex == idxPlayer) {
			ret = i + 1;
			break;
		}
	}

	R->EAX(ret);
	return 0x4E4570;
}

DEFINE_HOOK(4E4580, hWnd_IsAvailableColor, 5) {
	GET(int const, idxColor, ECX);

	R->AL(Ares::UISettings::Colors[idxColor + 1].selectedIndex == -1);
	return 0x4E4592;
}

DEFINE_HOOK(4E4C9D, hWnd_UpdatePlayerColors_A, 7) {
	GET(int const, idxPlayer, EAX);

	// check players and reset used color for this player
	for(auto i = 0; i < Ares::UISettings::ColorCount; ++i) {
		auto& Color = Ares::UISettings::Colors[i + 1];
		if(Color.selectedIndex == idxPlayer) {
			Color.selectedIndex = -1;
			break;
		}
	}

	return 0x4E4CC9;
}

DEFINE_HOOK(4E4D67, hWnd_UpdatePlayerColors_B, 7) {
	GET(int const, idxColor, EAX);
	GET(int const, idxPlayer, ESI);

	// reserve the color for a player. skip the observer
	Ares::UISettings::Colors[idxColor + 1].selectedIndex = idxPlayer;

	return 0x4E4D6E;
}

DEFINE_HOOK(69B97D, Game_ProcessRandomPlayers_ObserverColor, 7)
{
	GET(NodeNameType* const, pStartingSpot, ESI);

	// observer uses last color, beyond the actual colors
	pStartingSpot->Color = Ares::UISettings::ColorCount;

	return 0x69B984;
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

DEFINE_HOOK(69B7FF, Session_SetColor_Unlimited, 6) {
	R->EAX(ScenarioClass::Instance->Random.RandomRanged(0, Ares::UISettings::ColorCount - 1));
	return 0x69B813;
}
