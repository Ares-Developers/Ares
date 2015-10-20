#include "Body.h"

#include <ScenarioClass.h>
#include <VocClass.h>

#include <Helpers\Macro.h>

#include <type_traits>

// #1004906: support more than 100 waypoints
DEFINE_HOOK(6E1780, TActionClass_PlayAudioAtRandomWP, 6)
{
	GET(TActionClass*, pThis, ECX);
	//GET_STACK(HouseClass*, pHouse, 0x4);
	//GET_STACK(ObjectClass*, pSourceObject, 0x8);
	//GET_STACK(TriggerClass*, pTrigger, 0xC);
	//GET_STACK(const CellStruct*, pLocation, 0x10);

	constexpr auto const MaxWaypoints = static_cast<int>(
		std::extent<decltype(ScenarioClass::Waypoints)>::value);

	int buffer[MaxWaypoints];
	DynamicVectorClass<int> eligible(MaxWaypoints, buffer);

	auto const pScen = ScenarioClass::Instance;

	for(auto ix = 0; ix < MaxWaypoints; ++ix) {
		if(pScen->IsDefinedWaypoint(ix)) {
			eligible.AddItem(ix);
		}
	}

	if(eligible.Count > 0) {
		auto const index = pScen->Random.RandomRanged(0, eligible.Count - 1);
		auto const luckyWP = eligible[index];
		auto const cell = pScen->GetWaypointCoords(luckyWP);
		auto const coords = CellClass::Cell2Coord(cell);
		VocClass::PlayIndexAtPos(pThis->Value, coords);
	}

	R->EAX(1);
	return 0x6E182C;
}
