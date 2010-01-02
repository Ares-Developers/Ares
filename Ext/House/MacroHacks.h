#ifndef CRAZY_MACROS_H
#define CRAZY_MACROS_H

/*
 * This file contains the macro fu needed to make the 100 unit bug fixes work better
 * Don't get it? Don't touch it!
 */

// Westwood, meet my friend the resizable array
// what? copy pasting original code, leave it be
#define CRAZY_MACRO_GO_AWAY_1(retaddr, TypeName)                                                     \
	retfunc_fixed<DWORD> ret(R, retaddr, 0xF);                                                       \
	                                                                                                 \
	GET(HouseClass*, pThis, ECX);                                                                    \
	                                                                                                 \
	if(pThis->Producing ## TypeName ## TypeIndex != -1) {                                            \
		return ret();                                                                                \
	}                                                                                                \
	                                                                                                 \
	int AIDiff = pThis->AIDifficulty;                                                                \


#define CRAZY_MACRO_GO_AWAY_2(TypeName)                                                              \
	std::vector<int> CreationFrames(TypeName ## TypeClass::Array->Count, 0x7FFFFFFF);                \
	std::vector<int> Values(TypeName ## TypeClass::Array->Count, 0);                                 \
	                                                                                                 \
	for(int i = 0; i < TeamClass::Array->Count; ++i) {                                               \
		TeamClass *CurrentTeam = TeamClass::Array->GetItem(i);                                       \
		if(!CurrentTeam || CurrentTeam->Owner != pThis) {                                            \
			continue;                                                                                \
		}                                                                                            \
		                                                                                             \
		int TeamCreationFrame = CurrentTeam->CreationFrame;                                          \
		                                                                                             \
		if(                                                                                          \
		 (!CurrentTeam->Type->Reinforce || CurrentTeam->unknown_79)                                  \
		  &&                                                                                         \
		 (CurrentTeam->unknown_77 || CurrentTeam->unknown_78)                                        \
		) {                                                                                          \
			continue;                                                                                \
		}                                                                                            \
		                                                                                             \
		DynamicVectorClass<TechnoTypeClass *> TaskForceMembers;                                      \
		CurrentTeam->GetTaskForceMembers(&TaskForceMembers);                                         \
		for(int j = 0; j < TaskForceMembers.Count; ++j) {                                            \
			TechnoTypeClass *CurrentMember = TaskForceMembers[j];                                    \
			if(CurrentMember->WhatAmI() != abs_ ## TypeName ## Type) {                               \
				continue;                                                                            \
			}                                                                                        \
			int Idx = CurrentMember->GetArrayIndex();                                                \
			++Values[Idx];                                                                           \
			if(CreationFrames[Idx] < TeamCreationFrame) {                                            \
				CreationFrames[Idx] = TeamCreationFrame;                                             \
			}                                                                                        \
		}                                                                                            \
	}                                                                                                \
	                                                                                                 \
	for(int i = 0; i < TypeName ## Class::Array->Count; ++i) {                                       \
		TypeName ## Class *T = TypeName ## Class::Array->GetItem(i);                                 \
		int Idx = T->GetType()->GetArrayIndex();                                                     \
		if(Values[Idx] > 0 && T->CanBeRecruited(pThis)) {                                            \
			--Values[Idx];                                                                           \
		}                                                                                            \
	}                                                                                                \
	                                                                                                 \
	std::vector<int> BestChoices;                                                                    \
	int BestValue = -1;                                                                              \
	int EarliestTypenameIndex = -1;                                                                  \
	int EarliestFrame = 0x7FFFFFFF;                                                                  \
	                                                                                                 \
	for(int i = 0; i < TypeName ## TypeClass::Array->Count; ++i) {                                   \
		TypeName ## TypeClass *TT = TypeName ## TypeClass::Array->GetItem(i);                        \
		int CurrentValue = Values[i];                                                                \
		if(CurrentValue <= 0 || !pThis->CanBuild(TT, 0, 0)                                           \
		 ||                                                                                          \
		 TT->GetActualCost(pThis) > pThis->Available_Money()                                         \
		) {                                                                                          \
			continue;                                                                                \
		}                                                                                            \
		                                                                                             \
		if(BestValue < CurrentValue || BestValue == -1) {                                            \
			BestValue = CurrentValue;                                                                \
			BestChoices.clear();                                                                     \
		}                                                                                            \
		BestChoices.push_back(i);                                                                    \
		if(EarliestFrame > CreationFrames[i] || EarliestTypenameIndex == -1) {                       \
			EarliestTypenameIndex = i;                                                               \
			EarliestFrame = CreationFrames[i];                                                       \
		}                                                                                            \
	}                                                                                                \
	                                                                                                 \
	int EarliestOdds = RulesClass::Instance->FillEarliestTeamProbability[AIDiff];                    \
	if(ScenarioClass::Instance->Random.RandomRanged(1, 100) < EarliestOdds) {                        \
		pThis->Producing ## TypeName ## TypeIndex = EarliestTypenameIndex;                           \
		return ret();                                                                                \
	}                                                                                                \
	if(BestChoices.size()) {                                                                         \
		int RandomChoice = ScenarioClass::Instance->Random.RandomRanged(0, BestChoices.size() - 1);  \
		int RandomIndex = BestChoices.at(RandomChoice);                                              \
		pThis->Producing ## TypeName ## TypeIndex = RandomIndex;                                     \
	}                                                                                                \
	return ret();                                                                                    \


#endif
