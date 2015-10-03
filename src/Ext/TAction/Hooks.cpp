#include "Body.h"

#include <Helpers\Macro.h>

DEFINE_HOOK(6DD8D7, TActionClass_Execute, A)
{
	GET(TActionClass* const, pAction, ESI);
	GET(ObjectClass* const, pObject, ECX);

	GET_STACK(HouseClass* const, pHouse, 0x254);
	GET_STACK(TriggerClass* const, pTrigger, 0x25C);
	GET_STACK(CellStruct const*, pLocation, 0x260);

	enum { Handled = 0x6DFDDD, Default = 0x6DD8E7u };

	// check for actions handled in Ares.
	auto ret = false;
	if(TActionExt::Execute(
		pAction, pHouse, pObject, pTrigger, *pLocation, &ret))
	{
		// returns true or false
		R->AL(ret ? 1 : 0);
		return Handled;
	}

	// replicate the original instructions, using underflow
	auto const value = static_cast<unsigned int>(pAction->ActionKind) - 1;
	R->EDX(value);
	return (value > 144u) ? Handled : Default;
}
