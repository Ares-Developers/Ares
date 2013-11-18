#include "Body.h"

#include <VocClass.h>

// #1004906: support more than 100 waypoints
DEFINE_HOOK(6E1780, TActionClass_PlayAudioAtRandomWP, 6)
{
	GET(TActionClass*, pThis, ECX);
	//GET_STACK(HouseClass*, pHouse, 0x4);
	//GET_STACK(ObjectClass*, pSourceObject, 0x8);
	//GET_STACK(TriggerClass*, pTrigger, 0xC);
	//GET_STACK(const CellStruct*, pLocation, 0x10);

	std::vector<int> eligibleWPs;

	auto pScen = ScenarioClass::Instance;

	for(auto ix = 0; ix < 702; ++ix) {
		if(pScen->IsDefinedWaypoint(ix)) {
			eligibleWPs.push_back(ix);
		}
	}

	if(eligibleWPs.size() > 0) {
		auto luckyWP = pScen->Random.RandomRanged(0, eligibleWPs.size() - 1);
		CellStruct XY;
		pScen->GetWaypointCoords(&XY, luckyWP);
		CoordStruct XYZ;
		CellClass::Cell2Coord(&XY, &XYZ);
		VocClass::PlayIndexAtPos(pThis->Value, &XYZ);
	}

	R->EAX(1);
	return 0x6E182C;
}
