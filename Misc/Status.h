#ifndef STATUS_MESSAGES_H
#define STATUS_MESSAGES_H

#include "..\Ares.h"
#include <Drawing.h>
#include <LoadProgressManager.h>

class StatusMessages {
public:
	static Point2D TLPoint;
	static Point2D Delta;

	static void Add(const wchar_t * pText, DWORD dwColor) {
		LoadProgressManager::DrawText(pText, TLPoint.X, TLPoint.Y, dwColor);
		TLPoint += Delta;
	}

};

#endif
