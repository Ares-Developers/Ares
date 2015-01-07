#include "../../Ares.h"

#include <Helpers/Enumerators.h>

CellStruct GetRelation(const CellStruct &offset) {
	return{static_cast<short>(Math::sgn(-offset.X)),
		static_cast<short>(Math::sgn(-offset.Y))};
}

/* MapClass::RevealArea0
*
* Stack 0x44 has been repurposed, was CellStruct*
*/

// setup enumerator, which can be null for a fast exit
DEFINE_HOOK(5674EA, MapClass_RevealArea0_CellSpread1, 6)
{
	REF_STACK(int, radius, STACK_OFFS(0x54, -0x8));
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0x54, 0x44));
	REF_STACK(bool, allowNoRevealByHeight, STACK_OFFS(0x54, -0x10));
	GET_STACK(HouseClass*, pOwnerHouse, STACK_OFFS(0x54, -0xC));

	if(!radius) {
		pIter = nullptr;
		return 0x5678CF;
	}

	if(radius > CellSpreadEnumerator::Max) {
		radius = CellSpreadEnumerator::Max;
	}

	size_t start = 0;
	if(!RulesClass::Instance->RevealByHeight && allowNoRevealByHeight && radius > 2) {
		start = radius - 3;
	}

	pIter = new CellSpreadEnumerator(radius, start);

	// reproduce skipped instructions
	R->EDI(pOwnerHouse);
	return pOwnerHouse ? 0x567572 : 0x5675C9;
}

// validate enumerator which can be null (quick exit path)
DEFINE_HOOK(567602, MapClass_RevealArea0_CellSpread2, 5)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0x54, 0x44));

	// end condition
	if(!pIter || !*pIter) {
		delete pIter;
		return 0x5678B7;
	}

	// coords
	auto& offset = **pIter;
	R->AX(static_cast<WORD>(R->BX() + static_cast<WORD>(offset.X)));
	R->CX(R->Stack16(0x2A) + static_cast<WORD>(offset.Y));

	// that other thing
	static CellStruct rel;
	rel = GetRelation(offset);
	R->EDX(&rel);

	return 0x567635; // originally 0x567612, where another hook would be;
}

// just advance, validation is done above
DEFINE_HOOK(5678A8, MapClass_RevealArea0_CellSpread3, 5)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0x54, 0x44));
	++*pIter;
	return 0x567602; // originally 0x567612;, but this saves a hook
}


/* MapClass::RevealArea1
*
* Stack 0x38 has been repurposed, was CellStruct*
*/

// clear the enumerator
DEFINE_HOOK(5679FC, MapClass_RevealArea1_CellSpread1, 6)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0x48, 0x38));
	pIter = nullptr;
	return 0;
}

// validate values and create enumerator
DEFINE_HOOK(567A4A, MapClass_RevealArea1_CellSpread2, 5)
{
	REF_STACK(int, radius, STACK_OFFS(0x48, -0x8));
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0x48, 0x38));
	REF_STACK(bool, allowNoRevealByHeight, STACK_OFFS(0x48, -0x10));
	GET_STACK(HouseClass*, pOwnerHouse, STACK_OFFS(0x48, -0xC));

	if(radius > CellSpreadEnumerator::Max) {
		radius = CellSpreadEnumerator::Max;
	}

	size_t start = 0;
	if(!RulesClass::Instance->RevealByHeight && allowNoRevealByHeight && radius > 2) {
		start = radius - 3;
	}

	pIter = new CellSpreadEnumerator(radius, start);

	// reproduce skipped instructions
	R->EBX(pOwnerHouse);
	return pOwnerHouse ? 0x567AB4 : 0x567B0B;
}

// initial loop check
DEFINE_HOOK(567B18, MapClass_RevealArea1_CellSpread3, 5)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0x48, 0x38));
	return (pIter && *pIter) ? 0x567B33 : 0x567D88;
}

// apply enumerator value
DEFINE_HOOK(567B37, MapClass_RevealArea1_CellSpread4, 5)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0x48, 0x38));

	// coords
	auto& offset = **pIter;
	R->ECX(&offset);

	// that other thing
	static CellStruct rel;
	rel = GetRelation(offset);
	R->EBP(&rel - 1);

	return 0x567B40;
}

// move to next
DEFINE_HOOK(567D79, MapClass_RevealArea1_CellSpread5, 5)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0x48, 0x38));
	return ++*pIter ? 0x567B2A : 0x567D88;
}

// delete the enumerator
DEFINE_HOOK(567D88, MapClass_RevealArea1_CellSpread6, 7)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0x48, 0x38));
	delete pIter;
	return 0;
}


/* MapClass::RevealArea2
*
* Stack -0xC has been repurposed in all but the first hook, was CellStruct*
*/

// validate values, create enumerator
DEFINE_HOOK(567DA1, MapClass_RevealArea2_CellSpread1, 8)
{
	REF_STACK(int, height, STACK_OFFS(0x4, -0x8));
	REF_STACK(int, radius, STACK_OFFS(0x4, -0xC));
	REF_STACK(bool, a5, STACK_OFFS(0x4, -0x10));

	if(radius > CellSpreadEnumerator::Max) {
		radius = CellSpreadEnumerator::Max;
	} else if(radius < 3) {
		radius = 3;
	}

	size_t start = 0;
	if(height < 0) {
		height = 0;
	} else if(height) {
		if(height > CellSpreadEnumerator::Max - 3) {
			height = CellSpreadEnumerator::Max - 3;
		}

		start = height - 1;
	}

	// bring this check forward, to not needlessy allocate memory
	CellSpreadEnumerator* ret = nullptr;
	if(!a5) {
		ret = new CellSpreadEnumerator(radius, start);
	}

	R->EAX(ret);
	return 0x567DFD;
}

// loop enter
DEFINE_HOOK(567E9A, MapClass_RevealArea2_CellSpread2, 6)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0x14, -0xC));
	return (pIter && *pIter) ? 0x567EAE : 0x567F5C;
}

// apply enumerator value
DEFINE_HOOK(567EAE, MapClass_RevealArea2_CellSpread3, 6)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0x14, -0xC));

	auto& offset = **pIter;
	R->EAX(&offset);

	return 0x567EB3;
}

// advance and loop
DEFINE_HOOK(567F4D, MapClass_RevealArea2_CellSpread4, 5)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0x14, -0xC));
	return ++*pIter ? 0x567E9A : 0x567F5C; // first one originally was 0x567EAE;
}

// delete the enumerator
DEFINE_HOOK(567F5C, MapClass_RevealArea2_CellSpread5, 5)
{
	REF_STACK(CellSpreadEnumerator*, pIter, STACK_OFFS(0x14, -0xC));
	delete pIter;
	return 0;
}
