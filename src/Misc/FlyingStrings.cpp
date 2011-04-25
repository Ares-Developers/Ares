/* FlyingStrings.cpp - controls floating messages, used by Bounty
Logic by Joshy
INI controls are by Graion Dilach
Beware! This file has an unfinished experience per player counting as well.
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

DynamicVectorClass<FlyingStrings::Item> FlyingStrings::data;

//int xp;

DEFINE_HOOK(702E64,RecordTheKill_CalculateBountyXP,6){

	//Get object being killed
	GET(TechnoClass *, Victim, ESI);
	TechnoTypeClass * pTypeVictim = Victim->GetTechnoType();
	TechnoTypeExt::ExtData * pTypeVictimExt = TechnoTypeExt::ExtMap.Find(pTypeVictim);
	HouseClass *pOwnerVictim = Victim->Owner;

	//Get object that kills
	GET(TechnoClass *, Killer, EDI);
	TechnoTypeClass * pTypeKiller = Killer->GetTechnoType();
	TechnoTypeExt::ExtData * pTypeKillerExt = TechnoTypeExt::ExtMap.Find(pTypeKiller);
	if (!pTypeVictimExt->ImmuneToBounty){
		if (!pTypeKillerExt->Bounty_Pillager){
			float victimActualCost = (float)pTypeVictim->GetActualCost(pOwnerVictim);

			FootClass *F = reinterpret_cast<FootClass *>(Victim);
			CoordStruct coords;
			F->GetCoords(&coords);
			Point2D point;
			TacticalClass::Instance->CoordsToClient(&coords, &point);

			//Killer cannot kill something allied, and it must be human owned
			if(Killer->Owner->UIName != pOwnerVictim->UIName &&  !pOwnerVictim->IsAlliedWith(Killer->Owner)){
				
				victimActualCost *= pTypeKillerExt->Bounty_Modifier;
				int victimActualCostInt = (float)victimActualCost;

					if (victimActualCostInt != 0){
						Killer->Owner->GiveMoney(victimActualCostInt);
						Debug::Log("[Bounty] %s's %s destroyed an enemy %s, gained %d as Bounty.", Killer->Owner, Killer, Victim, victimActualCostInt);
				//xp = xp + victimActualCost * Killer->Bounty_Modifier;
						if (!!pTypeKillerExt->Bounty_Message){ //output message only if we want to - Graion
							wchar_t Message[256];
							_snwprintf(Message, 256, L"+$%d", victimActualCostInt);

							FlyingStrings::Add(Message, point, COLOR_GREEN);
						}
					}
				}
	//Bady!
			if(Killer->Owner->UIName == pOwnerVictim->UIName || pOwnerVictim->IsAlliedWith(Killer->Owner)){
				
				victimActualCost*=pTypeKillerExt->Bounty_FriendlyModifier;
				int victimActualCostInt = (float)victimActualCost;

				if (victimActualCostInt != 0){
						Killer->Owner->TakeMoney(victimActualCostInt);
						Debug::Log("[Bounty] %s's %s destroyed an ally %s, lost %d as Bounty.", Killer->Owner, Killer, Victim, victimActualCostInt);
						if (!!pTypeKillerExt->Bounty_FriendlyMessage){ //output message only if we want to - Graion
							wchar_t Message2[256];
							_snwprintf(Message2, 256, L"-$%d", victimActualCostInt);

							FlyingStrings::Add(Message2, point, COLOR_RED);
							}
						}
				}

			}
		}
	return 0;
}


// 1523, Get money per Damage!
DEFINE_HOOK(701DCC,BountyPillage,7) //Thank Joshy for the hook
	{
	//Get object being damaged
	GET(TechnoClass *, Victim, ESI);
	TechnoTypeClass * pTypeVictim = Victim->GetTechnoType();
	TechnoTypeExt::ExtData * pTypeVictimExt = TechnoTypeExt::ExtMap.Find(pTypeVictim);
	HouseClass *pOwnerVictim = Victim->Owner;

	//Get actual damage
	GET(int, Damage, EBX);
		
	//Get object that damages
	GET_STACK(TechnoClass*, Attacker, 0xC8);
	TechnoTypeClass * pTypeAttacker = Attacker->GetTechnoType();
	TechnoTypeExt::ExtData * pTypeAttackerExt = TechnoTypeExt::ExtMap.Find(pTypeAttacker);
	if(!!pTypeVictimExt->ImmuneToBounty){
		if(!!pTypeAttackerExt->Bounty_Pillager){
		float PillageMoney = (float)Damage;

		FootClass *F = reinterpret_cast<FootClass *>(Victim);
		CoordStruct coords;
		F->GetCoords(&coords);
		Point2D point;
		TacticalClass::Instance->CoordsToClient(&coords, &point);


			if(Attacker->Owner->UIName != pOwnerVictim->UIName && !pOwnerVictim->IsAlliedWith(Attacker->Owner)){
				
				PillageMoney *= pTypeAttackerExt->Bounty_Modifier;
				int PillageMoneyInt = PillageMoney;
				
				if (PillageMoneyInt != 0){
					Attacker->Owner->GiveMoney(PillageMoneyInt);
						Debug::Log("[Bounty] %s is a Pillager, gained %d as Bounty through damage.", Attacker, Victim, PillageMoneyInt);
				//xp = xp + PillageMoney * Attacker->Bounty_Modifier;
				if (!!pTypeAttackerExt->Bounty_Message){ //output message only if we want to - Graion
						wchar_t Message[256];
						_snwprintf(Message, 256, L"+$%d", PillageMoney);

						FlyingStrings::Add(Message, point, COLOR_GREEN);
						}
				}

			}
	//Bady!
			if(Attacker->Owner->UIName == pOwnerVictim->UIName || pOwnerVictim->IsAlliedWith(Attacker->Owner)){
				
				PillageMoney *= pTypeAttackerExt->Bounty_FriendlyModifier;
				int PillageMoneyInt=PillageMoney;
				
				
				if (PillageMoneyInt != 0){
						Attacker->Owner->TakeMoney(PillageMoneyInt);
						Debug::Log("[Bounty] %s is a Pillager, lost %d as Bounty through damage.", Attacker, Victim, PillageMoneyInt);
						if (!!pTypeAttackerExt->Bounty_FriendlyMessage){ //output message only if we want to - Graion
							wchar_t Message2[256];
							_snwprintf(Message2, 256, L"-$%d", PillageMoneyInt);

							FlyingStrings::Add(Message2, point, COLOR_RED);
							}
				}
		
			}
		}
	}

	return 0;
}


DEFINE_HOOK(4F4558,loc_4F4480,5){
	FlyingStrings::UpdateAll();
	return 0;
}

/*
DEFINE_HOOK(5C9A44,loc_568A0A,6){
	CCFileClass file("RA2MD.INI");
	CCINIClass pINI;
	pINI.ReadCCFile(&file);

	pINI.WriteInteger("Network", "Xp.Total", 5608945, true);
	return 0;
}
*/