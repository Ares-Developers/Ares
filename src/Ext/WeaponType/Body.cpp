#include "Body.h"
#include <AbstractClass.h>
#include <BulletClass.h>
#include <FootClass.h>
#include <TechnoTypeClass.h>
#include <LocomotionClass.h>
#include "../WarheadType/Body.h"
#include "../Techno/Body.h"
#include "../TechnoType/Body.h"
#include "../../Utilities/TemplateDef.h"

template<> const DWORD Extension<WeaponTypeClass>::Canary = 0x33333333;
Container<WeaponTypeExt> WeaponTypeExt::ExtMap;

template<> WeaponTypeExt::TT *Container<WeaponTypeExt>::SavingObject = nullptr;
template<> IStream *Container<WeaponTypeExt>::SavingStream = nullptr;

const ColorStruct WeaponTypeExt::ExtData::DefaultWaveColor = ColorStruct(255, 255, 255); // placeholder
const ColorStruct WeaponTypeExt::ExtData::DefaultWaveColorMagBeam = ColorStruct(0xB0, 0, 0xD0); // rp2 values
const ColorStruct WeaponTypeExt::ExtData::DefaultWaveColorSonic = ColorStruct(0, 0, 0); // 0,0,0 is a magic value for "no custom handling"

hash_bombExt WeaponTypeExt::BombExt;
hash_waveExt WeaponTypeExt::WaveExt;
hash_boltExt WeaponTypeExt::BoltExt;
hash_radsiteExt WeaponTypeExt::RadSiteExt;

void WeaponTypeExt::ExtData::Initialize(WeaponTypeClass *pThis)
{
	this->Wave_Reverse[idxVehicle] = pThis->IsMagBeam;
};

