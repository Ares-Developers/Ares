#include "Body.h"
#include "../TechnoType/Body.h"
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

#include "../../Misc/Debug.h"

#include <vector>

// =============================
// hooks for 100 unit bug

// fix the 100 unit bug for vehicles
DEFINE_HOOK(4FEA60, HouseClass_AI_UnitProduction, 0)
{
	GET(HouseClass* const, pThis, ECX);

	retfunc_fixed<DWORD> ret(R, 0x4FEEDA, 15);

	if(pThis->ProducingUnitTypeIndex != -1) {
		return ret();
	}

	auto const pRules = RulesClass::Instance;

	auto const AIDiff = static_cast<int>(pThis->GetAIDifficultyIndex());

	auto const idxParentCountry = pThis->Type->FindParentCountryIndex();

	auto const pHarvester = HouseExt::FindOwned(
		pThis, idxParentCountry, make_iterator(pRules->HarvesterUnit));

	if(pHarvester) {
		//Buildable harvester found
		auto const harvesters = pThis->CountResourceGatherers;

		auto maxHarvesters = HouseExt::FindBuildable(
			pThis, idxParentCountry, make_iterator(pRules->BuildRefinery)) 
			? pRules->HarvestersPerRefinery[AIDiff] * pThis->CountResourceDestinations
			: pRules->AISlaveMinerNumber[AIDiff];

		if(pThis->IQLevel2 >= pRules->Harvester && !pThis->unknown_bool_242
			&& !pThis->ControlledByHuman() && harvesters < maxHarvesters
			&& pThis->TechLevel >= pHarvester->TechLevel)
		{
			pThis->ProducingUnitTypeIndex = pHarvester->ArrayIndex;
			return ret();
		}
	} else {
		//No buildable harvester found
		auto const maxHarvesters = pRules->AISlaveMinerNumber[AIDiff];

		if(pThis->CountResourceGatherers < maxHarvesters) {
			auto const pRefinery = HouseExt::FindBuildable(
				pThis, idxParentCountry, make_iterator(pRules->BuildRefinery));

			if(pRefinery) {
				//awesome way to find out whether this building is a slave miner, isn't it? ...
				if(auto const pSlaveMiner = pRefinery->UndeploysInto) {
					pThis->ProducingUnitTypeIndex = pSlaveMiner->ArrayIndex;
					return ret();
				}
			}
		}
	}

	GetTypeToProduce<UnitClass, UnitTypeClass>(pThis, pThis->ProducingUnitTypeIndex);

	return ret();
}

DEFINE_HOOK(4FEEE0, HouseClass_AI_InfantryProduction, 6)
{
	GET(HouseClass*, pThis, ECX);

	if(pThis->ProducingInfantryTypeIndex == -1) {
		GetTypeToProduce<InfantryClass, InfantryTypeClass>(pThis, pThis->ProducingInfantryTypeIndex);
	}

	R->EAX(15);
	return 0x4FF204;
}

DEFINE_HOOK(4FF210, HouseClass_AI_AircraftProduction, 6)
{
	GET(HouseClass*, pThis, ECX);

	if(pThis->ProducingAircraftTypeIndex == -1) {
		GetTypeToProduce<AircraftClass, AircraftTypeClass>(pThis, pThis->ProducingAircraftTypeIndex);
	}

	R->EAX(15);
	return 0x4FF534;
}
