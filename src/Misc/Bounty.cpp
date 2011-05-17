/* FlyingStrings.cpp - controls floating messages, used by Bounty
Logic by Joshy
INI controls are by Graion Dilach
Beware! This file has an unfinished experience per player counting as well.
	(xp and last hook)
Commented that one out.
*/

//Don't ask me which one is needed, which one isn't - Graion
#include "FlyingStrings.h"
#include "Debug.h"
#include "../Ares.h"
#include "../Ext/House/Body.h"
#include "../Ext/HouseType/Body.h"
#include "../Ext/Techno/Body.h"
#include "../Ext/TechnoType/Body.h" //Added the Bounty tags into this one - Graion
#include <SlaveManagerClass.h>

DynamicVectorClass<FlyingStrings::Item> FlyingStrings::data;

void BountyMessageOutput(TechnoClass* Messager, int Amount) {
	CoordStruct coords;
	Messager->GetCoords(&coords);
	wchar_t Message[256];
	if (Amount>0){
		_snwprintf(Message, 256, L"+$%d", Amount);
		FlyingStrings::Add(Message, &coords, COLOR_GREEN);
	} else {
		_snwprintf(Message, 256, L"-$%d", -Amount);
		FlyingStrings::Add(Message, &coords, COLOR_RED);
	}
}

