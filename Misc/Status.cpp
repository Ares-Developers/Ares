#include "..\Ares.version.h"
#include <Drawing.h>

DEFINE_HOOK(6876A0, Scenario_Start0, 5)
{
	Drawing::DSurface_Hidden->DrawText(L"Loading scenario...", 10, 300, COLOR_GREEN);
	return 0;
}

DEFINE_HOOK(687748, Scenario_Start1, 6)
{
	Drawing::DSurface_Hidden->DrawText(L"Initializing Rules...", 10, 340, COLOR_WHITE);
	return 0;
}

DEFINE_HOOK(68797A, Scenario_Start2, 5)
{
	Drawing::DSurface_Hidden->DrawText(L"Parsing Rules...", 10, 380, COLOR_WHITE);
	return 0;
}

DEFINE_HOOK(68797A, Scenario_Start3, 5)
{
	Drawing::DSurface_Hidden->DrawText(L"Parsing AI...", 10, 380, COLOR_GREEN);
	return 0;
}

DEFINE_HOOK(6879F9, Scenario_Start4, 5)
{
	Drawing::DSurface_Hidden->DrawText(L"Parsing Map...", 10, 380, COLOR_WHITE);
	return 0;
}

DEFINE_HOOK(531413, Game_Start, 5)
{
	Drawing::DSurface_Hidden->DrawText(L"Ares is active.", 10, 460, COLOR_GREEN);
	Drawing::DSurface_Hidden->DrawText(L"This is a testing version, NOT a final product.", 20, 480, COLOR_RED);
	Drawing::DSurface_Hidden->DrawText(L"Bugs are to be expected.", 20, 500, COLOR_RED);
	Drawing::DSurface_Hidden->DrawText(L"Ares is © pd, DCoder and Electro 2007 - 2009.", 10, 520, COLOR_GREEN);

	wchar_t wVersion[256];
	wsprintfW(wVersion, L"%hs", VERSION_STRVER);

	Drawing::DSurface_Hidden->DrawText(wVersion, 10, 540, COLOR_RED | COLOR_GREEN);
	return 0;
}
