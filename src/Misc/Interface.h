#ifndef INTERFACE_H
#define INTERFACE_H

#include "../Ares.h"

class Interface
{
private:
	Interface(void);
	~Interface(void);

public:
	struct MenuItem {
		int nIDDlgItem;
		Ares::UISettings::UIAction uiaAction;
	};

	static int lastDialogTemplateID;
	static int nextReturnMenu;
	static int nextAction;
	static const wchar_t* nextMessageText;

	static int slots[4];

	static bool invokeClickAction(Ares::UISettings::UIAction, const char*, int*, int);
	static void updateMenuItems(HWND hWnd, const MenuItem* items, size_t count);
	static void updateMenu(HWND hDlg, int iID);
	static Ares::UISettings::UIAction parseUIAction(const char*, Ares::UISettings::UIAction);
	static int getSlotIndex(int);

private:
	static void moveItem(HWND, RECT, POINT);
	static void swapItems(HWND, int, int);
};

#endif