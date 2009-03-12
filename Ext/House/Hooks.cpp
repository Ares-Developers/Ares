#include "Body.h"
#include "..\TechnoType\Body.h"

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
// other hooks

DEFINE_HOOK(4F7870, HouseClass_PrereqValidator, 7)
{
	// int (TechnoTypeClass *item, bool BuildLimitOnly, bool includeQueued)
	/* return
		 1 - cameo shown
		 0 - cameo not shown
		-1 - cameo greyed out
	 */

	GET(HouseClass *, pHouse, ECX);
	GET_STACK(TechnoTypeClass *, pItem, 0x4);
	GET_STACK(bool, BuildLimitOnly, 0x8);
	GET_STACK(bool, IncludeQueued, 0xC);

	R->set_EAX(HouseExt::PrereqValidate(pHouse, pItem, BuildLimitOnly, IncludeQueued));
	return 0x4F8361;
}

// upgrades as prereqs, facepalm of epic proportions
DEFINE_HOOK(4F7E49, HouseClass_CanBuildHowMany_Upgrades, 5)
{
		return R->get_EAX() < 3 ? 0x4F7E41 : 0x4F7E34;
}

// fix the 100 unit bug for vehicles
DEFINE_HOOK(4FEA60, Fix100UnitBug_Vehicles, 0)
{
	retfunc_fixed<DWORD> ret(R, 0x4FEEDA, 0xF);

	GET(HouseClass*, pThis, ECX);

	if(pThis->ProducingUnitTypeIndex != -1) {
		return ret();
	}

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

	int AIDiff = pThis->AIDifficulty;

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

	// Westwood, meet my friend the resizable array
	std::vector<int> CreationFrames(UnitTypeClass::Array->Count, 0);
	std::vector<int> Values(UnitTypeClass::Array->Count, 0x7FFFFFFF);

	for(int i = 0; i < TeamClass::Array->Count; ++i) {
		TeamClass *CurrentTeam = TeamClass::Array->GetItem(i);
		if(!CurrentTeam || CurrentTeam->Owner != pThis) {
			continue;
		}

		int TeamCreationFrame = CurrentTeam->CreationFrame;

		// what? copy pasting original code, leave it be
		if(
		 (!CurrentTeam->Type->Reinforce || CurrentTeam->unknown_79)
		  &&
		 (!CurrentTeam->unknown_77 || !CurrentTeam->unknown_78)
		) {
			continue;
		}

		DynamicVectorClass<TechnoTypeClass *> TaskForceMembers;
		CurrentTeam->GetTaskForceMembers(&TaskForceMembers);
		for(int j = 0; j < TaskForceMembers.Count; ++j) {
			TechnoTypeClass *CurrentMember = TaskForceMembers[i];
			if(CurrentMember->WhatAmI() != abs_UnitType) {
				continue;
			}
			int Idx = CurrentMember->GetArrayIndex();
			++Values[Idx];
			if(CreationFrames[Idx] < TeamCreationFrame) {
				CreationFrames[Idx] = TeamCreationFrame;
			}
		}
	}
	
	for(int i = 0; i < UnitClass::Array->Count; ++i) {
		UnitClass *U = UnitClass::Array->GetItem(i);
		int Idx = U->GetArrayIndex();
		if(Values[Idx] > 0 && U->CanBeRecruited(pThis)) {
			--Values[Idx];
		}
	}

	std::vector<int> BestChoices;
	int BestValue = -1;
	int EarliestUnitIndex = -1;
	int EarliestFrame = 0x7FFFFFFF;

	for(int i = 0; i < UnitTypeClass::Array->Count; ++i) {
		UnitTypeClass *UT = UnitTypeClass::Array->GetItem(i);
		int CurrentValue = Values[i];
		if(CurrentValue <= 0 || !pThis->CanBuild(UT, 0, 0) || UT->GetActualCost(pThis) > pThis->Available_Money()) {
			continue;
		}

		if(CurrentValue < BestValue || CurrentValue == -1) {
			BestValue = CurrentValue;
			BestChoices.clear();
		}
		BestChoices.push_back(i);
		if(EarliestFrame < CreationFrames[i] || EarliestUnitIndex == -1) {
			EarliestUnitIndex = i;
			EarliestFrame = CreationFrames[i];
		}
	}

	int EarliestOdds = RulesClass::Global()->get_FillEarliestTeamProbability()->GetItem(AIDiff);
	if(ScenarioClass::Global()->get_Random()->RandomRanged(1, 100) < EarliestOdds) {
		pThis->ProducingUnitTypeIndex = EarliestUnitIndex;
		return ret();
	}
	if(BestChoices.size()) {
		int RandomChoice = ScenarioClass::Global()->get_Random()->RandomRanged(0, BestChoices.size() - 1);
		int RandomIndex = BestChoices.at(RandomChoice);
		pThis->ProducingUnitTypeIndex = RandomIndex;
	}
	return ret();
}

