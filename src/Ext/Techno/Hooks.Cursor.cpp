#include "Body.h"

#include <AircraftClass.h>
#include <BuildingClass.h>

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

// skip the check for UnitRepair, as it does not play well with UnitReload and
// Factory=AircraftType at all. in fact, it's prohibited, and thus docking to
// other structures was never allowed.
DEFINE_HOOK(417E16, AircraftClass_GetCursorOverObject_Dock, 6)
{
	// target is known to be a building
	GET(AircraftClass* const, pThis, ESI);
	GET(BuildingClass* const, pBuilding, EDI);

	// enter and no-enter cursors only if aircraft can dock
	if(pThis->Type->Dock.FindItemIndex(pBuilding->Type) != -1) {
		return 0x417E4B;
	}

	// select cursor
	return 0x417E7D;
}
