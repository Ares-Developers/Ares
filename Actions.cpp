#include <YRPP.h>
#include "Ares.h"
#include "Actions.h"

MouseCursor * Actions::CustomCursor = NULL;
MouseCursor * Actions::LastCustomCursor = NULL;
int Actions::LastTimerFrame = 0;
int Actions::LastFrameIndex = 0;

void Actions::Set(MouseCursor *pCursor)
{
	if(pCursor != Actions::LastCustomCursor)
	{
		Actions::LastFrameIndex = 0;
		Actions::LastTimerFrame = Unsorted::CurrentFrame;
	}
	Actions::CustomCursor = pCursor;
};


// 5BDDC8, 6
// reset cursor
EXPORT_FUNC(MouseClass_Update)
{
	Actions::LastCustomCursor = Actions::CustomCursor;
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

	R->set_EAX(0xFF); // Actions::LastCustomCursor == Actions::CustomCursor ? 0x7F : 0xFF); // DOESN'T WORK
	R->set_ESI((DWORD)pCursor);

	return 0x5BDCB4;
}

// 5BDD86, 5
EXPORT_FUNC(MouseClass_SetCursor2)
{
	RET_UNLESS(Actions::CustomCursor);

	MouseCursor *pCursor = Actions::CustomCursor;

	if(Actions::LastTimerFrame - Unsorted::CurrentFrame > pCursor->Interval)
	{
		++Actions::LastFrameIndex;
		Actions::LastFrameIndex %= pCursor->Count;
		Actions::LastTimerFrame = Unsorted::CurrentFrame;
	}

	R->set_EDX(pCursor->Frame + Actions::LastFrameIndex);

	return 0;
}

