/*
Bounty logic by Joshy, Graion Dilach
*/


#include "Debug.h"
#include "FlyingStrings.h"
#include "../Ext/House/Body.h"
#include "../Ext/Techno/Body.h"
#include <SlaveManagerClass.h>

//=========================actual logic

void BountyClass::CalculateBounty(TechnoClass* Attacker, TechnoClass* Victim, int PillageDamage) {
	TechnoTypeClass * pTypeAttacker = Attacker->GetTechnoType();
	TechnoExt::ExtData * pAttackerExt = TechnoExt::ExtMap.Find(Attacker);

	TechnoTypeClass * pTypeVictim = Victim->GetTechnoType();
	TechnoExt::ExtData *pVictimExt = TechnoExt::ExtMap.Find(Victim);

	double VictimCostMult;

	if(PillageDamage) {
		VictimCostMult = (PillageDamage*1.0) / pTypeVictim->Strength * pVictimExt->Get_Bounty_PillageMultiplier();
	} else {
		VictimCostMult = pVictimExt->Get_Bounty_CostMultiplier();
	}

	auto VictimActualCost = int(pTypeVictim->GetCost() * VictimCostMult);

	//Alliance check
	if (!Victim->Owner->IsAlliedWith(Attacker->Owner)) {
		VictimActualCost *= pAttackerExt->Get_Bounty_Modifier();
	} else {
		VictimActualCost *= pAttackerExt->Get_Bounty_FriendlyModifier() * -1;
	}

	if (VictimActualCost != 0){
		Attacker->Owner->GiveMoney(VictimActualCost);
		Debug::Log("[Bounty] %ls's %s attacked %s, gained %d as Bounty.", Attacker->Owner->UIName, pTypeAttacker->ID, pTypeVictim->ID, VictimActualCost);
						
		if (pAttackerExt->Get_Bounty_Message()) { //output message only if we want to - Graion
			if (pTypeAttacker->MissileSpawn && Attacker->SpawnOwner) { //V3, Dred
				TechnoExt::ExtData * pSpawnerExt = TechnoExt::ExtMap.Find(Attacker->SpawnOwner);
				pSpawnerExt->Bounty_Amount += VictimActualCost;
			} else {
				pAttackerExt->Bounty_Amount += VictimActualCost;
			}
		}
	}
}

void BountyClass::Read(INI_EX *exINI, const char * section) {
		this->Message.Read(exINI, section, "Bounty.Message");
		this->Modifier.Read(exINI, section, "Bounty.Modifier");
		this->FriendlyModifier.Read(exINI, section, "Bounty.FriendlyModifier");
		this->CostMultiplier.Read(exINI, section, "Bounty.CostMultiplier");
		// #1523 Money-Conversion per applied Damage... tag will be Bounty.Pillager
		this->Pillager.Read(exINI, section, "Bounty.Pillager");
		this->PillageMultiplier.Read(exINI, section, "Bounty.PillageMultiplier");
	}

void BountyClass::BountyMessageOutput(TechnoClass* Messager) {

	TechnoExt::ExtData *pMessagerExt =TechnoExt::ExtMap.Find(Messager);
	int Amount = pMessagerExt->Bounty_Amount;
	pMessagerExt->Bounty_Amount = 0;
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

//===================== container hooks

DEFINE_HOOK(702E64, TechnoClass_CalculateBountyAtKill, 6) {

	//Get object being killed
	GET(TechnoClass *, Victim, ESI);

	//Get object that kills
	GET(TechnoClass *, Killer, EDI);

	if(Killer) {
		TechnoExt::ExtData * pKillerExt = TechnoExt::ExtMap.Find(Killer);

		if (!(pKillerExt->Get_Bounty_Pillager()) &&
			pKillerExt->Get_Bounty_Modifier() != 0 ||
			pKillerExt->Get_Bounty_FriendlyModifier() != 0
			) {
			BountyClass::CalculateBounty(Killer, Victim, 0);
		}
	}
	return 0;
}
	

DEFINE_HOOK(701DFF, TechnoClass_ReceiveDamage_BountyPillage, 7) {

	//Get object being damaged
	GET(TechnoClass *, Victim, ESI);

	//Get actual damage
	GET(int*, pDamage, EBX);

	//Get object that damages
	GET_STACK(TechnoClass*, Attacker, 0xD4);

	if(Attacker) {
		TechnoExt::ExtData *pAttackerExt =TechnoExt::ExtMap.Find(Attacker);

		if (pAttackerExt->Get_Bounty_Pillager() &&
			pAttackerExt->Get_Bounty_Modifier() != 0 ||
			pAttackerExt->Get_Bounty_FriendlyModifier() != 0
			) {
			BountyClass::CalculateBounty(Attacker, Victim, *pDamage);
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

	// Get Attacker
	TechnoClass* Attacker = Temp->Owner;

	if(Attacker) {
		TechnoExt::ExtData *pAttackerExt =TechnoExt::ExtMap.Find(Attacker);

		if (pAttackerExt->Get_Bounty_Pillager() &&
			pAttackerExt->Get_Bounty_Modifier() != 0 ||
			pAttackerExt->Get_Bounty_FriendlyModifier() != 0
			) {
			BountyClass::CalculateBounty(Attacker, Victim, Victim->Health);
		}
	}
	return 0x71A97D; //keeping the fix of 379 and 1266
}

//Time to crush things
DEFINE_HOOK(7418D4, UnitClass_TryCrush, 6) {

	//Get Crusher
    GET(UnitClass *, Crusher, EDI);

	//Get crushed
    //GET(ObjectClass *, Victim, ESI); //let's see what happens
	GET(TechnoClass *, Victim, ESI);

	if(Crusher) {
		TechnoExt::ExtData *pCrusherExt =TechnoExt::ExtMap.Find(Crusher);

		if (pCrusherExt->Get_Bounty_Modifier() != 0 ||
			pCrusherExt->Get_Bounty_FriendlyModifier() != 0) {
			if (pCrusherExt->Get_Bounty_Pillager()) {
				BountyClass::CalculateBounty(Crusher, Victim, Victim->Health);
			} else {
				BountyClass::CalculateBounty(Crusher, Victim, 0);
			}
		}
	}

	return 0;
}