#ifndef ACTIONS_H
#define ACTIONS_H

#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include <MouseClass.h>

class Actions
{
	public:
		static void Set(MouseCursor *pCursor);
		static void Set(MouseCursor *pCursor, bool bAllowShroud);

		static MouseCursor MP;
		static MouseCursor* MPCurrent;
		static MouseCursor* MPCustom;

		static MouseCursor* TempCursor;

		static bool MPCustomAllowShroud;
};

#endif
