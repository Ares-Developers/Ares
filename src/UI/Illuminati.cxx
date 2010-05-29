#include <windows.h>
#include <CommCtrl.h>

#include <MouseClass.h>
#include <ScenarioClass.h>
#include <Unsorted.h>

#include "../Misc/Debug.h"
#include "Dialogs.h"
#include "registered.h"

BOOL CALLBACK Dialogs::Illuminati::WndProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	BOOL result = UI::StandardWndProc(hwndDlg, message, wParam, lParam);
	if(!result) {
		switch(message) {
		case WM_INITDIALOG:
			Init(hwndDlg);
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDCANCEL:
				case ARES_BTN_YES:
					Apply(hwndDlg, 1);
					break;
				case ARES_BTN_NO:
					Apply(hwndDlg, 0);
					break;
			}
			break;
		case WM_CLOSE:
			Apply(hwndDlg, 1);
			break;
		}
	}
	return result;
}

void Dialogs::Illuminati::Init(HWND hwndDlg) {
	if(HWND Slider = Imports::GetDlgItem(hwndDlg, ARES_ILLUMINATI_SLIDER_R)) {
		Imports::SendMessageA(Slider, TBM_SETRANGE, 1, 100 * 0x10000);
		Imports::SendMessageA(Slider, TBM_SETPOS, 1, ScenarioClass::Instance->Red);
	}

	if(HWND Slider = Imports::GetDlgItem(hwndDlg, ARES_ILLUMINATI_SLIDER_G)) {
		Imports::SendMessageA(Slider, TBM_SETRANGE, 1, 100 * 0x10000);
		Imports::SendMessageA(Slider, TBM_SETPOS, 1, ScenarioClass::Instance->Green);
	}

	if(HWND Slider = Imports::GetDlgItem(hwndDlg, ARES_ILLUMINATI_SLIDER_B)) {
		Imports::SendMessageA(Slider, TBM_SETRANGE, 1, 100 * 0x10000);
		Imports::SendMessageA(Slider, TBM_SETPOS, 1, ScenarioClass::Instance->Blue);
	}

	if(HWND Slider = Imports::GetDlgItem(hwndDlg, ARES_ILLUMINATI_SLIDER_AMBIENT)) {
		Imports::SendMessageA(Slider, TBM_SETRANGE, 1, 100 * 0x10000);
		Imports::SendMessageA(Slider, TBM_SETPOS, 1, ScenarioClass::Instance->AmbientCurrent);
	}

	if(HWND Slider = Imports::GetDlgItem(hwndDlg, ARES_ILLUMINATI_SLIDER_GROUND)) {
		Imports::SendMessageA(Slider, TBM_SETRANGE, 1, 1000 * 0x10000);
		Imports::SendMessageA(Slider, TBM_SETPOS, 1, ScenarioClass::Instance->Ground);
	}

	if(HWND Slider = Imports::GetDlgItem(hwndDlg, ARES_ILLUMINATI_SLIDER_LEVEL)) {
		Imports::SendMessageA(Slider, TBM_SETRANGE, 1, 1000 * 0x10000);
		Imports::SendMessageA(Slider, TBM_SETPOS, 1, ScenarioClass::Instance->Level);
	}
}

void Dialogs::Illuminati::Apply(HWND hwndDlg, bool Close) {
	if(HWND Slider = Imports::GetDlgItem(hwndDlg, ARES_ILLUMINATI_SLIDER_R)) {
		ScenarioClass::Instance->Red = Imports::SendMessageA(Slider, TBM_GETPOS, 0, 0);
	}

	if(HWND Slider = Imports::GetDlgItem(hwndDlg, ARES_ILLUMINATI_SLIDER_G)) {
		ScenarioClass::Instance->Green = Imports::SendMessageA(Slider, TBM_GETPOS, 0, 0);
	}

	if(HWND Slider = Imports::GetDlgItem(hwndDlg, ARES_ILLUMINATI_SLIDER_B)) {
		ScenarioClass::Instance->Blue = Imports::SendMessageA(Slider, TBM_GETPOS, 0, 0);
	}

	if(HWND Slider = Imports::GetDlgItem(hwndDlg, ARES_ILLUMINATI_SLIDER_AMBIENT)) {
		ScenarioClass::Instance->AmbientTarget = ScenarioClass::Instance->AmbientCurrent = Imports::SendMessageA(Slider, TBM_GETPOS, 0, 0);
	}

	if(HWND Slider = Imports::GetDlgItem(hwndDlg, ARES_ILLUMINATI_SLIDER_GROUND)) {
		ScenarioClass::Instance->Ground = Imports::SendMessageA(Slider, TBM_GETPOS, 0, 0);
	}

	if(HWND Slider = Imports::GetDlgItem(hwndDlg, ARES_ILLUMINATI_SLIDER_LEVEL)) {
		ScenarioClass::Instance->Level = Imports::SendMessageA(Slider, TBM_GETPOS, 0, 0);
	}

	ScenarioClass::RecalcLighting(10 * ScenarioClass::Instance->Red, 10 * ScenarioClass::Instance->Green, 10 * ScenarioClass::Instance->Blue, 0);
	MouseClass::Instance->DrawOnTop();
	if(Close) {
		LONG status = Imports::GetWindowLongA(hwndDlg, 8);
		LONG *statusPtr = (LONG *)status;
		*statusPtr = 1;
	}
}
