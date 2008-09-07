#ifndef ACTIONS_H
#define ACTIONS_H

#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include "Ares.h"

class Actions
{
	public:
		static void Set(MouseCursor *pCursor);

		static MouseCursor * CustomCursor;
		static MouseCursor * LastCustomCursor;
		static int LastTimerFrame;
		static int LastFrameIndex;
};

#endif
