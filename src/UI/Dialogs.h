#pragma once

#include "registered.h"
#include <WWMouseClass.h>
#include <UI.h>

class Dialogs {
public:

	static const char *StatusString;
	static const int ExceptControlID;

	static void TakeMouse() {
		WWMouseClass::Instance->ReleaseMouse();
		Imports::ShowCursor(1);
	}

	static void ReturnMouse() {
		Imports::ShowCursor(0);
		WWMouseClass::Instance->CaptureMouse();
	}
};
