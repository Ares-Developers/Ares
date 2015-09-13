#include <InfantryClass.h>
#include <ScenarioClass.h>

#include "Body.h"
#include "../BulletType/Body.h"
#include "../Building/Body.h"
#include "../BuildingType/Body.h"

#include "../../Misc/SavegameDef.h"

template<> const DWORD Extension<BulletClass>::Canary = 0x2A2A2A2A;
BulletExt::ExtContainer BulletExt::ExtMap;

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
	auto const pThis = this->OwnerObject();

	auto const pBuilding = abstract_cast<BuildingClass*>(pThis->Target);

	auto const Logging = false;

	// if that pointer is null, something went wrong
	if(!pBuilding) {
		return false;
	}

	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);
	auto const pBldTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

	auto const occupants = pBuilding->Occupants.Count;
	auto const passThrough = pBldTypeExt->UCPassThrough.Get();

/*	Debug::Log(
		Logging, "Bullet %s is about to damage occupants of %s: occupants "
		"#%d, UC.PT = %lf\n", pThis->Type->ID, pBuilding->Type->ID,
		occupants, passThrough); */

	// only work when UCPassThrough is set, 
	// as per community vote in thread #1392
	if(!occupants || !passThrough) {
		return false;
	}

	auto const subjectToTrenches = pTypeExt->SubjectToTrenches.Get();
	Debug::Log(Logging, "SubjToTrenches = %d\n", subjectToTrenches);

	// being SubjectToTrenches means "we're getting stopped by trenches".
	auto& Random = ScenarioClass::Instance->Random;
	if(subjectToTrenches && Random.RandomDouble() >= passThrough) {
		return false;
	}

	// which Occupant is getting it?
	auto const idxPoorBastard = Random.RandomRanged(0, occupants - 1);
	auto const pPoorBastard = pBuilding->Occupants[idxPoorBastard];

	Debug::Log(Logging, "Poor Bastard #%d\n", idxPoorBastard);

	auto const fatalRate = pBldTypeExt->UCFatalRate.Get();
	if(fatalRate > 0.0 && Random.RandomDouble() < fatalRate) {
		Debug::Log(Logging, "Fatal hit!\n");
		// fatal hit
		pPoorBastard->Destroyed(pThis->Owner);
		pPoorBastard->UnInit();
		// don't separate these two lines - poor guy's already ~dtor'd,
		// but his pointer is still dangling from the vector
		pBuilding->Occupants.RemoveItem(idxPoorBastard);
		pBuilding->UpdateThreatInCell(pBuilding->GetCell());
	} else {
		// just a flesh wound
		auto const multiplier = pBldTypeExt->UCDamageMultiplier.Get();
		Debug::Log(Logging, "Flesh wound - health(%d) * UCDmgMult(%lf)\n",
			pThis->Health, multiplier);
		// Bullet->Health is the damage it delivers (go Westwood)
		auto adjustedDamage = static_cast<int>(
			std::ceil(pThis->Health * multiplier));
		Debug::Log(Logging, "Adjusted damage = %d\n", adjustedDamage);
		auto const result = pPoorBastard->ReceiveDamage(&adjustedDamage, 0,
			pThis->WH, pThis->Owner, false, true, pThis->GetOwningHouse());
		Debug::Log(Logging, "Received damage, %d\n", result);
	}

	// fix up the firing index, otherwise building stops to fire
	if(pBuilding->FiringOccupantIndex >= pBuilding->GetOccupantCount()) {
		pBuilding->FiringOccupantIndex = 0;
	}

	// if the last occupant was killed and this building was raided,
	// it needs to be returned to its owner. (Bug #700)
	auto const pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);
	pBuildingExt->evalRaidStatus();

	return true;
}

// =============================
// load / save

template <typename T>
void BulletExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->NukeSW);
}

void BulletExt::ExtData::LoadFromStream(AresStreamReader &Stm) {
	Extension<BulletClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BulletExt::ExtData::SaveToStream(AresStreamWriter &Stm) {
	Extension<BulletClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

BulletExt::ExtContainer::ExtContainer() : Container("BulletClass") {
}

BulletExt::ExtContainer::~ExtContainer() = default;

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