void WeaponTypeExt::ExtData::LoadFromINIFile(WeaponTypeExt::TT *pThis, CCINIClass *pINI)
{
	const char * section = pThis->get_ID();
	if(!pINI->GetSection(section)) {
		return;
	}

	if(pThis->Damage == 0 && this->Weapon_Loaded) {
		// blargh
		// this is the ugly case of a something that apparently isn't loaded from ini yet, wonder why
		this->Weapon_Loaded = 0;
		pThis->LoadFromINI(pINI);
		return;
	}

	ColorStruct tmpColor;

	INI_EX exINI(pINI);

	this->Beam_Duration     = pINI->ReadInteger(section, "Beam.Duration", this->Beam_Duration);
	this->Beam_Amplitude    = pINI->ReadDouble(section, "Beam.Amplitude", this->Beam_Amplitude);
	this->Beam_IsHouseColor = pINI->ReadBool(section, "Beam.IsHouseColor", this->Beam_IsHouseColor);
	this->Beam_Color.Read(exINI, section, "Beam.Color");

	this->Wave_IsLaser      = pINI->ReadBool(section, "Wave.IsLaser", this->Wave_IsLaser);
	this->Wave_IsBigLaser   = pINI->ReadBool(section, "Wave.IsBigLaser", this->Wave_IsBigLaser);
	this->Wave_IsHouseColor = pINI->ReadBool(section, "Wave.IsHouseColor", this->Wave_IsHouseColor);

	if(this->IsWave(pThis) && !this->Wave_IsHouseColor) {
		this->Wave_Color.Read(exINI, section, "Wave.Color");
	}

	this->Wave_Reverse[idxVehicle]   =
		pINI->ReadBool(section, "Wave.ReverseAgainstVehicles", this->Wave_Reverse[idxVehicle]);
	this->Wave_Reverse[idxAircraft]  =
		pINI->ReadBool(section, "Wave.ReverseAgainstAircraft", this->Wave_Reverse[idxAircraft]);
	this->Wave_Reverse[idxBuilding] =
		pINI->ReadBool(section, "Wave.ReverseAgainstBuildings", this->Wave_Reverse[idxBuilding]);
	this->Wave_Reverse[idxInfantry]  =
		pINI->ReadBool(section, "Wave.ReverseAgainstInfantry", this->Wave_Reverse[idxInfantry]);
	this->Wave_Reverse[idxOther]  =
		pINI->ReadBool(section, "Wave.ReverseAgainstOthers", this->Wave_Reverse[idxOther]);

	if(pThis->IsElectricBolt) {
		this->Bolt_Color1.Read(exINI, section, "Bolt.Color1");
		this->Bolt_Color2.Read(exINI, section, "Bolt.Color2");
		this->Bolt_Color3.Read(exINI, section, "Bolt.Color3");
	}

	this->Laser_Thickness.Read(exINI, section, "LaserThickness");

//	pData->Wave_InitialIntensity = pINI->ReadInteger(section, "Wave.InitialIntensity", pData->Wave_InitialIntensity);
//	pData->Wave_IntensityStep    = pINI->ReadInteger(section, "Wave.IntensityStep", pData->Wave_IntensityStep);
//	pData->Wave_FinalIntensity   = pINI->ReadInteger(section, "Wave.FinalIntensity", pData->Wave_FinalIntensity);

	if(!pThis->Warhead) {
		Debug::Log("Weapon %s doesn't have a Warhead yet, what gives?\n", section);
		return;
	}

	if(pThis->Warhead->IvanBomb) {
		this->Ivan_KillsBridges.Read(exINI, section, "IvanBomb.DestroysBridges");
		this->Ivan_Detachable.Read(exINI, section, "IvanBomb.Detachable");

		this->Ivan_Damage.Read(exINI, section, "IvanBomb.Damage");
		this->Ivan_Delay.Read(exINI, section, "IvanBomb.Delay");

		this->Ivan_FlickerRate.Read(exINI, section, "IvanBomb.FlickerRate");

		this->Ivan_TickingSound.Read(exINI, section, "IvanBomb.TickingSound");

		this->Ivan_AttachSound.Read(exINI, section, "IvanBomb.AttachSound");

		this->Ivan_WH.Read(exINI, section, "IvanBomb.Warhead");

		this->Ivan_Image.Read(exINI, section, "IvanBomb.Image");

		this->Ivan_CanDetonateTimeBomb.Read(exINI, section, "IvanBomb.CanDetonateTimeBomb");
		this->Ivan_CanDetonateDeathBomb.Read(exINI, section, "IvanBomb.CanDetonateDeathBomb");
	}
//
/*
	if(pThis->get_RadLevel()) {
//		pData->Beam_Duration     = pINI->ReadInteger(section, "Beam.Duration", pData->Beam_Duration);
		if(pINI->ReadString(section, "Radiation.Type", "", buffer, 256)) {
			RadType * rType = RadType::Find(buffer);
			if(!rType) {
				Debug::Log("Weapon [%s] references undeclared RadiationType '%s'!\n", section, buffer);
			}
			pData->Rad_Type = rType;
		}
	}
*/
	// #680 Chrono Prison
	this->Abductor.Read(exINI, section, "Abductor");
	this->Abductor_AnimType.Read(exINI, section, "Abductor.Anim");
	this->Abductor_ChangeOwner.Read(exINI, section, "Abductor.ChangeOwner");
	this->Abductor_AbductBelowPercent.Read(exINI, section, "Abductor.AbductBelowPercent");

	// brought back from TS
	this->ProjectileRange.Read(exINI, section, "ProjectileRange");
}

