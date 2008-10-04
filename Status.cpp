#include <YRPP.h>
#include "Ares.Version.h"

EXPORT_FUNC(Scenario_Start0)
{
	Drawing::GetSurface_Hidden()->DrawText(L"Loading scenario...", 10, 300, COLOR_GREEN);
	return 0;
}

EXPORT_FUNC(Scenario_Start1)
{
	Drawing::GetSurface_Hidden()->DrawText(L"Initializing Rules...", 10, 340, COLOR_WHITE);
	return 0;
}

EXPORT_FUNC(Scenario_Start2)
{
	Drawing::GetSurface_Hidden()->DrawText(L"Parsing Rules...", 10, 380, COLOR_WHITE);
	return 0;
}

EXPORT_FUNC(Scenario_Start3)
{
	Drawing::GetSurface_Hidden()->DrawText(L"Parsing AI...", 10, 380, COLOR_GREEN);
	return 0;
}

EXPORT_FUNC(Scenario_Start4)
{
	Drawing::GetSurface_Hidden()->DrawText(L"Parsing Map...", 10, 380, COLOR_WHITE);
	return 0;
}

EXPORT_FUNC(Game_Start)
{
	Drawing::GetSurface_Hidden()->DrawText(L"Ares is active.", 10, 460, COLOR_GREEN);
	Drawing::GetSurface_Hidden()->DrawText(L"This is a testing version, NOT a final product.", 20, 480, COLOR_RED);
	Drawing::GetSurface_Hidden()->DrawText(L"Bugs are to be expected.", 20, 500, COLOR_RED);
	Drawing::GetSurface_Hidden()->DrawText(L"Ares is © pd, DCoder and Electro 2007 - 2008.", 10, 520, COLOR_GREEN);

	wchar_t wVersion[256];
	wsprintfW(wVersion, L"%hs", VERSION_STRVER);

	Drawing::GetSurface_Hidden()->DrawText(wVersion, 10, 540, COLOR_RED | COLOR_GREEN);
	return 0;
}

