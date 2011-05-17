/*FlyingStrings.h
Useable to get out messages from units
Used to output Bounty messages
By AlexB and Joshy
*/

#ifndef FLYINGSTRINGS_H
#define FLYINGSTRINGS_H

#include "../Ares.CRT.h"
#include "../Ares.h"
#include <TacticalClass.h>

class FlyingStrings
{
private:
	struct Item {
		CoordStruct coords;
		int created;
		wchar_t text[0x20];
		DWORD color;

		bool operator == (Item &t) const {
			return false;
		}
	};
	
	static const int Duration = 75;
	
	static DynamicVectorClass<Item> data;

	static void Remove(int idx) {
		data.RemoveItem(idx);
	}
	
public:
	static void Add(const wchar_t* text, CoordStruct* coords, DWORD color) {
		// "remember" a text.
		Item item;
		item.coords = *coords;
		item.created = Unsorted::CurrentFrame;
		item.color = color;
		AresCRT::wstrCopy(item.text, text, 0x20);
		
		data.AddItem(item);
	}
	
	static void UpdateAll() {
		for(int i=data.Count-1; i>=0; --i) {
			if(Item* pItem = &data[i]) {
				Point2D point;
				TacticalClass::Instance->CoordsToClient(&pItem->coords, &point);
				int pointX, pointY;

				if(Unsorted::CurrentFrame > pItem->created + Duration - 70) {

					pointX = point.X;
					pointY = point.Y - (Unsorted::CurrentFrame - pItem->created);
					DSurface::Hidden_2->DrawText(pItem->text, pointX, pointY, pItem->color);
				}
				else{
					pointX = point.X;
					pointY = point.Y;
					DSurface::Hidden_2->DrawText(pItem->text, pointX, pointY, pItem->color);
				}

				if(Unsorted::CurrentFrame > pItem->created + Duration || Unsorted::CurrentFrame < pItem->created) {
					Remove(i);
				}
			}
		}
	}
};

#endif
