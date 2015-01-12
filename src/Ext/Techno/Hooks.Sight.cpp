#include "../../Ares.h"
#include "../../Misc/MapRevealer.h"

DEFINE_HOOK(5673A0, MapClass_RevealArea0, 5)
{
	//GET(MapClass*, pThis, ECX);
	GET_STACK(CoordStruct const*, pCoords, 0x4);
	GET_STACK(int, radius, 0x8);
	GET_STACK(HouseClass*, pHouse, 0xC);
	GET_STACK(bool, onlyOutline, 0x10);
	GET_STACK(bool, a6, 0x14);
	GET_STACK(bool, fog, 0x18);
	GET_STACK(bool, allowRevealByHeight, 0x1C);
	GET_STACK(bool, add, 0x20);

	MapRevealer const revealer(*pCoords);
	revealer.Reveal0(*pCoords, radius, pHouse, onlyOutline, a6, fog, allowRevealByHeight, add);
	revealer.UpdateShroud(0, static_cast<size_t>(std::max(radius, 0)), false);

	return 0x5678D6;
}

DEFINE_HOOK(5678E0, MapClass_RevealArea1, 5)
{
	//GET(MapClass*, pThis, ECX);
	GET_STACK(CoordStruct const*, pCoords, 0x4);
	GET_STACK(int, radius, 0x8);
	GET_STACK(HouseClass*, pHouse, 0xC);
	GET_STACK(bool, onlyOutline, 0x10);
	//GET_STACK(bool, a6, 0x14);
	GET_STACK(bool, fog, 0x18);
	GET_STACK(bool, allowRevealByHeight, 0x1C);
	GET_STACK(bool, add, 0x20);

	MapRevealer const revealer(*pCoords);
	revealer.Reveal1(*pCoords, radius, pHouse, onlyOutline, fog, allowRevealByHeight, add);

	return 0x567D8F;
}

DEFINE_HOOK(567DA0, MapClass_RevealArea2, 5)
{
	//GET(MapClass*, pThis, ECX);
	GET_STACK(CoordStruct const*, pCoords, 0x4);
	GET_STACK(int, start, 0x8);
	GET_STACK(int, radius, 0xC);
	GET_STACK(bool, fog, 0x10);

	MapRevealer const revealer(*pCoords);
	revealer.UpdateShroud(static_cast<size_t>(std::max(start, 0)), static_cast<size_t>(std::max(radius, 0)), fog);

	return 0x567F61;
}
