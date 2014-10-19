#include "Body.h"
#include "../TechnoType/Body.h"

#include <AirstrikeClass.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <VocClass.h>
#include <VoxClass.h>

// #346, #464, #970, #1014
// handle all veterancy gains ourselves
DEFINE_HOOK(702E9D, TechnoClass_RegisterDestruction_Veterancy, 6) {
	GET(TechnoClass*, pKiller, EDI);
	GET(TechnoClass*, pVictim, ESI);
	GET(const int, VictimCost, EBP);

	// get the unit that receives veterancy
	TechnoClass* pExperience = nullptr;
	double ExpFactor = 1.0;

	// before we do any other logic, check if this kill was committed by an
	// air strike and its designator shall get the experience. 
	if(pKiller->Airstrike) {
		if(TechnoClass* pDesignator = pKiller->Airstrike->Owner) {
			if(TechnoTypeExt::ExtData *pTDesignatorData = TechnoTypeExt::ExtMap.Find(pDesignator->GetTechnoType())) {
				if(pTDesignatorData->ExperienceFromAirstrike) {
					pKiller = pDesignator;
					ExpFactor *= pTDesignatorData->AirstrikeExperienceModifier;
				}
			}
		}
	}

	// get some values that are needed multiple times
	TechnoTypeClass* pTKiller = pKiller->GetTechnoType();
	TechnoClass* pTransporter = pKiller->Transporter;
	TechnoTypeExt::ExtData *pTKillerData = TechnoTypeExt::ExtMap.Find(pTKiller);
	TechnoTypeExt::ExtData *pTTransporterData = nullptr;
	bool TransporterAndKillerAllied = true;
	bool promoteImmediately = false;
	if(pTransporter) {
		pTTransporterData = TechnoTypeExt::ExtMap.Find(pTransporter->GetTechnoType());
		TransporterAndKillerAllied = pTransporter->Owner->IsAlliedWith(pKiller);
	}

	if(pKiller->InOpenToppedTransport && pTransporter) {
		// check for passenger of an open topped vehicle. transporter can get
		// experience from passengers; but only if the killer and its transporter
		// are allied. so a captured opentopped vehicle won't get experience from
		// the enemy's orders.
	
		// if passengers can get promoted and this transport is already elite,
		// don't promote this transport in favor of the real killer.
		TechnoTypeClass* pTTransporter = pTransporter->GetTechnoType();
		if((!pTTransporter->Trainable || pTTransporterData->PassengersGainExperience) && (pTransporter->Veterancy.IsElite() || !TransporterAndKillerAllied) && pTKiller->Trainable) {
			// the passenger gets experience
			pExperience = pKiller;
			ExpFactor *= pTTransporterData->PassengerExperienceModifier;
		} else if(pTTransporter->Trainable && pTTransporterData->ExperienceFromPassengers && TransporterAndKillerAllied) {
			// the transporter gets experience
			pExperience = pTransporter;
		}

	} else if(pTKiller->Gunner) {
		// an IFV can get experience, too, but we have to have an extra check
		// because the gunner is not the killer.
		FootClass* pGunner = pKiller->Passengers.GetFirstPassenger();
		if(pTKiller->Trainable && !pKiller->Veterancy.IsElite() && (!pGunner || pTKillerData->ExperienceFromPassengers)) {
			// the IFV gets credited
			pExperience = pKiller;
		} else if(pGunner && (pKiller->Veterancy.IsElite() || !pTKillerData->ExperienceFromPassengers) && pGunner->GetTechnoType()->Trainable && pTKillerData->PassengersGainExperience) {
			pExperience = pGunner;
			ExpFactor *= pTKillerData->PassengerExperienceModifier;
			promoteImmediately = true;
		}

	} else if(pTKiller->Trainable) {
		// the killer itself gets credited.
		pExperience = pKiller;

	} else if(pTKiller->MissileSpawn) {
		// unchanged game logic
		if(TechnoClass* pSpawner = pKiller->SpawnOwner) {
			TechnoTypeClass* pTSpawner = pSpawner->GetTechnoType();
			if(pTSpawner->Trainable) {
				pExperience = pSpawner;
			}
		}

	} else if(pKiller->CanOccupyFire()) {
		// game logic, with added check for Trainable
		if(BuildingClass* pKillerBld = specific_cast<BuildingClass *>(pKiller)) {
			InfantryClass* pOccupant = pKillerBld->Occupants[pKillerBld->FiringOccupantIndex];
			if(pOccupant->Type->Trainable) {
				pExperience = pOccupant;
			}
		}
	}

	// update the veterancy
	if(pExperience) {

		// no way to get experience by proxy by an enemy unit. you cannot
		// promote your mind-controller by capturing friendly units.
		if(pExperience->Owner->IsAlliedWith(pKiller)) {

			auto AddExperience = [](TechnoClass* pTechno, int victimCost, double factor) {
				auto TechnoCost = pTechno->GetTechnoType()->GetActualCost(pTechno->Owner);
				auto WeightedVictimCost = static_cast<int>(victimCost * factor);
				if(TechnoCost > 0 && WeightedVictimCost > 0) {
					pTechno->Veterancy.Add(TechnoCost, WeightedVictimCost);
				}
			};

			// if this is a non-missile spawn, handle the spawn manually and switch over to the
			// owner then. this way, a mind-controlled owner is supported.
			TechnoClass* pSpawn = nullptr;
			double SpawnFactor = 1.0;
			if(auto pSpawner = pExperience->SpawnOwner) {
				auto pTSpawner = pSpawner->GetTechnoType();

				if(!pTSpawner->MissileSpawn && pTSpawner->Trainable) {
					auto pTSpawnerData = TechnoTypeExt::ExtMap.Find(pTSpawner);

					// add experience to the spawn. this is done later so mind-control
					// can be factored in.
					SpawnFactor = pTSpawnerData->SpawnExperienceSpawnModifier;
					pSpawn = pExperience;

					// switch over to spawn owners, and factor in the spawner multiplier
					ExpFactor *= pTSpawnerData->SpawnExperienceOwnerModifier;
					pExperience = pSpawner;
				}
			}

			// mind-controllers get experience, too.
			if(auto pController = pExperience->MindControlledBy) {
				if(!pController->Owner->IsAlliedWith(pVictim->Owner)) {

					// get the mind controllers extended properties
					auto pTController = pController->GetTechnoType();
					auto pTControllerData = TechnoTypeExt::ExtMap.Find(pTController);

					// promote the mind-controller
					if(pTController->Trainable) {
						// the mind controller gets its own factor
						auto ControllerFactor = ExpFactor * pTControllerData->MindControlExperienceSelfModifier;
						AddExperience(pController, VictimCost, ControllerFactor);
					}

					// modify the cost of the victim.
					ExpFactor *= pTControllerData->MindControlExperienceVictimModifier;
				}
			}

			// default. promote the unit this function selected.
			AddExperience(pExperience, VictimCost, ExpFactor);

			// if there is a spawn, let it get its share.
			if(pSpawn) {
				AddExperience(pSpawn, VictimCost, ExpFactor * SpawnFactor);
			}

			// gunners need to be promoted manually, or they won't only get
			// the experience until after they exited their transport once.
			if(promoteImmediately) {
				auto newRank = pExperience->Veterancy.GetRemainingLevel();

				if(pExperience->CurrentRanking != newRank) {
					if(pExperience->CurrentRanking != Rank::Invalid) {
						int sound = -1;
						if(pExperience->Veterancy.IsVeteran()) {
							sound = RulesClass::Instance->UpgradeVeteranSound;
						} else if (pExperience->Veterancy.IsElite()) {
							sound = RulesClass::Instance->UpgradeEliteSound;
						}

						if(pExperience->Owner->ControlledByHuman()) {
							VocClass::PlayAt(sound, pExperience->Transporter->Location, nullptr);
							VoxClass::Play("EVA_UnitPromoted");
						}
					}

					pExperience->CurrentRanking = newRank;
				}
			}
		}
	}

	// skip the entire veterancy handling
	return 0x702FF5;
}
