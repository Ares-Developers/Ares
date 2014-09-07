//#include "Body.h"

#include "../../Ares.h"

#include <FootClass.h>
#include <MapClass.h>
#include <ScenarioClass.h>
#include <TerrainClass.h>
#include <WarheadTypeClass.h>

DEFINE_HOOK(5F4FF9, ObjectClass_Put_IsFlammable, 7)
{
	//GET(ObjectClass*, pThis, ESI);
	GET(ObjectTypeClass*, pType, EBX);

	enum { RequiresUpdate = 0x5F501B, NoUpdate = 0x5F5045 };

	// terrain only needs to get Update called when it spawns, or now when
	// it is flammable. if none is set, don't update
	if(auto pTerrainType = abstract_cast<TerrainTypeClass*>(pType)) {
		if(!pTerrainType->SpawnsTiberium && !pTerrainType->IsFlammable) {
			return NoUpdate;
		}
	}

	return RequiresUpdate;
}

DEFINE_HOOK(71C5D2, TerrainClass_Ignite_IsFlammable, 6)
{
	GET(TerrainClass*, pThis, EDI);
	auto pType = pThis->Type;

	enum { Ignite = 0x71C5F3, CantBurn = 0x71C69D };

	// prevent objects from burning that aren't flammable also
	return (pType->SpawnsTiberium || !pType->IsFlammable) ? CantBurn : Ignite;
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
	GET(DamageState, res, EAX);
	GET_STACK(int*, pDamage, 0x40);
	GET_STACK(WarheadTypeClass*, pWarhead, 0x48);

	// ignite this terrain object
	if(!pThis->IsBurning && *pDamage > 0 && pWarhead->Sparky) {
		pThis->Ignite();
	}

	return (res == DamageState::NowDead) ? 0x71B9A7 : 0x71BB79;
}