DEFINE_HOOK(4FEEE0, HouseClass_AI_InfantryProduction, 6)
{
	retfunc_fixed<DWORD> ret(R, 0x4FF204, 0xF);

	GET(HouseClass*, pThis, ECX);
	int AIDiff = pThis->AIDifficulty;

	// Westwood, meet my friend the resizable array
	std::vector<int> CreationFrames(InfantryTypeClass::Array->Count, 0);
	std::vector<int> Values(InfantryTypeClass::Array->Count, 0x7FFFFFFF);

	for(int i = 0; i < TeamClass::Array->Count; ++i) {
		TeamClass *CurrentTeam = TeamClass::Array->GetItem(i);
		if(!CurrentTeam || CurrentTeam->Owner != pThis) {
			continue;
		}

		int TeamCreationFrame = CurrentTeam->CreationFrame;

		// what? copy pasting original code, leave it be
		if(
		 (!CurrentTeam->Type->Reinforce || CurrentTeam->unknown_79)
		  &&
		 (!CurrentTeam->unknown_77 || !CurrentTeam->unknown_78)
		) {
			continue;
		}

		DynamicVectorClass<TechnoTypeClass *> TaskForceMembers;
		CurrentTeam->GetTaskForceMembers(&TaskForceMembers);
		for(int j = 0; j < TaskForceMembers.Count; ++j) {
			TechnoTypeClass *CurrentMember = TaskForceMembers[i];
			if(CurrentMember->WhatAmI() != abs_InfantryType) {
				continue;
			}
			int Idx = CurrentMember->GetArrayIndex();
			++Values[Idx];
			if(CreationFrames[Idx] < TeamCreationFrame) {
				CreationFrames[Idx] = TeamCreationFrame;
			}
		}
	}
	
	for(int i = 0; i < InfantryClass::Array->Count; ++i) {
		InfantryClass *I = InfantryClass::Array->GetItem(i);
		int Idx = I->GetArrayIndex();
		if(Values[Idx] > 0 && I->CanBeRecruited(pThis)) {
			--Values[Idx];
		}
	}

	std::vector<int> BestChoices;
	int BestValue = -1;
	int EarliestInfantryIndex = -1;
	int EarliestFrame = 0x7FFFFFFF;

	for(int i = 0; i < InfantryTypeClass::Array->Count; ++i) {
		InfantryTypeClass *IT = InfantryTypeClass::Array->GetItem(i);
		int CurrentValue = Values[i];
		if(CurrentValue <= 0 || !pThis->CanBuild(IT, 0, 0) || IT->GetActualCost(pThis) > pThis->Available_Money()) {
			continue;
		}

		if(CurrentValue < BestValue || CurrentValue == -1) {
			BestValue = CurrentValue;
			BestChoices.clear();
		}
		BestChoices.push_back(i);
		if(EarliestFrame < CreationFrames[i] || EarliestInfantryIndex == -1) {
			EarliestInfantryIndex = i;
			EarliestFrame = CreationFrames[i];
		}
	}

	int EarliestOdds = RulesClass::Global()->get_FillEarliestTeamProbability()->GetItem(AIDiff);
	if(ScenarioClass::Global()->get_Random()->RandomRanged(1, 100) < EarliestOdds) {
		pThis->ProducingInfantryTypeIndex = EarliestInfantryIndex;
		return ret();
	}
	if(BestChoices.size()) {
		int RandomChoice = ScenarioClass::Global()->get_Random()->RandomRanged(0, BestChoices.size() - 1);
		int RandomIndex = BestChoices.at(RandomChoice);
		pThis->ProducingInfantryTypeIndex = RandomIndex;
	}
	return ret();
}

