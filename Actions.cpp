#include <YRPP.h>
#include "Ares.h"
#include "Actions.h"

MousePointer * Actions::CustomPointer = NULL;

// 5BDDC0, 5
// reset cursor
EXPORT_FUNC(MouseClass_Update)
{
	Actions::CustomPointer = NULL;
	return 0;
}

// 5BDC8C, 7
// reset cursor
// EAX <= current pointer index
// ESI => &cursor
EXPORT_FUNC(MouseClass_SetPointer)
{
	RET_UNLESS(Actions::CustomPointer);

	MousePointer *pCursor = Actions::CustomPointer;

	if(pCursor->MiniFrame != -1)
	{
		R->set_BL(R->get_StackVar8(0x24));
	}
	else
	{
		R->set_StackVar8(0x24, 0);
		R->set_BL(0);
	}

	R->set_ESI((DWORD)pCursor);

	return 0x5BDCB4;
}

