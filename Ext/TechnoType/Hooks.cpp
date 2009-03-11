#include "Body.h"
//include "Side.h"
#include "..\..\Enum\Prerequisites.h"
#include "..\..\Misc\Debug.h"

// =============================
// other hooks


DEFINE_HOOK(732D10, TacticalClass_CollectSelectedIDs, 5)
{
	return 0;
}
