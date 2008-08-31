#ifndef ACTIONS_H
#define ACTIONS_H

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
