#include <YRPP.h>
#include "Ares.h"
#include "Actions.h"

MouseCursor * Actions::CustomCursor = NULL;

// 5BDDC8, 6
// reset cursor
EXPORT_FUNC(MouseClass_Update)
{
	Actions::CustomCursor = NULL;
	return 0;
}

// 5BDC8C, 7
// reset cursor
// EAX <= current Cursor index
// ESI => &cursor
EXPORT_FUNC(MouseClass_SetCursor)
{
	RET_UNLESS(Actions::CustomCursor);

	MouseCursor *pCursor = Actions::CustomCursor;

	if(pCursor->MiniFrame != -1)
	{
		R->set_BL(R->get_StackVar8(0x24));
	}
	else
	{
		R->set_StackVar8(0x24, 0);
		R->set_BL(0);
	}

	R->set_EAX(-1);
	R->set_ESI((DWORD)pCursor);

	return 0x5BDCB4;
}