// #680 Chrono Prison / Abductor
/**
	This function checks if an abduction should be performed,
	and, if so, performs it, provided that is possible.

	\author Renegade
	\date 24.08.2010
	\param[in] Bullet The projectile that hits the victim.
	\retval true An abduction was performed. It should be assumed that the target is gone from the map at this point.
	\retval false No abduction was performed. This can be because the weapon is not an abductor, or because an error occurred. The target should still be on the map.
	\todo see if TechnoClass::Transporter needs to be set in here
*/
bool WeaponTypeExt::ExtData::conductAbduction(BulletClass * Bullet) {
	// ensuring a few base parameters
	if(!this->Abductor || !Bullet->Target || !Bullet->Owner) {
		return false;
	}

	auto Target = abstract_cast<FootClass*>(Bullet->Target);

	if(!Target) {
		// the target was not a valid passenger type
		return false;
	}

	auto Attacker = Bullet->Owner;
	auto TargetType = Target->GetTechnoType();
	auto TargetTypeExt = TechnoTypeExt::ExtMap.Find(TargetType);
	auto AttackerType = Attacker->GetTechnoType();

	//issue 1362
	if(TargetTypeExt->ImmuneToAbduction) {
		return false;
	}

	if(!WarheadTypeExt::canWarheadAffectTarget(Target, Attacker->Owner, Bullet->WH)) {
		return false;
	}

	if(Target->IsIronCurtained()) {
		return false;
	}

	//Don't abduct the target if it has more life then the abducting percent
	if(this->Abductor_AbductBelowPercent < Target->GetHealthPercentage()) {
		return false;
	}

	// Don't abduct the target if it's too fat in general, or if there's not enough room left in the hold // alternatively, NumPassengers
	if((TargetType->Size > AttackerType->SizeLimit)
		|| (TargetType->Size > (AttackerType->Passengers - Attacker->Passengers.GetTotalSize()))) {
		return false;
	}

	// if we ended up here, the target is of the right type, and the attacker can take it
	// so we abduct the target...

	Target->StopMoving();
	Target->SetDestination(nullptr, true); // Target->UpdatePosition(int) ?
	Target->SetTarget(nullptr);
	Target->CurrentTargets.Clear(); // Target->ShouldLoseTargetNow ?
	Target->SetFocus(nullptr);
	Target->QueueMission(mission_Sleep, true);
	Target->unknown_C4 = 0; // don't ask
	Target->unknown_5A0 = 0;
	Target->CurrentGattlingStage = 0;
	Target->SetCurrentWeaponStage(0);

	// the team should not wait for me
	if(Target->BelongsToATeam()) {
		Target->Team->LiberateMember(Target);
	}

	// if this unit is being mind controlled, break the link
	if(auto MindController = Target->MindControlledBy) {
		if(auto MC = MindController->CaptureManager) {
			MC->FreeUnit(Target);
		}
	}

	// if this unit is a mind controller, break the link
	if(Target->CaptureManager) {
		Target->CaptureManager->FreeAll();
	}

	// if this unit is currently in a state of temporal flux, get it back to our time-frame
	if(Target->TemporalTargetingMe) {
		Target->TemporalTargetingMe->Detach();
	}

	//if the target is spawned, detach it from it's spawner
	if(Target->SpawnOwner) {
		TechnoExt::DetachSpecificSpawnee(Target, HouseClass::FindSpecial());
	}

	// if the unit is a spawner, kill the spawns
	if(Target->SpawnManager) {
		Target->SpawnManager->KillNodes();
		Target->SpawnManager->ResetTarget();
	}

	//if the unit is a slave, it should be freed
	if(Target->SlaveOwner) {
		TechnoExt::FreeSpecificSlave(Target, HouseClass::FindSpecial());
	}

	// If the unit is a SlaveManager, free the slaves
	if(auto pSlaveManager = Target->SlaveManager) {
		pSlaveManager->Killed(Attacker);
		pSlaveManager->ZeroOutSlaves();
		Target->SlaveManager->Owner = Target;
	}

	// if we have an abducting animation, play it
	if(this->Abductor_AnimType) {
		GameCreate<AnimClass>(this->Abductor_AnimType, Bullet->posTgt);
		//this->Abductor_Anim->Owner=Bullet->Owner->Owner;
	}

	Target->Locomotor->Force_Track(-1, CoordStruct::Empty);
	CoordStruct coordsUnitSource = Target->GetCoords();
	Target->Locomotor->Mark_All_Occupation_Bits(0);
	Target->MarkAllOccupationBits(coordsUnitSource);
	Target->ClearPlanningTokens(nullptr);
	Target->Flashing.DurationRemaining = 0;

	//if it's owner meant to be changed, do it here
	if(this->Abductor_ChangeOwner && !TargetType->ImmuneToPsionics) {
		Target->SetOwningHouse(Attacker->Owner);
	}

	if(!Target->Remove()) {
		Debug::DevLog(Debug::Warning, "Abduction: Target unit %p (%s) could not be removed.\n", Target, Target->get_ID());
	}
	Target->OnBridge = false;

	// because we are throwing away the locomotor in a split second, piggybacking
	// has to be stopped. otherwise we would leak the memory of the original
	// locomotor saved in the piggy object.
	ILocomotion* Loco = nullptr;
	do {
		Loco = Target->Locomotor;
		LocomotionClass::End_Piggyback(Target->Locomotor);
	} while(Target->Locomotor && Loco != Target->Locomotor);

	// throw away the current locomotor and instantiate
	// a new one of the default type for this unit.
	if(!Target->Locomotor) {
		Game::RaiseError(E_POINTER);
	}
	ILocomotion* NewLoco = nullptr;
	if(LocomotionClass::CreateInstance(NewLoco, &TargetType->Locomotor)) {
		LocomotionClass::Move(Target->Locomotor, NewLoco);
		if(!Target->Locomotor) {
			Game::RaiseError(E_POINTER);
		}
		Target->Locomotor->Link_To_Object(Target);
	}

	// handling for Locomotor weapons: since we took this unit from the Magnetron
	// in an unfriendly way, set these fields here to unblock the unit
	if(Target->IsAttackedByLocomotor || Target->IsLetGoByLocomotor) {
		Target->IsAttackedByLocomotor = false;
		Target->IsLetGoByLocomotor = false;
		Target->FrozenStill = false;
	}

	Target->Transporter = Attacker;
	if(AttackerType->OpenTopped && Target->Owner->IsAlliedWith(Attacker)) {
		Attacker->EnteredOpenTopped(Target);
	}

	if(Attacker->WhatAmI() == abs_Building) {
		Target->Absorbed = true;
	}
	Attacker->AddPassenger(Target);
	Attacker->Undiscover();

	// ..and neuter the bullet, since it's not supposed to hurt the prisoner after the abduction
	Bullet->Health = 0;
	Bullet->DamageMultiplier = 0;
	Bullet->Remove();

	return true;
}

