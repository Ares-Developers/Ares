#include "Body.h"
#include "../Side/Body.h"

#include <GameModeOptionsClass.h>

void HouseExt::ExtData::ReplanBase() {
	HouseClass * H = this->AttachedToObject;

	HouseTypeClass * HT = H->Type;

	int SideIndex = HT->SideIndex;

	DWORD OwnerBits = 1 << HouseTypeClass::FindIndexOfName(HT->ParentCountry);

	DynamicVectorClass<BuildingTypeClass *> CanBuildEver;
	DynamicVectorClass<bool> PlannedToBuild;

	for(int i = 0; i < BuildingTypeClass::Array->Count; ++i) {
		BuildingTypeClass * B = BuildingTypeClass::Array->GetItem(i);
		if((B->OwnerFlags & OwnerBits) != 0) {
			int BasePlanSide = B->AIBasePlanningSide;
			if(BasePlanSide == SideIndex || BasePlanSide == -1) {
				if(B->AIBuildThis) {
					if(B->TechLevel <= H->TechLevel) {
						if(H->InRequiredHouses(B) && !H->InForbiddenHouses(B)) {
							int swIdx = B->SuperWeapon;
							bool OK = swIdx == -1 || GameModeOptionsClass::Instance->SWAllowed;
							if(!OK) {
								OK = RulesClass::Instance->BuildTech.FindItemIndex(&B) != -1;
								if(!OK) {
									OK = !H->Supers[swIdx]->Type->DisableableFromShell;
								}
							}
							if(OK) {
								CanBuildEver.AddItem(B);
								PlannedToBuild.AddItem(false);
							}
						}
					}
				}
			}
		}
	}

	DynamicVectorClass<BuildingTypeClass *> GoingToBuild;

	auto AddFirstSuitable = [&](DynamicVectorClass<BuildingTypeClass *>* choices, bool checkPrereqs, signed int swapWith) {
		if(BuildingTypeClass * Chosen = H->FirstBuildableFromArray(choices)) {
			bool canAdd = !checkPrereqs;
			int idx = -1;
			if(!canAdd) {
				idx = CanBuildEver.FindItemIndex(&Chosen);
				canAdd = idx != -1;
			}
			if(canAdd) {
				GoingToBuild.AddItem(Chosen);
				if(checkPrereqs) {
					PlannedToBuild[idx] = true;
					if(swapWith != -1) {
						// showtime! why do you do this, WW?
						PlannedToBuild[swapWith] = PlannedToBuild[idx];
						PlannedToBuild[idx] = false;

						BuildingTypeClass * B = CanBuildEver[swapWith];
						CanBuildEver[swapWith] = CanBuildEver[idx];
						CanBuildEver[idx] = B;
					}
				}
			}
		}
	};

	AddFirstSuitable(&RulesClass::Instance->BuildConst, true, -1);
	AddFirstSuitable(&RulesClass::Instance->BuildPower, false, -1);
	AddFirstSuitable(&RulesClass::Instance->BuildBarracks, true, 0);
	AddFirstSuitable(&RulesClass::Instance->BuildWeapons, true, 1);

	int attemptsRemaining = CanBuildEver.Count;
	while(attemptsRemaining > 0) {
		bool Changed = false;
		for(int i = 0; i < CanBuildEver.Count; ++i) {
			if(!PlannedToBuild[i]) {
				BuildingTypeClass * const curType = CanBuildEver[i];
				if(H->AllPrerequisitesAvailable(curType, &GoingToBuild, GoingToBuild.Count)) {
					PlannedToBuild[i] = true;
					GoingToBuild.AddItem(curType);
					Changed = true;
					--attemptsRemaining;
				}
			}
		}

		if(!Changed) {
			break;
		}
	}

	UnitTypeClass * Harvester = NULL;
	int CountExtraRefineries = 0;
	for(int i = 0; i < RulesClass::Instance->HarvesterUnit.Count; ++i) {
		UnitTypeClass * Harv = RulesClass::Instance->HarvesterUnit[i];
		if((Harv->OwnerFlags & OwnerBits) != 0) {
			Harvester = Harv;
			break;
		}
	}

	if(Harvester) {
		CountExtraRefineries = RulesClass::Instance->AIExtraRefineries[H->AIDifficulty];
	} else {
		CountExtraRefineries = RulesClass::Instance->AISlaveMinerNumber[H->AIDifficulty] - 1;
	}

	if(CountExtraRefineries > 0) {
		if(BuildingTypeClass * Refinery = H->FirstBuildableFromArray(&RulesClass::Instance->BuildRefinery)) {
			signed int idxRef = GoingToBuild.FindItemIndex(&Refinery);
			if(idxRef != -1) {
				do {
					int insertAfter = ScenarioClass::Instance->Random.RandomRanged(idxRef, GoingToBuild.Count - 1);
					GoingToBuild.AddItem(NULL);
					if(insertAfter < GoingToBuild.Count - 1) {
						// manual insert, whoo
						memcpy(
							&GoingToBuild.Items[insertAfter + 2],
							&GoingToBuild.Items[insertAfter + 1],
							(GoingToBuild.Count - 1 - insertAfter) * 4
						);
					}
					GoingToBuild[insertAfter + 1] = Refinery;
					--CountExtraRefineries;
				} while(CountExtraRefineries);
			}
		}
	}

	// westwood did a full vector copy here for no reason

	int BaseDefenseCount = 0;
	if(SideExt::ExtData * pSideData = SideExt::ExtMap.Find(SideClass::Array->GetItem(SideIndex))) {
		BaseDefenseCount = pSideData->BaseDefenseCounts[H->AIDifficulty];
	}

	if(BaseDefenseCount > 0) {
		do {
			int insertAfter = ScenarioClass::Instance->Random.RandomRanged(3, GoingToBuild.Count - 1);
			GoingToBuild.AddItem(NULL);
			if(insertAfter < GoingToBuild.Count - 1) {
				memcpy(
					&GoingToBuild.Items[insertAfter + 2],
					&GoingToBuild.Items[insertAfter + 1],
					(GoingToBuild.Count - 1 - insertAfter) * 4
				);
			}
			GoingToBuild[insertAfter + 1] = (BuildingTypeClass * const)-1; // MAGIC UP THE WAZOO
			--BaseDefenseCount;
		} while(BaseDefenseCount);
	}

	BaseClass * Base = &H->Base;
	auto BaseNodes = &Base->BaseNodes;
	BaseNodes->Clear();

	for(int i = 0; i < GoingToBuild.Count; ++i) {
		// your mind is about to be blown... again
		BuildingTypeClass * plannedBuilding = GoingToBuild[i];
		BaseNodeClass Node;
		Node.BuildingTypeIndex = 0;
		Node.MapCoords.X = Node.MapCoords.Y = 0;
		Node.Attempts = 0;
		Node.Placed = 0; // meaningful
		bool validNode = false;
		signed int plannedBuildingIdx = reinterpret_cast<signed int>(plannedBuilding); // yeehaaaaaaaaaaaaaaw
		if(plannedBuildingIdx >= 0 || plannedBuildingIdx < -4) {
			if(plannedBuilding) {
				Node.BuildingTypeIndex = plannedBuilding->ArrayIndex;
				validNode = true;
			}
		} else {
			Node.BuildingTypeIndex = plannedBuildingIdx;
			validNode = true;
		}
		if(validNode) {
			BaseNodes->AddItem(Node);
		}
	}
}

DEFINE_HOOK(5054B0, HouseClass_GenerateAIBuildList, 6)
{
	GET(HouseClass *, pHouse, ECX);
	auto pData = HouseExt::ExtMap.Find(pHouse);
	pData->ReplanBase();
	return 0x505F72;
}

DEFINE_HOOK(4F65BF, HouseClass_4F6540, 6)
{
	GET(UnitTypeClass *, BaseUnit, ECX);
	return (BaseUnit)
		? 0
		: 0x4F65DA
	;
}
