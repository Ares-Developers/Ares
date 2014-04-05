//#include "Body.h"

#include "../Rules/Body.h"

#include <AircraftClass.h>

// #895225: make the AI smarter. this code was missing from YR.
// it clears the targets and assigns the attacker the team's current focus.
DEFINE_HOOK(6EB432, TeamClass_AttackedBy_Retaliate, 9)
{
	GET(TeamClass*, pThis, ESI);
	GET(AbstractClass*, pAttacker, EBP);

	// get ot if global option is off
	if(!RulesExt::Global()->TeamRetaliate) {
		return 0x6EB47A;
	}

	auto pFocus = abstract_cast<TechnoClass*>(pThis->Focus);
	auto pSpawn = pThis->SpawnCell;

	if(!pFocus || !pFocus->GetWeapon(0)->WeaponType || !pSpawn || pFocus->IsCloseEnoughToAttackCoords(pSpawn->GetCoords())) {
		if(pAttacker->WhatAmI() != AircraftClass::AbsID) {
			pThis->Focus = pAttacker;
			for(auto i = pThis->FirstUnit; i; i = i->NextTeamMember) {
				if(i->IsAlive && i->Health && (Unsorted::IKnowWhatImDoing || !i->InLimbo)) {
					if(i->IsTeamLeader || i->WhatAmI() == AircraftClass::AbsID) {
						i->SetTarget(nullptr);
						i->SetDestination(nullptr, true);
					}
				}
			}
		}
	}

	return 0x6EB47A;
}