// Plants customizable IvanBombs on a target.
/*
	Plants a bomb and changes the customizable properties. Also, the weapon
	type that planted the bomb is remembered for use in hooks.

	The original Plant function has been changed to not play sounds any more,
	and it allows all kinds of TechnoClass sources, not just infantry.

	\param pSource The bomber techno who plants the bombs.
	\param pTarget The victim to be rigged.

	\author AlexB
	\date 2013-10-28
*/
void WeaponTypeExt::ExtData::PlantBomb(TechnoClass* pSource, ObjectClass* pTarget) const {
	// ensure target isn't rigged already
	if(pTarget && !pTarget->AttachedBomb) {
		BombListClass::Instance->Plant(pSource, pTarget);

		// if target has a bomb, planting was successful
		if(auto pBomb = pTarget->AttachedBomb) {
			WeaponTypeExt::BombExt[pBomb] = const_cast<ExtData*>(this);

			pBomb->DetonationFrame = Unsorted::CurrentFrame + this->Ivan_Delay.Get(RulesClass::Instance->IvanTimedDelay);
			pBomb->TickSound = this->Ivan_TickingSound.Get(RulesClass::Instance->BombTickingSound);

			int index = this->Ivan_AttachSound.Get(RulesClass::Instance->BombAttachSound);
			if(index != -1 && pSource->Owner->ControlledByPlayer()) {
				VocClass::PlayAt(index, pBomb->Target->Location, nullptr);
			}
		}
	}
}

// wave colors will be bound to the default values, thus a change of wave
// type will still point to the appropriate value, as long as the modder does not
// set the color by hand, in which case that value is used.
ColorStruct WeaponTypeExt::ExtData::GetWaveColor() const {
	auto pThis = this->AttachedToObject;

	if(pThis->IsMagBeam) {
		return this->Wave_Color.Get(WeaponTypeExt::ExtData::DefaultWaveColorMagBeam);
	} else if(pThis->IsSonic) {
		return this->Wave_Color.Get(WeaponTypeExt::ExtData::DefaultWaveColorSonic);
	} else {
		return this->Wave_Color.Get(WeaponTypeExt::ExtData::DefaultWaveColor);
	}
}

ColorStruct WeaponTypeExt::ExtData::GetBeamColor() const {
	auto pThis = this->AttachedToObject;

	if(pThis->IsRadBeam || pThis->IsRadEruption) {
		if(pThis->Warhead && pThis->Warhead->Temporal) { //Marshall added the check for Warhead because PrismForwarding.SupportWeapon does not require a Warhead
			// Well, a RadEruption Temporal will look pretty funny, but this is what WW uses
			return this->Beam_Color.Get(RulesClass::Instance->ChronoBeamColor);
		}
	}

	return this->Beam_Color.Get(RulesClass::Instance->RadColor);
}

