/* FlyingStrings.cpp - controls floating messages, used by Bounty
Logic by Joshy
INI controls are by Graion Dilach
Beware! This file has an unfinished experience per player counting as well.
	(xp and last hook)
Commented that one out.
*/

//Don't ask me which one is needed, which one isn't - Graion
#include "ASMMacros.h"
#include "Syringe.h"
#include "FlyingStrings.h"
#include "../Ares.h"
#include "../Misc/Debug.h"
#include "../Ext/House/Body.h"
#include "../Ext/TechnoType/Body.h" //Added the Bounty tags into this one - Graion
#include "../Ext/House/Body.h"
#include <HouseClass.h>
#include <HouseTypeClass.h>
#include <Commands/Commands.h>
#include <CCINIClass.h>
#include <SlaveManagerClass.h>

DynamicVectorClass<FlyingStrings::Item> FlyingStrings::data;

//int xp;

DEFINE_HOOK(702E64, RecordTheKill_CalculateBountyXP, 6) {

	//Get object being killed
	GET(TechnoClass *, Victim, ESI);
	TechnoTypeClass * pTypeVictim = Victim->GetTechnoType();
	TechnoTypeExt::ExtData * pTypeVictimExt = TechnoTypeExt::ExtMap.Find(pTypeVictim);
	HouseClass *pOwnerVictim = Victim->Owner;

	//Get object that kills
	GET(TechnoClass *, Killer, EDI);

	//Explosion crates? SWs?
	if(Killer) {
		TechnoTypeClass * pTypeKiller = Killer->GetTechnoType();
		TechnoTypeExt::ExtData * pTypeKillerExt = TechnoTypeExt::ExtMap.Find(pTypeKiller);

		if (pOwnerVictim->IsAlliedWith(Killer->Owner)) {
			if(BuildingClass * pBld = specific_cast<BuildingClass*>(Killer)) {
				//Armory - Hospital - logic check, yay for TS leftovers
				//Right now I won't care, anybody for 306 feel free to start from here 
				if(pBld->Type->Armory || pBld->Type->Hospital) {
					return 0;
				}
			}
		}



		if (pTypeKillerExt->Bounty_Modifier == 0 && pTypeKillerExt->Bounty_FriendlyModifier == 0) {
			return 0; //No bounty activated
		} else {
			if (!pTypeVictimExt->ImmuneToBounty) {
				if (!pTypeKillerExt->Bounty_Pillager) {
					int victimActualCost = pTypeVictim->GetActualCost(pOwnerVictim);

					FootClass *F = generic_cast<FootClass *>(Killer);
					CoordStruct coords;
					F->GetCoords(&coords);
					Point2D point;
					TacticalClass::Instance->CoordsToClient(&coords, &point);

					//Alliance check
					if(!pOwnerVictim->IsAlliedWith(Killer->Owner)) {
						victimActualCost *= pTypeKillerExt->Bounty_Modifier;

						if (victimActualCost != 0){
							Killer->Owner->GiveMoney(victimActualCost);
							Debug::Log("[Bounty] %ls's %s destroyed an enemy %s, gained %d as Bounty.", Killer->Owner->UIName, pTypeKiller->ID, pTypeVictim->ID, victimActualCost);
							//xp = xp + victimActualCost * Killer->Bounty_Modifier;

							if (!!pTypeKillerExt->Bounty_Message){ //output message only if we want to - Graion
								wchar_t Message[256];
								_snwprintf(Message, 256, L"+$%d", victimActualCost);

								FlyingStrings::Add(Message, point, COLOR_GREEN);
							}
						}
					} else { //Whoops, allied!
						if (pTypeKiller->Enslaves) {
							//Slave Miners, I hate ya.
							if (pTypeVictim->Slaved && pTypeVictim == pTypeKiller->Enslaves) {
								return 0;
							}
						}

						victimActualCost *= pTypeKillerExt->Bounty_FriendlyModifier;

						if (victimActualCost != 0){
							Killer->Owner->TakeMoney(victimActualCost);
							Debug::Log("[Bounty] %ls's %s destroyed an ally %s, lost %d as Bounty.", Killer->Owner->UIName, pTypeKiller->ID, pTypeVictim->ID, victimActualCost);
								
							if (!!pTypeKillerExt->Bounty_FriendlyMessage){ //output message only if we want to - Graion
								wchar_t Message2[256];
								_snwprintf(Message2, 256, L"-$%d", victimActualCost);

								FlyingStrings::Add(Message2, point, COLOR_RED);
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
	
/*Notes
Checks done: 
Armory (306?), Hospital, Slave Miner.
Checks undone-unknown
Temporal, DeathWeapon?
*/

// 1523, Get money per Damage! Replaced Joshy's hook with D's
//A_FINE_HOOK(701DCC,Damage_EBX-Victim_ESI-Attacker_StackC8,7)
//Kept for record, once this one may comes helpful as well
DEFINE_HOOK(701DFF, TechnoClass_ReceiveDamage_BountyPillage, 7) {
	//Get object being damaged
	GET(TechnoClass *, Victim, ESI);
	TechnoTypeClass * pTypeVictim = Victim->GetTechnoType();
	TechnoTypeExt::ExtData * pTypeVictimExt = TechnoTypeExt::ExtMap.Find(pTypeVictim);
	HouseClass *pOwnerVictim = Victim->Owner;

	//Get actual damage
	GET(int*, pDamage, EBX);
		
	//Get object that damages
	GET_STACK(TechnoClass*, Attacker, 0xC8);

	//Explosion crates? SWs?
	if (Attacker) {
		TechnoTypeClass * pTypeAttacker = Attacker->GetTechnoType();
		TechnoTypeExt::ExtData * pTypeAttackerExt = TechnoTypeExt::ExtMap.Find(pTypeAttacker);

		if (pOwnerVictim->IsAlliedWith(Attacker->Owner)) {
			if(BuildingClass * pBld = specific_cast<BuildingClass*>(Attacker)) {
				//Armory - Hospital - I guess they deliver damage as well
				if(pBld->Type->Armory || pBld->Type->Hospital) {
					return 0;
				}
			}
		}

		if (pTypeAttackerExt->Bounty_Modifier == 0 && pTypeAttackerExt->Bounty_FriendlyModifier == 0) {
			return 0; //No bounty activated
		} else {
			if (!pTypeVictimExt->ImmuneToBounty){
				if (!!pTypeAttackerExt->Bounty_Pillager) {
					int PillageMoney = *pDamage;

					FootClass *F = generic_cast<FootClass *>(Attacker);
					CoordStruct coords;
					F->GetCoords(&coords);
					Point2D point;
					TacticalClass::Instance->CoordsToClient(&coords, &point);

					//Alliance check
					if(!pOwnerVictim->IsAlliedWith(Attacker->Owner)) {
						PillageMoney *= pTypeAttackerExt->Bounty_Modifier;

						if (PillageMoney != 0) {
							Attacker->Owner->GiveMoney(PillageMoney);
							Debug::Log("[Pillage] %ls's %s delivered %d Damage to an enemy %s, gained %d as Pillage.", Attacker->Owner->UIName, pTypeAttacker->ID, pDamage, pTypeVictim->ID, PillageMoney);
							//xp = xp + PillageMoney * Attacker->Bounty_Modifier;

							if (!!pTypeAttackerExt->Bounty_Message) { //output message only if we want to - Graion
								wchar_t Message[256];
								_snwprintf(Message, 256, L"+$%d", PillageMoney);

								FlyingStrings::Add(Message, point, COLOR_GREEN);
							}
						}
					} else { //Whoops, allied!
						if (pTypeAttacker->Enslaves) {
							//Slave Miners, I hate ya.
							if (pTypeVictim->Slaved && pTypeVictim == pTypeAttacker->Enslaves) {
								return 0;
							}
						}

						PillageMoney *= pTypeAttackerExt->Bounty_FriendlyModifier;

						if (PillageMoney != 0){
							Attacker->Owner->TakeMoney(PillageMoney);
							Debug::Log("[Pillage] %ls's %s delivered %d Damage to an ally %s, lost %d as Pillage.", Attacker->Owner->UIName, pTypeAttacker->ID, pDamage, pTypeVictim->ID, PillageMoney);

							if (!!pTypeAttackerExt->Bounty_FriendlyMessage){ //output message only if we want to - Graion
								wchar_t Message2[256];
								_snwprintf(Message2, 256, L"-$%d", PillageMoney);

								FlyingStrings::Add(Message2, point, COLOR_RED);
							}
						}
					}
				}
			}
		}
	}
	return 0;
}


DEFINE_HOOK(4F4558, FlyingStrings_Update, 5) {
	FlyingStrings::UpdateAll();
	return 0;
}

/*
Maybe it should be dropped out totally, but I won't be the one who decides it
A_FINE_HOOK(5C9A44, TotalExperience, 6){
	CCFileClass file("RA2MD.INI");
	CCINIClass pINI;
	pINI.ReadCCFile(&file);

	pINI.WriteInteger("Network", "Xp.Total", 5608945, true);
	return 0;
}
*/
