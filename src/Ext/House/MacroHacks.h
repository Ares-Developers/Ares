#pragma once

/*
 * This file contains the macro fu needed to make the 100 unit bug fixes work better
 * Don't get it? Don't touch it!
 */

// Westwood, meet my friend the resizable array
// what? copy pasting original code, leave it be
template <class TClass, class TType>
void GetTypeToProduce(HouseClass* pThis, int& ProducingTypeIndex) {
	auto& CreationFrames = HouseExt::AIProduction_CreationFrames;
	auto& Values = HouseExt::AIProduction_Values;
	auto& BestChoices = HouseExt::AIProduction_BestChoices;

	auto const count = static_cast<unsigned int>(TType::Array->Count);
	CreationFrames.assign(count, 0x7FFFFFFF);
	Values.assign(count, 0);

	for(auto CurrentTeam : *TeamClass::Array) {
		if(!CurrentTeam || CurrentTeam->Owner != pThis) {
			continue;
		}

		int TeamCreationFrame = CurrentTeam->CreationFrame;

		if((!CurrentTeam->Type->Reinforce || CurrentTeam->unknown_79)
		  && (CurrentTeam->unknown_77 || CurrentTeam->unknown_78))
		{
			continue;
		}

		DynamicVectorClass<TechnoTypeClass *> TaskForceMembers;
		CurrentTeam->GetTaskForceMissingMemberTypes(TaskForceMembers);
		for(auto CurrentMember : TaskForceMembers) {
			if(CurrentMember->WhatAmI() != TType::AbsID) {
				continue;
			}
			auto const Idx = static_cast<unsigned int>(CurrentMember->GetArrayIndex());
			++Values[Idx];
			if(TeamCreationFrame < CreationFrames[Idx]) {
				CreationFrames[Idx] = TeamCreationFrame;
			}
		}
	}

	for(auto T : *TClass::Array) {
		auto const Idx = static_cast<unsigned int>(T->GetType()->GetArrayIndex());
		if(Values[Idx] > 0 && T->CanBeRecruited(pThis)) {
			--Values[Idx];
		}
	}

	BestChoices.clear();

	int BestValue = -1;
	int EarliestTypenameIndex = -1;
	int EarliestFrame = 0x7FFFFFFF;

	for(auto i = 0u; i < count; ++i) {
		auto const TT = TType::Array->Items[static_cast<int>(i)];
		int CurrentValue = Values[i];
		if(CurrentValue <= 0 || !pThis->CanBuild(TT, false, false)
			|| TT->GetActualCost(pThis) > pThis->Available_Money())
		{
			continue;
		}

		if(BestValue < CurrentValue || BestValue == -1) {
			BestValue = CurrentValue;
			BestChoices.clear();
		}
		BestChoices.push_back(static_cast<int>(i));
		if(EarliestFrame > CreationFrames[i] || EarliestTypenameIndex == -1) {
			EarliestTypenameIndex = static_cast<int>(i);
			EarliestFrame = CreationFrames[i];
		}
	}

	auto const AIDiff = static_cast<int>(pThis->GetAIDifficultyIndex());
	int EarliestOdds = RulesClass::Instance->FillEarliestTeamProbability[AIDiff];
	if(ScenarioClass::Instance->Random.RandomRanged(0, 99) < EarliestOdds) {
		ProducingTypeIndex = EarliestTypenameIndex;
	} else if(auto const size = static_cast<int>(BestChoices.size())) {
		int RandomChoice = ScenarioClass::Instance->Random.RandomRanged(0, size - 1);
		ProducingTypeIndex = BestChoices[static_cast<unsigned int>(RandomChoice)];
	}
}
