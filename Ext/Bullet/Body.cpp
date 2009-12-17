#include "Body.h"

//! Does the entire PassThrough logic, checks & damage
/*!
	This function determines whether the projectile should
	pass through the outter shell of a building and damage
	the occupants instead.

	If so, it proceeds to damage the occupants, and then
	returns true. If the projectile did not pass through,
	and the building has to be damaged according to
	normal rules. the function returns false.

	\return true if the bullet passed through and the damage case was handled, otherwise false.

	\author Renegade
	\date 02.12.09+

*/
virtual bool BulletExt::ExtData::DamageOccupants() {
	BulletClass* TheBullet = this->AttachedToObject;
	BuildingClass* Building = NULL;

	if(TheBullet->Target->WhatAmI() == abs_Building) {
		BuildingClass* Building = reinterpret_cast<BuildingClass *> (TheBullet->Target);
	} else {
		return false; // target is not a building, so nothing to do
	}

	if(Building) { // if that pointer is null, something went wrong
		BulletTypeExt::ExtData* TheBulletTypeExt = BulletTypeExt::ExtMap.Find(TheBullet->Type);
		BuildingTypeExt::ExtData* BuildingAresData = BuildingTypeExt::ExtMap.Find(Building->Type);

		if(Building->Occupants.Count && BuildingAresData->UCPassThrough) { // only work when UCPassThrough is set, as per community vote in thread #1392
			if(!TheBulletTypeExt->SubjectToTrenches || ((ScenarioClass::Instance->Randomizer.RandomRanged(0, 99) / 100) < BuildingAresData->UCPassThrough)) { // test for negative b/c being SubjectToTrenches means "we're getting stopped by trenches".
				int poorBastard = ScenarioClass::Instance->Randomizer.RandomRanged(0, Building->Occupants.Count - 1); // which Occupant is getting it?
				if(BuildingAresData->UCFatalRate && ((ScenarioClass::Instance->Randomizer.RandomRanged(0, 99) / 100) < BuildingAresData->UCFatalRate)) {
					// fatal hit
					Building->Occupants[poorBastard]->Destroyed(TheBullet->Owner);
				} else {
					/* ReceiveDamage args:
					virtual eDamageState ReceiveDamage(int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH,
						ObjectClass* Attacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse) R0;
						where
						DistanceFromEpicenter -> used for CellSpread/PercentAtMax
						IgnoreDefenses -> ignore Immune=yes
					*/

					// just a flesh wound
					int adjustedDamage = static_cast<int> (ceil(TheBullet->Health * UCDamageMultiplier)); // Bullet->Health is the damage it delivers (go Westwood)
					Building->Occupants[poorBastard]->ReceiveDamage(&adjustedDamage, 0, TheBullet->WH, TheBullet->Owner, false, true, TheBullet->GetOwningHouse()) R0;
				}
				return true;
			} else {
				return false; // no damage dealt b/c bullet was SubjectToTrenches=yes and UC.PassThrough did not apply
			}
		} else {
			return false; // no damage dealt b/c either there were no occupants, or UC.PassThrough is set to 0
		}
	} else {
		return false; // no damage dealt b/c the building-pointer was NULL (which is concerning)
	}
}
