#include "Body.h"
#include "..\TechnoType\Body.h"
#include "MacroHacks.h"

#include <RulesClass.h>
#include <ScenarioClass.h>
#include <TeamClass.h>
#include <TeamTypeClass.h>
#include <HouseTypeClass.h>
#include <AircraftClass.h>
#include <AircraftTypeClass.h>
#include <InfantryClass.h>
#include <InfantryTypeClass.h>
#include <UnitClass.h>
#include <UnitTypeClass.h>

#include "..\..\Misc\Debug.h"

#include <vector>

// =============================
// hooks for 100 unit bug

// fix the 100 unit bug for vehicles
DEFINE_HOOK(4FEA60, HouseClass_AI_UnitProduction, 0)
{
	CRAZY_MACRO_GO_AWAY_1(0x4FEEDA, Unit);

	int nParentCountryIndex = HouseTypeClass::FindIndex(pThis->Type->get_ParentCountry());
	DWORD flagsOwner = 1 << nParentCountryIndex;

	UnitTypeClass* pHarvester = NULL;
	for(int i = 0; i < RulesClass::Global()->get_HarvesterUnit()->Count; i++) {
		UnitTypeClass* pCurrent = RulesClass::Global()->get_HarvesterUnit()->GetItem(i);
		if(pCurrent->OwnerFlags & flagsOwner) {
			pHarvester = pCurrent;
			break;
		}
	}

	if(pHarvester) {
		//Buildable harvester found
		int nHarvesters = pThis->CountResourceGatherers;

		int mMaxHarvesters = 
			RulesClass::Global()->get_HarvestersPerRefinery()->GetItem(AIDiff)
				 * pThis->get_CountResourceDestinations();
		if(!pThis->FirstBuildableFromArray(RulesClass::Global()->get_BuildRefinery())) {
			mMaxHarvesters = 
				RulesClass::Global()->get_AISlaveMinerNumber()->GetItem(AIDiff);
		}

		if(pThis->IQLevel2 >= RulesClass::Global()->get_Harvester() && !pThis->unknown_bool_242) {

			bool bPlayerControl;

			//TODO : Session::Global()->get_GameMode()
			if(*(eGameMode*)0xA8B238 == gm_Campaign) {
				bPlayerControl = pThis->CurrentPlayer || pThis->PlayerControl;
			} else {
				bPlayerControl = pThis->CurrentPlayer;
			}

			if(!bPlayerControl && nHarvesters < mMaxHarvesters && pThis->TechLevel >= pHarvester->TechLevel) {
				pThis->ProducingUnitTypeIndex = pHarvester->ArrayIndex;
				return ret();
			}
		}
	} else {
		//No buildable harvester found
		int mMaxHarvesters = RulesClass::Global()->get_AISlaveMinerNumber()->GetItem(AIDiff);

		if(pThis->CountResourceGatherers < mMaxHarvesters) {
			BuildingTypeClass* pBT = pThis->FirstBuildableFromArray(RulesClass::Global()->get_BuildRefinery());
			if(pBT) {
				//awesome way to find out whether this building is a slave miner, isn't it? ...
				UnitTypeClass* pSlaveMiner = pBT->UndeploysInto;
				if(pSlaveMiner) {
					pThis->ProducingUnitTypeIndex = pSlaveMiner->ArrayIndex;
					return ret();
				}
			}
		}
	}

	CRAZY_MACRO_GO_AWAY_2(Unit);
}

DEFINE_HOOK(4FEEE0, HouseClass_AI_InfantryProduction, 6)
{
	CRAZY_MACRO_GO_AWAY_1(0x4FF204, Infantry)
	CRAZY_MACRO_GO_AWAY_2(Infantry)
}

DEFINE_HOOK(4FF210, HouseClass_AI_AircraftProduction, 6)
{
	CRAZY_MACRO_GO_AWAY_1(0x4FF534, Aircraft)
	CRAZY_MACRO_GO_AWAY_2(Aircraft)
}

