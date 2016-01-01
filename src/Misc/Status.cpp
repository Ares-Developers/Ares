#include "../Ares.version.h"
#include "Status.h"

#ifdef IS_RELEASE_VER
#define RELEASE 1
#else
#define RELEASE 0
#endif

Point2D StatusMessages::TLPoint = { 10, 340 };
Point2D StatusMessages::Delta = { 0, 20 };

bool StatusMessages::Visible = false; // changed per Marshall's request, TODO INI flag

DEFINE_HOOK(686B4F, Scenario_SetMessagePosition, 6)
{
	GET_STACK(byte, Is_SP, 0x16);
	StatusMessages::TLPoint.X = 10;
	if(Is_SP) {
		StatusMessages::TLPoint.Y = 120;
	} else {
		StatusMessages::TLPoint.Y = 340;
	}
	return 0;
}

DEFINE_HOOK(68758D, Scenario_Start0, 5)
{
	StatusMessages::Add(L"First pass at Swizzling ...", COLOR_WHITE);
	return 0;
}

DEFINE_HOOK(6875F3, Scenario_Start1, 6)
{
	StatusMessages::Add(L"Initializing Tactical display ...", COLOR_WHITE);
	return 0;
}

DEFINE_HOOK(687748, Scenario_Start2, 6)
{
	StatusMessages::Add(L"Overriding rules with scenario ...", COLOR_WHITE);
	return 0;
}

DEFINE_HOOK(68797A, Scenario_Start3, 5)
{
	StatusMessages::Add(L"Overriding AI with scenario ...", COLOR_WHITE);
	return 0;
}

DEFINE_HOOK(6879F9, Scenario_Start4, 5)
{
	StatusMessages::Add(L"Loading Map ...", COLOR_WHITE);
	return 0;
}

DEFINE_HOOK(687643, Scenario_Start5, 6)
{
	StatusMessages::Add(L"Initializing Theater...", COLOR_WHITE);
	return 0;
}

DEFINE_HOOK(6876A0, Scenario_Start6, 5)
{
	StatusMessages::Add(L"Reading rules, langrule and gamemode controls...", COLOR_WHITE);
	return 0;
}

DEFINE_HOOK(687A8F, Scenario_Start7, 5)
{
	StatusMessages::Add(L"Placing objects onto map ...", COLOR_WHITE);
	return 0;
}

DEFINE_HOOK(687B21, Scenario_Start8, 5)
{
	StatusMessages::Add(L"Overriding rules with TMCJ4F if needed ...", COLOR_WHITE);
	return 0;
}

DEFINE_HOOK(531413, Game_Start, 5)
{
	int topActive = RELEASE ? 500 : 460;

	DSurface::Hidden->DrawText(L"Ares is active.", 10, topActive, COLOR_GREEN);
#if !RELEASE
	DSurface::Hidden->DrawText(L"This is a testing version, NOT a final product.", 20, 480, COLOR_RED);
	DSurface::Hidden->DrawText(L"Bugs are to be expected.", 20, 500, COLOR_RED);
#endif
	DSurface::Hidden->DrawText(L"Ares is © The Ares Contributors 2007 - 2016.", 10, 520, COLOR_GREEN);

	wchar_t wVersion[256];
	wsprintfW(wVersion, L"%hs", DISPLAY_STRVER);

	DSurface::Hidden->DrawText(wVersion, 10, 540, COLOR_RED | COLOR_GREEN);
	return 0;
}

DEFINE_HOOK(74FDC0, GetModuleVersion, 5)
{
	R->EAX<const char *>(VERSION_INTERNAL);
	return 0x74FEEF;
}

DEFINE_HOOK(74FAE0, GetModuleInternalVersion, 5)
{
	R->EAX<const char *>(DISPLAY_STRMINI);
	return 0x74FC7B;
}

DEFINE_HOOK(532017, DlgProc_MainMenu_Version, 5)
{
	GET(HWND, hWnd, ESI);

	// account for longer version numbers
	const int MinimumWidth = 168;

	RECT Rect;
	if(GetWindowRect(hWnd, &Rect)) {
		int width = Rect.right - Rect.left;

		if(width < MinimumWidth) {
			// extend to the left by the difference
			Rect.left -= (MinimumWidth - width);

			// if moved out of screen, move right by this amount
			if(Rect.left < 0) {
				Rect.right += -Rect.left;
				Rect.left = 0;
			}

			MoveWindow(hWnd, Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, FALSE);
		}
	}

	return 0;
}

// end the loading screen as early as possible, ignoring the delay.
DEFINE_HOOK(52CA37, InitGame_Delay, 5)
{
	return 0x52CA65;
}
