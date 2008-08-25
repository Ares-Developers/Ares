#include <YRPP.h>
#include "Ares.h"

EXPORT_FUNC(Scenario_Start0)
{
	Ares::Log("Start 0\n");
	Drawing::GetSurface_Hidden()->DrawText(L"Loading scenario...", 10, 300, COLOR_GREEN);
	return 0;
}

EXPORT_FUNC(Scenario_Start1)
{
	Ares::Log("Start 1\n");
	Drawing::GetSurface_Hidden()->DrawText(L"Initializing Rules...", 10, 340, COLOR_WHITE);
	return 0;
}

EXPORT_FUNC(Scenario_Start2)
{
	Ares::Log("Start 2\n");
	Drawing::GetSurface_Hidden()->DrawText(L"Parsing Rules...", 10, 380, COLOR_WHITE);
	return 0;
}

EXPORT_FUNC(Scenario_Start3)
{
	Ares::Log("Start 3\n");
	Drawing::GetSurface_Hidden()->DrawText(L"Parsing AI...", 10, 380, COLOR_GREEN);
	return 0;
}

EXPORT_FUNC(Scenario_Start4)
{
	Ares::Log("Start 4\n");
	Drawing::GetSurface_Hidden()->DrawText(L"Parsing Map...", 10, 380, COLOR_WHITE);
	return 0;
}

EXPORT_FUNC(Game_Start)
{
	Drawing::GetSurface_Hidden()->DrawText(L"Ares is active.", 10, 260, COLOR_WHITE);
	Drawing::GetSurface_Hidden()->DrawText(L"This is a testing version, NOT a final product.", 20, 290, COLOR_WHITE);
	Drawing::GetSurface_Hidden()->DrawText(L"Bugs are to be expected.", 20, 320, COLOR_WHITE);
	Drawing::GetSurface_Hidden()->DrawText(L"Ares is © pd and DCoder 2007 - 2008.", 10, 540, COLOR_GREEN);
	return 0;
}