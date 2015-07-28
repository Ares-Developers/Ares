//#include "Body.h"

#include "../Rules/Body.h"

#include <AircraftClass.h>
//#include <Helpers/Enumerators.h>

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
		// disallow aircraft, or units considered as aircraft, or stuff not on map like parasites
		if(pAttacker->WhatAmI() != AircraftClass::AbsID) {
			if(auto pAttackerFoot = abstract_cast<FootClass*>(pAttacker)) {
				if(pAttackerFoot->InLimbo || pAttackerFoot->GetTechnoType()->ConsideredAircraft) {
					return 0x6EB47A;
				}
			}

			pThis->Focus = pAttacker;

			// this is the original code, but commented out because it's responsible for switching
			// targets when the team is attacked by two or more opponents. Now, the team should pick
			// the first target, and keep it. -AlexB
			//for(NextTeamMember i(pThis->FirstUnit); i; ++i) {
			//	if(i->IsAlive && i->Health && (Unsorted::IKnowWhatImDoing || !i->InLimbo)) {
			//		if(i->IsTeamLeader || i->WhatAmI() == AircraftClass::AbsID) {
			//			i->SetTarget(nullptr);
			//			i->SetDestination(nullptr, true);
			//		}
			//	}
			//}
		}
	}

	return 0x6EB47A;
}
