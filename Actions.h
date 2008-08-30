#ifndef ACTIONS_H
#define ACTIONS_H

#include "Ares.h"

class Actions
{
	public:
		static void Set(MouseCursor *pCursor)
		{
			CustomCursor = pCursor;
		};

		static MouseCursor * CustomCursor;
};

#endif
