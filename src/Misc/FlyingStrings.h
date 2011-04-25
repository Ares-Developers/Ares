/*FlyingStrings.h
Useable to get out messages from units
Used to output Bounty messages
By Joshy
*/

#ifndef FLYINGSTRINGS_H
#define FLYINGSTRINGS_H

#include "../Ares.CRT.h"
#include "../Ares.h"

class FlyingStrings
{
private:
	struct Item {
		Point2D point;
		int created;
		wchar_t text[0x20];
		DWORD color;

		bool operator == (Item &t) {
			return false;
		}
	};
	
	static const int Duration = 75;
	
	static DynamicVectorClass<Item> data;

	static void Remove(int idx) {
		data.RemoveItem(idx);
	}
	
public:
	static void Add(const wchar_t* text, Point2D point, DWORD color) {
		// "remember" a text.
		Item item;
		item.point = point;
		item.created = Unsorted::CurrentFrame;
		item.color = color;
		AresCRT::wstrCopy(item.text, text, 0x20);
		
		data.AddItem(item);
	}
	
	static void UpdateAll() {
		for(int i=data.Count-1; i>=0; --i) {
			if(Item* pItem = &data[i]) {
				
				if(Unsorted::CurrentFrame > pItem->created + Duration - 70) {
					int pointX, pointY;
					pointX = pItem->point.X;
					pointY = pItem->point.Y - (Unsorted::CurrentFrame - pItem->created);
					DSurface::Hidden_2->DrawText(pItem->text, pointX, pointY, pItem->color);
				}
				else{
					DSurface::Hidden_2->DrawText(pItem->text, &pItem->point, pItem->color);
				}


				if(Unsorted::CurrentFrame > pItem->created + Duration) {
					Remove(i);
				}

			}
		}
	}
};

#endif