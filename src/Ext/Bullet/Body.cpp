#include <InfantryClass.h>
#include <ScenarioClass.h>

#include "Body.h"
#include "../BulletType/Body.h"
#include "../Building/Body.h"
#include "../BuildingType/Body.h"

template<> const DWORD Extension<BulletClass>::Canary = 0x2A2A2A2A;
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
	BulletClass* TheBullet = this->OwnerObject();

	auto Building = specific_cast<BuildingClass*>(TheBullet->Target);

	// if that pointer is null, something went wrong
	if(!Building) {
		// no damage dealt b/c the building-pointer was NULL (which is concerning)
		return false;
	}

	auto TheBulletTypeExt = BulletTypeExt::ExtMap.Find(TheBullet->Type);
	auto BuildingAresData = BuildingTypeExt::ExtMap.Find(Building->Type);

/*
	Debug::Log("Bullet %s is about to damage occupants of %s: occupants #%d, UC.PT = %lf\n",
		TheBullet->Type->ID, Building->Type->ID, Building->Occupants.Count, BuildingAresData->UCPassThrough);
*/
	// only work when UCPassThrough is set, as per community vote in thread #1392
	if(!Building->Occupants.Count || !BuildingAresData->UCPassThrough) {
		// no damage dealt b/c either there were no occupants, or UC.PassThrough is set to 0
		return false;
	}

	Debug::Log("SubjToTrenches = %d\n", TheBulletTypeExt->SubjectToTrenches.Get());

	// test for SubjectToTrenches because being SubjectToTrenches means "we're getting stopped by trenches".
	if(TheBulletTypeExt->SubjectToTrenches && ScenarioClass::Instance->Random.RandomDouble() >= BuildingAresData->UCPassThrough) {
		// no damage dealt b/c bullet was SubjectToTrenches=yes and UC.PassThrough did not apply
		return false;
	}

	// which Occupant is getting it?
	int idxPoorBastard = ScenarioClass::Instance->Random.RandomRanged(0, Building->Occupants.Count - 1);
	auto poorBastard = Building->Occupants[idxPoorBastard];

	Debug::Log("Poor Bastard #%d\n", idxPoorBastard);
	if(BuildingAresData->UCFatalRate && ScenarioClass::Instance->Random.RandomDouble() < BuildingAresData->UCFatalRate) {
		Debug::Log("Fatal hit!\n");
		// fatal hit
		poorBastard->Destroyed(TheBullet->Owner);
		poorBastard->UnInit();
		// don't separate these two lines - poor guy's already ~dtor'd, but his pointer is still dangling from the vector
		Building->Occupants.RemoveItem(idxPoorBastard);
		Building->UpdateThreatInCell(Building->GetCell());
	} else {
		// just a flesh wound
		Debug::Log("Flesh wound - health(%d) * UCDmgMult(%lf)\n", TheBullet->Health, BuildingAresData->UCDamageMultiplier.Get());
		// Bullet->Health is the damage it delivers (go Westwood)
		int adjustedDamage = static_cast<int>(std::ceil(TheBullet->Health * BuildingAresData->UCDamageMultiplier));
		Debug::Log("Adjusted damage = %d\n", adjustedDamage);
		auto result = poorBastard->ReceiveDamage(&adjustedDamage, 0, TheBullet->WH,
			TheBullet->Owner, false, true, TheBullet->GetOwningHouse());
		Debug::Log("Received damage, %d\n", result);
	}

	// fix up the firing index, otherwise building stops to fire
	if(Building->FiringOccupantIndex >= Building->GetOccupantCount()) {
		Building->FiringOccupantIndex = 0;
	}

	// if the last occupant was killed and this building was raided, it needs to be returned to its owner. (Bug #700)
	auto SpecificBuildingExt = BuildingExt::ExtMap.Find(Building);
	SpecificBuildingExt->evalRaidStatus();

	return true;
}

// =============================
// container hooks

DEFINE_HOOK(4664BA, BulletClass_CTOR, 5)
{
	GET(BulletClass*, pItem, ESI);

	BulletExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(4665E9, BulletClass_DTOR, A)
{
	GET(BulletClass*, pItem, ESI);

	BulletExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(46AFB0, BulletClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(46AE70, BulletClass_SaveLoad_Prefix, 5)
{
	GET_STACK(BulletClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BulletExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK_AGAIN(46AF97, BulletClass_Load_Suffix, 7)
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
