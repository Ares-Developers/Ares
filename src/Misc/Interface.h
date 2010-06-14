#ifndef INTERFACE_H
#define INTERFACE_H

#include <Windows.h>

class Interface
{
private:
	Interface(void);
	~Interface(void);

public:
	enum eUIAction {
		uia_Default = 0,
		uia_Message = 1,
		uia_Disable = 2,
		uia_Hide = 3,
		uia_SneakPeak = 13,
		uia_Credits = 15
	};

	struct MenuItem {
		int nIDDlgItem;
		eUIAction uiaAction;
	};

	static int lastDialogTemplateID;
	static int nextReturnMenu;
	static int nextAction;
	const static wchar_t* nextMessageText;

	static bool invokeClickAction(eUIAction, char*, int*, int);
	static void updateMenuItems(HWND, MenuItem*, int);
	static void updateMenu(HWND hDlg);
	static eUIAction parseUIAction(char*, eUIAction);

private:
	static void moveItem(HWND, RECT, POINT);
	static void swapItems(HWND, int, int);
};

#endif