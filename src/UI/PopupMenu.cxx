#include "PopupMenu.h"
#include <Unsorted.h>
#include <MouseClass.h>
#include <TechnoClass.h>
#include <WarheadTypeClass.h>
#include <GeneralDefinitions.h>
#include <Surface.h>
#include <Drawing.h>
#include "../Misc/Debug.h"

CoordStruct PopupMenu::LeptonCoords;
CellStruct PopupMenu::CellCoords;
Point2D PopupMenu::PixelCoords;
bool PopupMenu::Drawing = false;
RectangleStruct PopupMenu::MenuBox = {0, 0, 0, 0};
RectangleStruct PopupMenu::MenuItem = {0, 0, 0, 0};
short PopupMenu::HighlightedItem = -1;

DEFINE_HOOK(4AAD34, DisplayClass_RightMouseButtonUp, 6)
{
	enum {Yes = 0, No = 0x4AADA4 } DoNormal = Yes;
	Debug::Log("Right clicking\n");

	if(!PopupMenu::Drawing) {
		DoNormal = No;
		PopupMenu::Drawing = true;
	} else {
		PopupMenu::Drawing = false;
	}
	return DoNormal;
}

DEFINE_HOOK(4AB9B0, DisplayClass_LeftMouseButtonUp, 5)
{
	enum {Yes = 0, No = 0x4AC2A7 } DoNormal = Yes;
	Debug::Log("Left clicking\n");
	PopupMenu::Drawing = false;
	return DoNormal;
}

DEFINE_HOOK(4AAC7D, DisplayClass_MouseEvent, 8)
{
	GET_STACK(Point2D , XY, 0x1C);
	REF_STACK(const MouseEvent, MouseFlags, 0x34);

	if(MouseFlags & (MouseEvent::RightUp | MouseEvent::LeftUp)) {
//		PopupMenu::Coords = XY;
	}

	return 0;
}



DEFINE_HOOK(693263, MouseClass_ReactToClicks_LMBUp, 5)
{
	if(PopupMenu::Drawing) {
		if(PopupMenu::HighlightedItem != -1) {
			Debug::Log("Clicked #%d\n", PopupMenu::HighlightedItem);
			if(ObjectClass::CurrentObjects->Count >= 1) {
				if(TechnoClass *T = generic_cast<TechnoClass *>(ObjectClass::CurrentObjects->Items[0])) {
					int n = 45;
					switch(PopupMenu::HighlightedItem) {
						case 0:
							T->Deactivate();
							break;
						case 1:
							T->Flash(90);
							break;
						case 2:
							T->IronCurtain(90, T->Owner, false);
							break;
						case 3:
							T->ReceiveDamage(&n, 0, WarheadTypeClass::Find("NUKE"), nullptr, false, false, nullptr);
							break;
						case 4:
							T->QueueMission(Mission::Hunt, true);
							break;
					}
				}
			}
		}
		PopupMenu::Drawing = false;
	} else {

		REF_STACK(CellStruct, XY, STACK_OFFS(0x24, 0x1C));
		REF_STACK(CoordStruct, XYZ, STACK_OFFS(0x24, 0xC));
		REF_STACK(ObjectClass*, pTarget, STACK_OFFS(0x24, -0x8));
		Action A = MouseClass::Instance->DecideAction(XY, pTarget, 0);
		MouseClass::Instance->LeftMouseButtonUp(XYZ, XY, pTarget, A, 0);
	}

	return 0x693290;
}

DEFINE_HOOK(693388, MouseClass_ReactToClicks_RMBUp, 7)
{
	GET(int, X, EAX);
	GET(int, Y, ECX);
	LEA_STACK(Point2D *, XY, STACK_OFFS(0x24, 0x14));

	LEA_STACK(DWORD *, ptr, STACK_OFFS(0x24, 0x8));
	BYTE a5 = 0;
	BYTE a6 = 0;
	CoordStruct XYZdst = {0,0,0};
	CellStruct XYdst = {0,0};

	Point2D P2D = {X, Y};
	Game::sub_63AB00(P2D);

	*XY = P2D;

	MouseClass::Instance->ProcessClickCoords(XY, &XYdst, &XYZdst, (ObjectClass **)&ptr, &a5, &a6);

	PopupMenu::PixelCoords = P2D;
	PopupMenu::CellCoords = XYdst;

	return 0x69338F;
}

DEFINE_HOOK(6D4B42, TacticalClass_DrawStuff, 6)
{
	static const wchar_t * MenuItems[] = {L"Deactivate", L"Flash", L"IronCurtain", L"Damage", L"Hunt", L"\0"};

	RectangleStruct pRect;
	const wchar_t **menu = MenuItems;
	int i = 0;
	while(*menu && **menu) {
		pRect = Drawing::GetTextBox(*menu, &PopupMenu::PixelCoords, 3);
		if(pRect.Width > PopupMenu::MenuItem.Width) {
			PopupMenu::MenuItem = PopupMenu::MenuBox = pRect;
		}
		++menu;
	}

	if(PopupMenu::Drawing) {
		Point2D TL = PopupMenu::PixelCoords;
		menu = MenuItems;
		int i = 0;
		pRect = PopupMenu::MenuItem;
		pRect.X += TL.X;
		pRect.Y += TL.Y;
		while(*menu && **menu) {
			DSurface::Hidden_2->FillRect(&pRect, i == PopupMenu::HighlightedItem ? COLOR_RED : COLOR_WHITE);
			DSurface::Hidden_2->DrawTextA(*menu, &TL, COLOR_BLUE);
			TL.Y += pRect.Height;
			pRect.Y += pRect.Height;
			PopupMenu::MenuBox.Height += pRect.Height;
			++menu;
			++i;
		}
	}
	return 0;
}

DEFINE_HOOK(692FDC, sub_692F30, 8)
{
	if(PopupMenu::Drawing && PopupMenu::MenuItem.Height) {
		LEA_STACK(Point2D *, XY, STACK_OFFS(0x30, 0x18));

		Point2D offset = *XY - PopupMenu::PixelCoords;

		PopupMenu::HighlightedItem = -1;
		if(offset.X >= 0 && offset.X <= PopupMenu::MenuBox.Width) {
			if(offset.Y >= 0 && offset.Y <= PopupMenu::MenuBox.Height) {
				PopupMenu::HighlightedItem = static_cast<int>(offset.Y / PopupMenu::MenuItem.Height);
			}
		}

	}
	return 0;
}
