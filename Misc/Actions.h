#ifndef ACTIONS_H
#define ACTIONS_H

#include <MouseClass.h>

// actions for custom sw
#define SW_YES_CURSOR 0x7F
#define SW_NO_CURSOR 0x7E

class Actions
{
	public:
		static void Set(MouseCursor *pCursor, bool bAllowShroud = false);

		static MouseCursor MP;
		static MouseCursor* MPCurrent;
		static MouseCursor* MPCustom;

		static MouseCursor* TempCursor;

		static bool MPCustomAllowShroud;
};

#endif
