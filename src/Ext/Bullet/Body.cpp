#include <InfantryClass.h>
#include <ScenarioClass.h>

#include "Body.h"
#include "../BulletType/Body.h"
#include "../Building/Body.h"
#include "../BuildingType/Body.h"

template<> const DWORD Extension<BulletClass>::Canary = 0x87654321;
Container<BulletExt> BulletExt::ExtMap;

template<> BulletClass *Container<BulletExt>::SavingObject = nullptr;
template<> IStream *Container<BulletExt>::SavingStream = nullptr;

// #663: PassThrough; #667: SubjectToTrenches
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
bool BulletExt::ExtData::DamageOccupants() {
	BulletClass* TheBullet = this->AttachedToObject;

	if(BuildingClass* Building = specific_cast<BuildingClass *> (TheBullet->Target)) { // if that pointer is null, something went wrong
		BulletTypeExt::ExtData* TheBulletTypeExt = BulletTypeExt::ExtMap.Find(TheBullet->Type);
		BuildingTypeExt::ExtData* BuildingAresData = BuildingTypeExt::ExtMap.Find(Building->Type);

/*
		Debug::Log("Bullet %s is about to damage occupants of %s: occupants #%d, UC.PT = %lf\n",
			TheBullet->Type->ID, Building->Type->ID, Building->Occupants.Count, BuildingAresData->UCPassThrough);
*/
		// only work when UCPassThrough is set, as per community vote in thread #1392
		if(Building->Occupants.Count && BuildingAresData->UCPassThrough) {
			Debug::Log("SubjToTrenches = %d\n", TheBulletTypeExt->SubjectToTrenches);
			// test for !SubjectToTrenches because being SubjectToTrenches means "we're getting stopped by trenches".
			if(!TheBulletTypeExt->SubjectToTrenches
				|| ((ScenarioClass::Instance->Random.RandomRanged(0, 99) / 100.0) < BuildingAresData->UCPassThrough)) {
				int poorBastard = ScenarioClass::Instance->Random.RandomRanged(0, Building->Occupants.Count - 1); // which Occupant is getting it?
				Debug::Log("Poor Bastard #%d\n", poorBastard);
				if(BuildingAresData->UCFatalRate
					&& ((ScenarioClass::Instance->Random.RandomRanged(0, 99) / 100.0) < BuildingAresData->UCFatalRate)) {
					Debug::Log("Fatal hit!\n");
					// fatal hit
					Building->Occupants[poorBastard]->Destroyed(TheBullet->Owner);
					Building->Occupants[poorBastard]->UnInit();
					// don't separate these two lines - poor guy's already ~dtor'd, but his pointer is still dangling from the vector
					Building->Occupants.RemoveItem(poorBastard);
					Building->UpdateThreatInCell(Building->GetCell());
				} else {
					/* ReceiveDamage args:
					virtual eDamageState ReceiveDamage(int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH,
						ObjectClass* Attacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse) R0;
						where
						DistanceFromEpicenter -> used for CellSpread/PercentAtMax
						IgnoreDefenses -> ignore Immune=yes
					*/

					// just a flesh wound
					Debug::Log("Flesh wound - health(%d) * UCDmgMult(%lf)\n", TheBullet->Health, BuildingAresData->UCDamageMultiplier);
					// Bullet->Health is the damage it delivers (go Westwood)
					int adjustedDamage = static_cast<int> (ceil(TheBullet->Health * BuildingAresData->UCDamageMultiplier));
					Debug::Log("Adjusted damage = %d\n", adjustedDamage);
					int result = Building->Occupants[poorBastard]->ReceiveDamage(&adjustedDamage, 0, TheBullet->WH,
								TheBullet->Owner, false, true, TheBullet->GetOwningHouse());
					Debug::Log("Received damage, %d\n", result);
				}

				// BuildingAresData is for the BuildingType for some reason, so we need a new Ext var
				BuildingExt::ExtData* SpecificBuildingExt = BuildingExt::ExtMap.Find(Building);
				// if the last occupant was killed and this building was raided, it needs to be returned to its owner. (Bug #700)
				SpecificBuildingExt->evalRaidStatus();
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

// =============================
// container hooks

DEFINE_HOOK(4664BA, BulletClass_CTOR, 5)
{
	GET(BulletClass*, pItem, ESI);

	BulletExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(466560, BulletClass_DTOR, 6)
{
	GET(BulletClass*, pItem, ECX);

	BulletExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(46AFB0, BulletClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(46AE70, BulletClass_SaveLoad_Prefix, 5)
{
	GET_STACK(BulletExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<BulletExt>::SavingObject = pItem;
	Container<BulletExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(46AF9E, BulletClass_Load_Suffix, 7)
{
	BulletExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(46AFC4, BulletClass_Save_Suffix, 3)
{
	BulletExt::ExtMap.SaveStatic();
	return 0;
}