void Container<WeaponTypeExt>::InvalidatePointer(void *ptr, bool bRemoved) {
	AnnounceInvalidPointerMap(WeaponTypeExt::BombExt, ptr);
	AnnounceInvalidPointerMap(WeaponTypeExt::WaveExt, ptr);
	AnnounceInvalidPointerMap(WeaponTypeExt::BoltExt, ptr);
	AnnounceInvalidPointerMap(WeaponTypeExt::RadSiteExt, ptr);
}

// =============================
// load / save

bool Container<WeaponTypeExt>::Save(WeaponTypeClass *pThis, IStream *pStm) {
	WeaponTypeExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if(pData) {
		ULONG out;

		DynamicVectorClass<BombClass *> *bombs = &BombListClass::Instance->Bombs;
		pStm->Write(&bombs->Count, 4, &out);
		for(int ii = 0; ii < bombs->Count; ++ii) {
			BombClass *ptr = bombs->Items[ii];
			pStm->Write(ptr, 4, &out);
			pStm->Write(WeaponTypeExt::BombExt[ptr], 4, &out);
		}

		pStm->Write(&WaveClass::Array->Count, 4, &out);
		for(int ii = 0; ii < WaveClass::Array->Count; ++ii) {
			WaveClass *ptr = WaveClass::Array->Items[ii];
			pStm->Write(ptr, 4, &out);
			pStm->Write(WeaponTypeExt::WaveExt[ptr], 4, &out);
		}

		pStm->Write(&RadSiteClass::Array->Count, 4, &out);
		for(int ii = 0; ii < RadSiteClass::Array->Count; ++ii) {
			RadSiteClass *ptr = RadSiteClass::Array->Items[ii];
			pStm->Write(ptr, 4, &out);
			pStm->Write(WeaponTypeExt::RadSiteExt[ptr], 4, &out);
		}
	}

	return pData != nullptr;
}

bool Container<WeaponTypeExt>::Load(WeaponTypeClass *pThis, IStream *pStm) {
	WeaponTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	ULONG out;

	int Count;
	WeaponTypeExt::ExtData* data;

	pStm->Read(&Count, 4, &out);
	for(int ii = 0; ii < Count; ++ii) {
		BombClass *bomb;
		pStm->Read(&bomb, 4, &out);
		SWIZZLE(bomb);

		pStm->Read(&data, 4, &out);
		SWIZZLE(data);
		WeaponTypeExt::BombExt[bomb] = data;
	}

	pStm->Read(&Count, 4, &out);
	for(int ii = 0; ii < Count; ++ii) {
		WaveClass *wave;
		pStm->Read(&wave, 4, &out);
		SWIZZLE(wave);

		pStm->Read(&data, 4, &out);
		SWIZZLE(data);
		WeaponTypeExt::WaveExt[wave] = data;
	}

	pStm->Read(&Count, 4, &out);
	for(int ii = 0; ii < Count; ++ii) {
		RadSiteClass *rad;
		pStm->Read(&rad, 4, &out);
		SWIZZLE(rad);

		pStm->Read(&data, 4, &out);
		SWIZZLE(data);
		WeaponTypeExt::RadSiteExt[rad] = data;
	}

	SWIZZLE(pData->Ivan_WH);
	SWIZZLE(pData->Ivan_Image);

	return pData != nullptr;
}

// =============================
// container hooks

DEFINE_HOOK(771EE9, WeaponTypeClass_CTOR, 5)
{
	GET(WeaponTypeClass*, pItem, ESI);

	WeaponTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(7730F0, WeaponTypeClass_SDDTOR, 5)
{
	GET(WeaponTypeClass*, pItem, ECX);

	WeaponTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(772EB0, WeaponTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(772CD0, WeaponTypeClass_SaveLoad_Prefix, 7)
{
	GET_STACK(WeaponTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<WeaponTypeExt>::PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(772EA6, WeaponTypeClass_Load_Suffix, 6)
{
	WeaponTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(772F8C, WeaponTypeClass_Save, 5)
{
	WeaponTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(7729C7, WeaponTypeClass_LoadFromINI, 5)
DEFINE_HOOK_AGAIN(7729D6, WeaponTypeClass_LoadFromINI, 5)
DEFINE_HOOK(7729B0, WeaponTypeClass_LoadFromINI, 5)
{
	GET(WeaponTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xE4);

	WeaponTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
