#include "Body.h"

#include <HouseClass.h>
#include <SuperClass.h>

#include <DiscreteSelectionClass.h>

DEFINE_HOOK(5098F0, HouseClass_Update_AI_TryFireSW, 5) {
	GET(HouseClass*, pThis, ECX);

	// this method iterates over every available SW and checks
	// whether it should be fired automatically. the original
	// method would abort if this house is human-controlled.
	bool AIFire = !pThis->ControlledByHuman();

	for(auto pSuper : pThis->Supers) {
		auto pExt = SWTypeExt::ExtMap.Find(pSuper->Type);

		// fire if this is AI owned or the SW has auto fire set.
		if(pSuper->IsCharged && (AIFire || pExt->SW_AutoFire)) {
			CellStruct Cell = CellStruct::Empty;
			auto LaunchSW = [&](const CellStruct &Cell) {
				int idxSW = pThis->Supers.FindItemIndex(pSuper);
				pThis->Fire_SW(idxSW, Cell);
			};

			// don't try to fire if we obviously haven't enough money
			if(pExt->Money_Amount < 0) {
				if(pThis->Available_Money() < -pExt->Money_Amount) {
					continue;
				}
			}

			// all the different AI targeting modes
			switch(pExt->SW_AITargetingType) {
			case SuperWeaponAITargetingMode::Nuke:
			{
				if(pThis->EnemyHouseIndex != -1) {
					if(pThis->PreferredTargetCell == CellStruct::Empty) {
						Cell = (pThis->PreferredTargetType == TargetType::Anything)
							? pThis->PickIonCannonTarget()
							: pThis->PickTargetByType(pThis->PreferredTargetType);
					} else {
						Cell = pThis->PreferredTargetCell;
					}

					if(Cell != CellStruct::Empty) {
						LaunchSW(Cell);
					}
				}
				break;
			}

			case SuperWeaponAITargetingMode::LightningStorm:
			{
				pThis->Fire_LightningStorm(pSuper);
				break;
			}

			case SuperWeaponAITargetingMode::PsychicDominator:
			{
				pThis->Fire_PsyDom(pSuper);
				break;
			}

			case SuperWeaponAITargetingMode::ParaDrop:
			{
				pThis->Fire_ParaDrop(pSuper);
				break;
			}

			case SuperWeaponAITargetingMode::GeneticMutator:
			{
				pThis->Fire_GenMutator(pSuper);
				break;
			}

			case SuperWeaponAITargetingMode::ForceShield:
			{
				if(pThis->PreferredDefensiveCell2 == CellStruct::Empty) {
					if(pThis->PreferredDefensiveCell != CellStruct::Empty
						&& RulesClass::Instance->AISuperDefenseFrames + pThis->PreferredDefensiveCellStartTime > Unsorted::CurrentFrame) {
						Cell = pThis->PreferredDefensiveCell;
					}
				} else {
					Cell = pThis->PreferredDefensiveCell2;
				}

				if(Cell != CellStruct::Empty) {
					LaunchSW(Cell);
					pThis->PreferredDefensiveCell = CellStruct::Empty;
				}
				break;
			}

			case SuperWeaponAITargetingMode::Offensive:
			{
				if(pThis->EnemyHouseIndex != -1 && pExt->IsHouseAffected(pThis, HouseClass::Array->GetItem(pThis->EnemyHouseIndex))) {
					Cell = pThis->PickIonCannonTarget();
					if(Cell != CellStruct::Empty) {
						LaunchSW(Cell);
					}
				}
				break;
			}

			case SuperWeaponAITargetingMode::NoTarget:
			{
				LaunchSW(Cell);
				break;
			}

			case SuperWeaponAITargetingMode::Stealth:
			{
				// find one of the cloaked enemy technos, posing the largest threat.
				DiscreteSelectionClass<TechnoClass*> list;
				for(auto pTechno : *TechnoClass::Array) {
					if(pTechno->CloakState != CloakState::Uncloaked) {
						if(pExt->IsHouseAffected(pThis, pTechno->Owner)) {
							if(pExt->IsTechnoAffected(pTechno)) {
								int value = pTechno->GetTechnoType()->ThreatPosed;
								list.Add(pTechno, value);
							}
						}
					}
				}
				if(auto pTarget = list.Select(ScenarioClass::Instance->Random)) {
					Cell = pTarget->GetCell()->MapCoords;
					LaunchSW(Cell);
				}
				break;
			}

			case SuperWeaponAITargetingMode::Base:
			{
				// fire at the SW's owner's base cell
				Cell = pThis->Base_Center();
				LaunchSW(Cell);
				break;
			}

			case SuperWeaponAITargetingMode::Self:
			{
				// find the first building providing pSuper
				SuperWeaponTypeClass *pType = pSuper->Type;
				BuildingClass *pBld = nullptr;
				for(auto pTBld : *BuildingTypeClass::Array) {
					if(pTBld->HasSuperWeapon(pType->ArrayIndex)) {
						pBld = pThis->FindBuildingOfType(pTBld->ArrayIndex, -1);
						if(pBld != nullptr) {
							break;
						}
					}
				}
				if(pBld) {
					CellStruct Offset = CellStruct{pBld->Type->GetFoundationWidth() / 2,
						pBld->Type->GetFoundationHeight(false) / 2};
					Cell = pBld->GetCell()->MapCoords + Offset;
					LaunchSW(Cell);
				}

				break;
			}
			}
		}
	}

	return 0x509AE7;
}
