#include "Body.h"

#include <Helpers\Enumerators.h>

// create enumerator
DEFINE_HOOK(4895B8, DamageArea_CellSpread1, 6) {
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0xE0, 0xB4));
	GET(int, spread, EAX);

	pIter = nullptr;

	if(spread >= 0) {
		pIter = new CellSpreadEnumerator(spread);
	}

	return (pIter && *pIter) ? 0x4895C3 : 0x4899DA;
}

// apply the current value
DEFINE_HOOK(4895C7, DamageArea_CellSpread2, 8)
{
	GET_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0xE0, 0xB4));

	auto &offset = **pIter;
	R->DX(offset.X);
	R->AX(offset.Y);

	return 0x4895D7;
}

// advance and delete if done
DEFINE_HOOK(4899BE, DamageArea_CellSpread3, 8)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0xE0, 0xB4));
	REF_STACK(int, index, STACK_OFFS(0xE0, 0xD0));

	// reproduce skipped instruction
	index++;

	// advance iterator
	if(++*pIter) {
		return 0x4895C0;
	}

	// all done. delete and go on
	delete pIter;

	return 0x4899DA;
}
