#include "Body.h"

// NOTE! enabling this hook breaks tank bunker. (#1155833)
//A_FINE_HOOK(6FFF03, TechnoClass_GetCursorOverObject, A)
//{
//	enum {
//		Cursor_Select = 0x6FFF7E,
//		Cursor_None = 0x7005E6,
//		Cursor_ToggleSelect = 0x700104,
//		Cursor_NotMyProblem = 0x6FFF8D
//	} Result = Cursor_NotMyProblem;
//
//	GET(TechnoClass *, pThis, ESI);
//
//	if(generic_cast<FootClass *>(pThis)) {
//		// BuildingClass::IsControllable only permits factories and conyards, derp herp
//		if(pThis->Owner->ControlledByPlayer() && !pThis->IsControllable()) {
//			if(pThis->CanBeSelected()) {
//				// all four derivate classes have special cased this value and will never attempt to override it
//				Result = Cursor_ToggleSelect;
//			} else {
//				Result = Cursor_None;
//			}
//		}
//	}
//
//	return Result;
//}