DEFINE_HOOK(4FF210, HouseClass_AI_AircraftProduction, 6)
{
	retfunc_fixed<DWORD> ret(R, 0x4FF534, 0xF);

	GET(HouseClass*, pThis, ECX);
	int AIDiff = pThis->AIDifficulty;

	// Westwood, meet my friend the resizable array
	std::vector<int> CreationFrames(AircraftTypeClass::Array->Count, 0);
	std::vector<int> Values(AircraftTypeClass::Array->Count, 0x7FFFFFFF);

	for(int i = 0; i < TeamClass::Array->Count; ++i) {
		TeamClass *CurrentTeam = TeamClass::Array->GetItem(i);
		if(!CurrentTeam || CurrentTeam->Owner != pThis) {
			continue;
		}

		int TeamCreationFrame = CurrentTeam->CreationFrame;

		// what? copy pasting original code, leave it be
		if(
		 (!CurrentTeam->Type->Reinforce || CurrentTeam->unknown_79)
		  &&
		 (!CurrentTeam->unknown_77 || !CurrentTeam->unknown_78)
		) {
			continue;
		}

		DynamicVectorClass<TechnoTypeClass *> TaskForceMembers;
		CurrentTeam->GetTaskForceMembers(&TaskForceMembers);
		for(int j = 0; j < TaskForceMembers.Count; ++j) {
			TechnoTypeClass *CurrentMember = TaskForceMembers[i];
			if(CurrentMember->WhatAmI() != abs_AircraftType) {
				continue;
			}
			int Idx = CurrentMember->GetArrayIndex();
			++Values[Idx];
			if(CreationFrames[Idx] < TeamCreationFrame) {
				CreationFrames[Idx] = TeamCreationFrame;
			}
		}
	}
	
	for(int i = 0; i < AircraftClass::Array->Count; ++i) {
		AircraftClass *A = AircraftClass::Array->GetItem(i);
		int Idx = A->GetArrayIndex();
		if(Values[Idx] > 0 && A->CanBeRecruited(pThis)) {
			--Values[Idx];
		}
	}

	std::vector<int> BestChoices;
	int BestValue = -1;
	int EarliestAircraftIndex = -1;
	int EarliestFrame = 0x7FFFFFFF;

	for(int i = 0; i < AircraftTypeClass::Array->Count; ++i) {
		AircraftTypeClass *AT = AircraftTypeClass::Array->GetItem(i);
		int CurrentValue = Values[i];
		if(CurrentValue <= 0 || !pThis->CanBuild(AT, 0, 0) || AT->GetActualCost(pThis) > pThis->Available_Money()) {
			continue;
		}

		if(CurrentValue < BestValue || CurrentValue == -1) {
			BestValue = CurrentValue;
			BestChoices.clear();
		}
		BestChoices.push_back(i);
		if(EarliestFrame < CreationFrames[i] || EarliestAircraftIndex == -1) {
			EarliestAircraftIndex = i;
			EarliestFrame = CreationFrames[i];
		}
	}

	int EarliestOdds = RulesClass::Global()->get_FillEarliestTeamProbability()->GetItem(AIDiff);
	if(ScenarioClass::Global()->get_Random()->RandomRanged(1, 100) < EarliestOdds) {
		pThis->ProducingAircraftTypeIndex = EarliestAircraftIndex;
		return ret();
	}
	if(BestChoices.size()) {
		int RandomChoice = ScenarioClass::Global()->get_Random()->RandomRanged(0, BestChoices.size() - 1);
		int RandomIndex = BestChoices.at(RandomChoice);
		pThis->ProducingAircraftTypeIndex = RandomIndex;
	}
	return ret();
}
