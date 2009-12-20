#include "Body.h"
#include "../TechnoType/Body.h"

// =============================
// other hooks

DEFINE_HOOK(4F7870, HouseClass_PrereqValidator, 7)
{
	// int (TechnoTypeClass *item, bool BuildLimitOnly, bool includeQueued)
	/* return
		 1 - cameo shown
		 0 - cameo not shown
		-1 - cameo greyed out
	 */

	GET(HouseClass *, pHouse, ECX);
	GET_STACK(TechnoTypeClass *, pItem, 0x4);
	GET_STACK(bool, BuildLimitOnly, 0x8);
	GET_STACK(bool, IncludeQueued, 0xC);

	R->EAX(HouseExt::PrereqValidate(pHouse, pItem, BuildLimitOnly, IncludeQueued));
	return 0x4F8361;
}

// upgrades as prereqs, facepalm of epic proportions
// not needed anymore since the whole function's been replaced
/*
A_FINE_HOOK(4F7E49, HouseClass_CanBuildHowMany_Upgrades, 5)
{
		return R->get_EAX() < 3 ? 0x4F7E41 : 0x4F7E34;
}
*/