DEFINE_HOOK(702E64, TechnoClass_CalculateBountyAtKill, 6) {

	//Get object being killed
	GET(TechnoClass *, Victim, ESI);
	TechnoTypeClass * pTypeVictim = Victim->GetTechnoType();
	//TechnoTypeExt::ExtData * pTypeVictimExt = TechnoTypeExt::ExtMap.Find(pTypeVictim);
	HouseClass *pOwnerVictim = Victim->Owner;
	TechnoExt::ExtData *pVictimExt = TechnoExt::ExtMap.Find(Victim);

	//Get object that kills
	GET(TechnoClass *, Killer, EDI);

	//Explosion crates? SWs?
	if(Killer) {
		TechnoTypeClass * pTypeKiller = Killer->GetTechnoType();
		//TechnoTypeExt::ExtData * pTypeKillerExt = TechnoTypeExt::ExtMap.Find(pTypeKiller);
		TechnoExt::ExtData * pKillerExt = TechnoExt::ExtMap.Find(Killer);

		if (pOwnerVictim->IsAlliedWith(Killer->Owner)) {
			if(BuildingClass * pBld = specific_cast<BuildingClass*>(Killer)) {
				//Armory - Hospital - logic check, yay for TS leftovers
				//Right now I won't care, anybody for 306 feel free to start from here 
				//Or maybe not, D said something else
				if(pBld->Type->Armory || pBld->Type->Hospital) {
					return 0;
				}
			}
		}


		auto KillerModifier = pKillerExt->Get_Bounty_Modifier();
		auto KillerFriendlyMod = pKillerExt->Get_Bounty_FriendlyModifier();
		auto KillerPillager = pKillerExt->Get_Bounty_Pillager();
		auto KillerMessage = pKillerExt->Get_Bounty_Message();
		auto KillerFriendlyMsg = pKillerExt->Get_Bounty_FriendlyMessage();

		auto VictimCostMult = pVictimExt->Get_Bounty_CostMultiplier();

		if (KillerModifier == 0 && KillerFriendlyMod == 0) {
			return 0; //No bounty activated
		} else {
			if (VictimCostMult) {
				if (!KillerPillager) {
					auto victimActualCost = int(pTypeVictim->GetCost() * VictimCostMult);

					//Alliance check
					if(!pOwnerVictim->IsAlliedWith(Killer->Owner)) {
						victimActualCost *= KillerModifier;

						if (victimActualCost != 0){
							Killer->Owner->GiveMoney(victimActualCost);
							Debug::Log("[Bounty] %ls's %s destroyed an enemy %s, gained %d as Bounty.", Killer->Owner->UIName, pTypeKiller->ID, pTypeVictim->ID, victimActualCost);
							//xp = xp + victimActualCost * Killer->Bounty_Modifier;

							if (KillerMessage) { //output message only if we want to - Graion
								if (pTypeKiller->MissileSpawn && Killer->SpawnOwner) { //V3, Dred
									BountyMessageOutput(Killer->SpawnOwner, victimActualCost);
								} else {
									BountyMessageOutput(Killer, victimActualCost);
								}
							}
						}
					} else { //Whoops, allied!
						if (pTypeKiller->Enslaves) {
							//Slave Miners, I hate ya.
							if (pTypeVictim->Slaved && pTypeVictim == pTypeKiller->Enslaves) {
								return 0;
							}
						}

						victimActualCost *= KillerFriendlyMod;

						if (victimActualCost != 0){
							Killer->Owner->TakeMoney(victimActualCost);
							Debug::Log("[Bounty] %ls's %s destroyed an ally %s, lost %d as Bounty.", Killer->Owner->UIName, pTypeKiller->ID, pTypeVictim->ID, victimActualCost);
								
							if (KillerFriendlyMsg) {//output message only if we want to - Graion
								if (pTypeKiller->MissileSpawn && Killer->SpawnOwner) { //V3, Dred
									BountyMessageOutput (Killer->SpawnOwner, -victimActualCost);
								} else {
									BountyMessageOutput (Killer, -victimActualCost);
								}
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
Armory (306?), Hospital, Slave Miner, MissileSpawn, Temporal, Crush
Checks haven't done
Sonic, ROF
*/

// 1523, Get money per Damage! Replaced Joshy's hook with D's
//A_FINE_HOOK(701DCC, Damage_EBX-Victim_ESI-Attacker_StackC8, 7)
//Kept for record, once this one may comes helpful as well
DEFINE_HOOK(701DFF, TechnoClass_ReceiveDamage_BountyPillage, 7) {
	//Get object being damaged
	GET(TechnoClass *, Victim, ESI);
	TechnoTypeClass * pTypeVictim = Victim->GetTechnoType();
	//TechnoTypeExt::ExtData * pTypeVictimExt = TechnoTypeExt::ExtMap.Find(pTypeVictim);
	TechnoExt::ExtData *pVictimExt =TechnoExt::ExtMap.Find(Victim);
	HouseClass *pOwnerVictim = Victim->Owner;

	//Get actual damage
	GET(int*, pDamage, EBX);

	//Get object that damages
	GET_STACK(TechnoClass*, Attacker, 0xD4);

	//Explosion crates? SWs?
	if (Attacker) {

		TechnoTypeClass * pTypeAttacker = Attacker->GetTechnoType();
		//TechnoTypeExt::ExtData * pTypeAttackerExt = TechnoTypeExt::ExtMap.Find(pTypeAttacker);
		//Debug::Log("[Pillage] D");
		TechnoExt::ExtData *pAttackerExt =TechnoExt::ExtMap.Find(Attacker);

		if (pOwnerVictim->IsAlliedWith(Attacker->Owner)) {
			if(BuildingClass * pBld = specific_cast<BuildingClass*>(Attacker)) {
				//Armory - Hospital - I guess they deliver damage as well
				if(pBld->Type->Armory || pBld->Type->Hospital){
					return 0;
				}
			}
		}

		auto AttackerModifier = pAttackerExt->Get_Bounty_Modifier();
		auto AttackerFriendlyMod = pAttackerExt->Get_Bounty_FriendlyModifier();
		auto AttackerPillager = pAttackerExt->Get_Bounty_Pillager();
		auto AttackerMessage = pAttackerExt->Get_Bounty_Message();
		auto AttackerFriendlyMsg = pAttackerExt->Get_Bounty_FriendlyMessage();

		auto VictimPillageMult = pVictimExt->Get_Bounty_PillageMultiplier();

		if (AttackerModifier == 0 && AttackerFriendlyMod == 0) {
			return 0; //No bounty activated
		} else {
			if (VictimPillageMult) {
				if (AttackerPillager) {
					double FloatPillage = (*pDamage*1.0) / pTypeVictim->Strength * pTypeVictim->GetCost() * VictimPillageMult;

					//Alliance check
					if(!pOwnerVictim->IsAlliedWith(Attacker->Owner)) {
						auto PillageMoney = int(FloatPillage * AttackerModifier);

						if (PillageMoney != 0) {
							Attacker->Owner->GiveMoney(PillageMoney);
							Debug::Log("[Pillage] %ls's %s delivered %d Damage to an enemy %s, gained %d as Pillage.", Attacker->Owner->UIName, pTypeAttacker->ID, *pDamage, pTypeVictim->ID, PillageMoney);
							//xp = xp + PillageMoney * Attacker->Bounty_Modifier;

							if (AttackerMessage) { //output message only if we want to - Graion
								if (pTypeAttacker->MissileSpawn && Attacker->SpawnOwner) { //V3, Dred
									BountyMessageOutput (Attacker->SpawnOwner, PillageMoney);
								} else {
									BountyMessageOutput (Attacker, PillageMoney);
								}
							}
						}
					} else { //Whoops, allied!
						if (pTypeAttacker->Enslaves) {
							//Slave Miners, I hate ya.
							if (pTypeVictim->Slaved && pTypeVictim == pTypeAttacker->Enslaves) {
								Debug::Log("[Pillage] SLAVES!");
								return 0;
							}
						}

						auto PillageMoney = int(FloatPillage * AttackerFriendlyMod);

						if (PillageMoney != 0){
							Attacker->Owner->TakeMoney(PillageMoney);
							Debug::Log("[Pillage] %ls's %s delivered %d Damage to an ally %s, lost %d as Pillage.", Attacker->Owner->UIName, pTypeAttacker->ID, *pDamage, pTypeVictim->ID, PillageMoney);

							if (AttackerFriendlyMsg) { //output message only if we want to - Graion
								if (pTypeAttacker->MissileSpawn && Attacker->SpawnOwner) { //V3, Dred
									BountyMessageOutput (Attacker->SpawnOwner, -PillageMoney);
								} else {
									BountyMessageOutput (Attacker, -PillageMoney);
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

//Since Bounty works with Temporal, only Pillager fails because of my check, Pillager needed a separate hook
//Moved out from Bugfixes.cpp
DEFINE_HOOK(71A922, Temporal_Kill_Pillage, 5)
{
	GET(TemporalClass *, Temp, ESI);


	// Get Target
	TechnoClass* Victim = Temp->Target;
	TechnoTypeClass * pTypeVictim = Victim->GetTechnoType();
	//TechnoTypeExt::ExtData * pTypeVictimExt = TechnoTypeExt::ExtMap.Find(pTypeVictim);
	TechnoExt::ExtData *pVictimExt =TechnoExt::ExtMap.Find(Victim);
	HouseClass *pOwnerVictim = Victim->Owner;

	// Get Attacker
	TechnoClass* Attacker = Temp->Owner;
	if (Attacker) {
		TechnoTypeClass * pTypeAttacker = Attacker->GetTechnoType();
		//TechnoTypeExt::ExtData * pTypeAttackerExt = TechnoTypeExt::ExtMap.Find(pTypeAttacker);
		TechnoExt::ExtData *pAttackerExt =TechnoExt::ExtMap.Find(Attacker);

		auto AttackerModifier = pAttackerExt->Get_Bounty_Modifier();
		auto AttackerFriendlyMod = pAttackerExt->Get_Bounty_FriendlyModifier();
		auto AttackerPillager = pAttackerExt->Get_Bounty_Pillager();
		auto AttackerMessage = pAttackerExt->Get_Bounty_Message();
		auto AttackerFriendlyMsg = pAttackerExt->Get_Bounty_FriendlyMessage();

		auto VictimPillageMult = pVictimExt->Get_Bounty_PillageMultiplier();

		if (AttackerModifier == 0 && AttackerFriendlyMod == 0) {
			return 0; //No bounty activated
		} else {
			if (VictimPillageMult) {
				if (AttackerPillager) {
					double FloatPillage =  (Victim->Health*1.0 / pTypeVictim->Strength) * pTypeVictim->GetCost() * VictimPillageMult;
					//Alliance check
					if(!pOwnerVictim->IsAlliedWith(Attacker->Owner)) {
						auto PillageMoney = int(FloatPillage * AttackerModifier);
						if (PillageMoney != 0) {
							Attacker->Owner->GiveMoney(PillageMoney);
							Debug::Log("[Pillage] %ls's %s erased an enemy %s, gained %d as Pillage.", Attacker->Owner->UIName, pTypeAttacker->ID, pTypeVictim->ID, PillageMoney);
							//xp = xp + PillageMoney * Attacker->Bounty_Modifier;

							if (AttackerMessage) { //output message only if we want to - Graion
								if (pTypeAttacker->MissileSpawn && Attacker->SpawnOwner) { //V3, Dred
									BountyMessageOutput (Attacker->SpawnOwner, PillageMoney);
								} else {
									BountyMessageOutput (Attacker, PillageMoney);
								}
							}
						}
					} else { //Whoops, allied!

						auto PillageMoney = int(FloatPillage * AttackerFriendlyMod);

						if (PillageMoney != 0) {
							Attacker->Owner->TakeMoney(PillageMoney);
							Debug::Log("[Pillage] %ls's %s erased an ally %s, lost %d as Pillage.", Attacker->Owner->UIName, pTypeAttacker->ID, pTypeVictim->ID, PillageMoney);

							if (AttackerFriendlyMsg) { //output message only if we want to - Graion
								if (pTypeAttacker->MissileSpawn && Attacker->SpawnOwner) { //V3, Dred
									BountyMessageOutput (Attacker->SpawnOwner, -PillageMoney);
								} else {
									BountyMessageOutput (Attacker, -PillageMoney);
								}
							}
						}
					}
				}
			}
		}
	}
	return 0x71A97D; //keeping the fix of 379 and 1266
}

//Time to crush things
DEFINE_HOOK(7418D4, UnitClass_TryCrush, 6) {

	//Get Crusher
    GET(UnitClass *, Crusher, EDI);
	UnitTypeClass * pTypeCrusher = Crusher->Type;
	//TechnoTypeExt::ExtData * pTypeCrusherExt = TechnoTypeExt::ExtMap.Find(pTypeCrusher);
	TechnoExt::ExtData *pCrusherExt =TechnoExt::ExtMap.Find(Crusher);

	//Get Crushed
    GET(ObjectClass *, Victim, ESI);
	TechnoTypeClass * pTypeVictim = Victim->GetTechnoType();
	//TechnoTypeExt::ExtData * pTypeVictimExt = TechnoTypeExt::ExtMap.Find(pTypeVictim);
	TechnoClass * pTechnoVictim = generic_cast<TechnoClass *>(Victim);
	if (pTechnoVictim){
		HouseClass *pOwnerVictim = pTechnoVictim->Owner;
		TechnoExt::ExtData *pVictimExt =TechnoExt::ExtMap.Find(pTechnoVictim);

		auto CrusherModifier = pCrusherExt->Get_Bounty_Modifier();
		auto CrusherFriendlyMod = pCrusherExt->Get_Bounty_FriendlyModifier();
		auto CrusherPillager = pCrusherExt->Get_Bounty_Pillager();
		auto CrusherMessage = pCrusherExt->Get_Bounty_Message();
		auto CrusherFriendlyMsg = pCrusherExt->Get_Bounty_FriendlyMessage();

		auto VictimCostMult = pVictimExt->Get_Bounty_CostMultiplier();
		auto VictimPillageMult = pVictimExt->Get_Bounty_PillageMultiplier();

		if (CrusherModifier == 0 && CrusherFriendlyMod == 0) {
				return 0; //No bounty activated

			} else {

				if (CrusherPillager) {
				//Pillager logic
					double FloatPillage =  (Victim->Health*1.0 / pTypeVictim->Strength) * pTypeVictim->GetCost() * VictimPillageMult;
					if(!pOwnerVictim->IsAlliedWith(Crusher->Owner)) {
						auto PillageMoney = int(FloatPillage * CrusherModifier);

						if (PillageMoney != 0) {
							Crusher->Owner->GiveMoney(PillageMoney);
							Debug::Log("[Pillage] %ls's %s crushed an enemy %s, gained %d as Pillage.", Crusher->Owner->UIName, pTypeCrusher->ID, pTypeVictim->ID, PillageMoney);
							//xp = xp + PillageMoney * Attacker->Bounty_Modifier;

							if (CrusherMessage) { //output message only if we want to - Graion
								//if (pTypeCrusher->MissileSpawn && Crusher->SpawnOwner) { //Small preparation, if SpawnManger is extended to eat TechnoClasses instead of Aircrafts
								//	BountyMessageOutput (Crusher->SpawnOwner, PillageMoney);
								//} else {
									BountyMessageOutput (Crusher, PillageMoney);
								//}
							}
						}

					} else { //Whoops, allied!

						auto PillageMoney = int(FloatPillage * CrusherFriendlyMod);

						if (PillageMoney != 0){
							Crusher->Owner->TakeMoney(PillageMoney);
							Debug::Log("[Pillage] %ls's %s crushed an ally %s, lost %d as Pillage.", Crusher->Owner->UIName, pTypeCrusher->ID, pTypeVictim->ID, PillageMoney);

							if (CrusherFriendlyMsg){ //output message only if we want to - Graion

								//if (pTypeCrusher->MissileSpawn && Crusher->SpawnOwner) { //same
								//	BountyMessageOutput (Crusher->SpawnOwner, -PillageMoney);
								//} else {
									BountyMessageOutput (Crusher, -PillageMoney);
								//}

							}
						}
					}
				} else {
					//Original Bounty
					auto PillageMoney = int(pTypeVictim->GetActualCost(pOwnerVictim) * VictimCostMult);
					if(!pOwnerVictim->IsAlliedWith(Crusher->Owner)) {

						PillageMoney *= CrusherModifier;

						if (PillageMoney != 0) {

							Crusher->Owner->GiveMoney(PillageMoney);
							Debug::Log("[Bounty] %ls's %s crushed an enemy %s, gained %d as Bounty.", Crusher->Owner->UIName, pTypeCrusher->ID, pTypeVictim->ID, PillageMoney);
							//xp = xp + PillageMoney * Attacker->Bounty_Modifier;

							if (CrusherMessage) { //output message only if we want to - Graion

								//if (pTypeCrusher->MissileSpawn && Crusher->SpawnOwner) {
								//	BountyMessageOutput (Crusher->SpawnOwner, PillageMoney);
								//} else {
									BountyMessageOutput (Crusher, PillageMoney);
								//}
							}
						}

					} else { //Whoops, allied!

						PillageMoney *= CrusherFriendlyMod;

						if (PillageMoney != 0){
							Crusher->Owner->TakeMoney(PillageMoney);
							Debug::Log("[Bounty] %ls's %s crushed an ally %s, lost %d as Bounty.", Crusher->Owner->UIName, pTypeCrusher->ID, pTypeVictim->ID, PillageMoney);

							if (CrusherFriendlyMsg){ //output message only if we want to - Graion

							//if (pTypeCrusher->MissileSpawn && Crusher->SpawnOwner) {
							//	BountyMessageOutput (Crusher->SpawnOwner, -PillageMoney);
							//} else {
								BountyMessageOutput (Crusher, -PillageMoney);
							//}

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
