//#include "Body.h"

#include "../../Ares.h"

#include <FootClass.h>
#include <MapClass.h>
#include <ScenarioClass.h>
#include <TerrainClass.h>
#include <WarheadTypeClass.h>

DEFINE_HOOK(5F4FF9, ObjectClass_Put_ForestFire, 7)
{
	//GET(ObjectClass*, pThis, ESI);
	return 0x5F501B;
}

DEFINE_HOOK(71C7C2, TerrainClass_Update_ForestFire, 6)
{
	GET(TerrainClass*, pThis, ESI);

	const auto& flammability = RulesClass::Instance->TreeFlammability;

	// burn spread probability this frame
	if(flammability > 0.0) {
		if(pThis->IsBurning && ScenarioClass::Instance->Random.RandomRanged(0, 99) == 0) {
			auto pCell = pThis->GetCell();

			// check all neighbour cells that contain terrain objects and
			// roll the dice for each of them.
			for(unsigned int i = 0; i < 8; ++i) {
				auto pNeighbour = pCell->GetNeighbourCell(i);
				if(auto pTree = pNeighbour->GetTerrain(false)) {
					if(!pTree->IsBurning && ScenarioClass::Instance->Random.RandomDouble() < flammability) {
						pTree->Ignite();
					}
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(71B99E, TerrainClass_ReceiveDamage_ForestFire, 9)
{
	GET(TerrainClass*, pThis, ESI);
	GET(DamageState::Value, res, EAX);
	GET_STACK(int*, pDamage, 0x40);
	GET_STACK(WarheadTypeClass*, pWarhead, 0x48);

	// ignite this terrain object
	if(!pThis->IsBurning && *pDamage > 0 && pWarhead->Sparky) {
		pThis->Ignite();
	}

	return (res == DamageState::NowDead) ? 0x71B9A7 : 0x71BB79;
}
