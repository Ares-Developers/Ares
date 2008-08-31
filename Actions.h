#ifndef ACTIONS_H
#define ACTIONS_H

#include "Ares.h"

class Actions
{
	public:
		static void Set(MouseCursor *pCursor)
		{
			if(pCursor == LastCustomCursor)
			{
				++LastFrame;
			}
			else
			{
				LastFrame = 0;
			}
			LastCustomCursor = CustomCursor;
			CustomCursor = pCursor;
		};
		
		static bool Changed()
			{ return LastFrame == 0; }

		static MouseCursor * CustomCursor;
		static MouseCursor * LastCustomCursor;
		static int LastFrame;
};

#endif
