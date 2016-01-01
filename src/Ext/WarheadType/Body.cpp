#include "Body.h"
#include <WeaponTypeClass.h>
#include "../../Enum/ArmorTypes.h"
#include "../House/Body.h"
#include "../Techno/Body.h"
#include "../TechnoType/Body.h"
#include "../../Misc/EMPulse.h"
#include "../../Utilities/TemplateDef.h"

#include <WarheadTypeClass.h>
#include <GeneralStructures.h>
#include <HouseClass.h>
#include <ObjectClass.h>
#include <BulletClass.h>
#include <IonBlastClass.h>
#include <CellClass.h>
#include <TechnoClass.h>
#include <TechnoTypeClass.h>
#include <SlaveManagerClass.h>
#include <EMPulseClass.h>
#include <AnimClass.h>
#include "../Bullet/Body.h"
#include <FootClass.h>
#include "../../Utilities/Helpers.Alex.h"

#include <Helpers/Template.h>
#include <set>

template<> const DWORD Extension<WarheadTypeClass>::Canary = 0x22222222;
WarheadTypeExt::ExtContainer WarheadTypeExt::ExtMap;

AresMap<IonBlastClass*, const WarheadTypeExt::ExtData*> WarheadTypeExt::IonExt;

WarheadTypeClass * WarheadTypeExt::Temporal_WH = nullptr;

WarheadTypeClass * WarheadTypeExt::EMP_WH = nullptr;

void WarheadTypeExt::ExtData::Initialize() {
	if(!_strcmpi(this->OwnerObject()->ID, "NUKE")) {
		this->PreImpactAnim = AnimTypeClass::FindIndex("NUKEBALL");
		this->NukeFlashDuration = 30;
	}
}

void WarheadTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	auto pThis = this->OwnerObject();
	const char * section = pThis->ID;

	INI_EX exINI(pINI);

	if(!pINI->GetSection(section)) {
		return;
	}

	// writing custom verses parser just because
	if(pINI->ReadString(section, "Verses", "", Ares::readBuffer)) {
		int idx = 0;
		char* context = nullptr;
		for(char *cur = strtok_s(Ares::readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context)) {
			this->Verses[idx].Parse(cur);
			++idx;
			if(idx > 10) {
				break;
			}
		}
	}

	ArmorType::LoadForWarhead(pINI, pThis);

	if(pThis->MindControl) {
		this->MindControl_Permanent = pINI->ReadBool(section, "MindControl.Permanent", this->MindControl_Permanent);
	}

	this->EMP_Duration = pINI->ReadInteger(section, "EMP.Duration", this->EMP_Duration);
	this->EMP_Cap = pINI->ReadInteger(section, "EMP.Cap", this->EMP_Cap);
	this->EMP_Sparkles.Read(exINI, section, "EMP.Sparkles");

	this->IC_Duration = pINI->ReadInteger(section, "IronCurtain.Duration", this->IC_Duration);
	this->IC_Cap = pINI->ReadInteger(section, "IronCurtain.Cap", this->IC_Cap);

	if(pThis->Temporal) {
		this->Temporal_WarpAway.Read(exINI, section, "Temporal.WarpAway");
	}

	this->DeployedDamage = pINI->ReadDouble(section, "Damage.Deployed", this->DeployedDamage);

	this->Ripple_Radius = pINI->ReadInteger(section, "Ripple.Radius", this->Ripple_Radius);

	this->AffectsEnemies = pINI->ReadBool(section, "AffectsEnemies", this->AffectsEnemies);

	this->InfDeathAnim.Read(exINI, section, "InfDeathAnim");
	
	this->PreImpactAnim.Read(exINI, section, "PreImpactAnim");
	//this->NukeFlashDuration.Read(exINI, section, "NukeFlash.Duration");

	this->KillDriver = pINI->ReadBool(section, "KillDriver", this->KillDriver);

	this->KillDriver_KillBelowPercent.Read(exINI, section, "KillDriver.KillBelowPercent");

	this->KillDriver_Owner.Read(exINI, section, "KillDriver.Owner");

	this->Malicious.Read(exINI, section, "Malicious");

	this->PreventScatter.Read(exINI, section, "PreventScatter");

	this->CellSpread_MaxAffect.Read(exINI, section, "CellSpread.MaxAffect");

	this->AttachedEffect.Read(exINI);

	//this->DamageAirThreshold.Read(exINI, section, "DamageAirThreshold");

	this->SuppressDeathWeapon_Vehicles.Read(exINI, section, "DeathWeapon.SuppressVehicles");
	this->SuppressDeathWeapon_Infantry.Read(exINI, section, "DeathWeapon.SuppressInfantry");
	this->SuppressDeathWeapon.Read(exINI, section, "DeathWeapon.Suppress");
};

/*!
	This function checks if the passed warhead has Ripple.Radius set, and, if so, applies the effect.
	\note Moved here from hook BulletClass_Fire.
	\param coords The coordinates of the warhead impact, the center of the Ripple area.
*/
void WarheadTypeExt::ExtData::applyRipples(const CoordStruct &coords) {
	if(this->Ripple_Radius) {
		auto const pBlast = GameCreate<IonBlastClass>(coords);
		pBlast->DisableIonBeam = TRUE;
		WarheadTypeExt::IonExt[pBlast] = this;
	}
}

// Applies this warhead's Iron Curtain effect.
/*!
	This function checks if the passed warhead has IronCurtain.Duration set,
	and, if so, applies the effect.

	Units will be damaged before the Iron Curtain gets effective. AffectAllies
	and AffectEnemies are respected. Verses support is limited: If it is 0%,
	the unit won't get affected, otherwise, it will be 100% affected.

	\note Moved here from hook BulletClass_Fire.

	\param coords The coordinates of the warhead impact, the center of the Iron Curtain area.
	\param Owner Owner of the Iron Curtain effect, i.e. the one triggering this.
	\param damage The damage the firing weapon deals before the Iron Curtain effect starts.

	\date 2010-06-28
*/
void WarheadTypeExt::ExtData::applyIronCurtain(const CoordStruct &coords, HouseClass* Owner, int damage) {
	CellStruct cellCoords = MapClass::Instance->GetCellAt(coords)->MapCoords;

	if(this->IC_Duration != 0) {
		// set of affected objects. every object can be here only once.
		auto items = Helpers::Alex::getCellSpreadItems(coords, this->OwnerObject()->CellSpread, true);

		// affect each object
		for(auto curTechno : items) {
			// don't protect the dead
			if(!curTechno || curTechno->InLimbo || !curTechno->IsAlive || !curTechno->Health) {
				continue;
			}

			// affects enemies or allies respectively?
			if(!WarheadTypeExt::CanAffectTarget(curTechno, Owner, this->OwnerObject())) {
				continue;
			}

			// duration modifier
			int duration = this->IC_Duration;

			auto pType = curTechno->GetTechnoType();

			// modify good durations only
			if(duration > 0) {
				if(auto pData = TechnoTypeExt::ExtMap.Find(pType)) {
					duration = static_cast<int>(duration * pData->IronCurtain_Modifier);
				}
			}

			// respect verses the boolean way
			if(std::abs(this->GetVerses(pType->Armor).Verses) < 0.001) {
				continue;
			}

			// get the values
			int oldValue = (curTechno->IronCurtainTimer.Expired() ? 0 : curTechno->IronCurtainTimer.GetTimeLeft());
			int newValue = Helpers::Alex::getCappedDuration(oldValue, duration, this->IC_Cap);

			// update iron curtain
			if(oldValue <= 0) {
				// start iron curtain effect?
				if(newValue > 0) {
					// damage the victim before ICing it
					if(damage) {
						curTechno->ReceiveDamage(&damage, 0, this->OwnerObject(), nullptr, true, false, Owner);
					}

					// unit may be destroyed already.
					if(curTechno->IsAlive) {
						// start and prevent the multiplier from being applied twice
						curTechno->IronCurtain(newValue, Owner, false);
						curTechno->IronCurtainTimer.Start(newValue);
					}
				}
			} else {
				// iron curtain effect is already on.
				if(newValue > 0) {
					// set new length and reset tint stage
					curTechno->IronCurtainTimer.Start(newValue);
					curTechno->IronTintStage = 4;
				} else {
					// turn iron curtain off
					curTechno->IronCurtainTimer.Stop();
				}
			}
		}
	}
}

/*!
	This function checks if the passed warhead has EMP.Duration set, and, if so, applies the effect.
	\note Moved here from hook BulletClass_Fire.
	\param coords The coordinates of the warhead impact, the center of the EMP area.
	\param source The unit that launched the EMP.
*/
void WarheadTypeExt::ExtData::applyEMP(const CoordStruct &coords, TechnoClass *source) {
	if (this->EMP_Duration) {
		// launch our rewritten EMP.
		EMPulse::CreateEMPulse(this, coords, source);
	}
}

/*!
	This function checks if the passed warhead has MindControl.Permanent set, and, if so, applies the effect.
	\note Moved here from hook BulletClass_Fire.
	\param Owner Owner of the Mind Control effect, i.e. the one controlling the target afterwards.
	\param Target Target of the Mind Control effect, i.e. the one being controlled by the owner afterwards.
	\return false if effect wasn't applied, true if it was.
		This is important for the chain of damage effects, as, in case of true, the target is now a friendly unit.
*/
bool WarheadTypeExt::ExtData::applyPermaMC(HouseClass* const Owner, AbstractClass* const Target) const {
	if(this->MindControl_Permanent && Owner) {
		if(auto const pTarget = abstract_cast<TechnoClass*>(Target)) {
			auto const pType = pTarget->GetTechnoType();

			if(!pType->ImmuneToPsionics) {
				if(auto const pController = pTarget->MindControlledBy) {
					pController->CaptureManager->FreeUnit(pTarget);
				}

				pTarget->SetOwningHouse(Owner, true);
				pTarget->MindControlledByAUnit = true;
				pTarget->QueueMission(Mission::Guard, false);

				if(auto& pAnim = pTarget->MindControlRingAnim) {
					pAnim->UnInit();
					pAnim = nullptr;
				}

				auto const pBld = abstract_cast<BuildingClass*>(pTarget);

				CoordStruct location = pTarget->GetCoords();
				if(pBld) {
					location.Z += pBld->Type->Height * Unsorted::LevelHeight;
				} else {
					location.Z += pType->MindControlRingOffset;
				}

				if(auto const pAnimType = RulesClass::Instance->PermaControlledAnimationType) {
					if(auto const pAnim = GameCreate<AnimClass>(pAnimType, location)) {
						pTarget->MindControlRingAnim = pAnim;
						pAnim->SetOwnerObject(pTarget);
						if(pBld) {
							pAnim->ZAdjust = -1024;
						}
					}
				}

				return true;
			}
		}
	}

	return false;
}

/*!
	This function checks if the projectile transporting the warhead should pass through
		the building's walls and deliver the warhead to the occupants instead. If so, it performs that effect.
	\note Moved here from hook BulletClass_Fire.
	\note This cannot logically be triggered in situations where the warhead is not delivered by a projectile,
		such as the GenericWarhead super weapon.
	\param Bullet The projectile
	\todo This should probably be moved to /Ext/Bullet/ instead, I just maintained the previous structure to ease transition.
		Since UC.DaMO (#778) in 0.5 will require a reimplementation of this logic anyway,
		we can probably just leave it here until then.
*/
void WarheadTypeExt::applyOccupantDamage(BulletClass* const Bullet) {
	if(auto const pExt = BulletExt::ExtMap.Find(Bullet)) {
		if(pExt->DamageOccupants()) {
			// the occupants have been damaged, do not damage the building (the original target)
			Bullet->Health = 0;
			Bullet->DamageMultiplier = 0;
			Bullet->Remove();
		}
	}
}

//! Gets whether a Techno can be affected by a warhead fired by a house.
/*!
	A warhead will not affect allies if AffectsAllies is not set and will not
	affect enemies if AffectsEnemies is not set.

	\param pTarget The Techno pWarhead is fired at.
	\param pSourceHouse The house that fired pWarhead.
	\param pWarhead The fired warhead.

	\returns True if pWarhead can affect pTarget, false otherwise.

	\author AlexB
	\date 2010-04-27
*/
bool WarheadTypeExt::CanAffectTarget(TechnoClass* const pTarget, HouseClass* const pSourceHouse, WarheadTypeClass* const pWarhead) {
	if(pSourceHouse && pTarget && pWarhead) {
		// apply AffectsAllies if owner and target house are allied
		if(pSourceHouse->IsAlliedWith(pTarget->Owner)) {
			return pWarhead->AffectsAllies;
		}

		// this warhead is designed to ignore enemy units
		const auto pExt = WarheadTypeExt::ExtMap.Find(pWarhead);
		return pExt->AffectsEnemies;
	}

	return true;
}

// Request #733: KillDriver/"Jarmen Kell"
/*! This function checks if the KillDriver effect should be applied, and, if so, applies it.
	\param pSource Pointer to the firing unit
	\param pVictim Pointer to the target unit
	\return true if the effect was applied, false if not
	\author Renegade & AlexB
	\date 05.04.10
	\todo This needs to be refactored to work with the generic warhead SW. I want to create a generic cellspread function first.
*/
bool WarheadTypeExt::ExtData::applyKillDriver(
	TechnoClass* const pSource, AbstractClass* const pVictim) const
{
	if(!pSource || !this->KillDriver) {
		return false;
	}

	if(auto const pTarget = abstract_cast<FootClass*>(pVictim)) {
		// don't penetrate the Iron Curtain // typedef IronCurtain ChastityBelt
		if(pTarget->BeingWarpedOut || pTarget->IsIronCurtained()) {
			return false;
		}

		// target must be Vehicle or Aircraft
		if(!Helpers::Alex::is_any_of(
			pTarget->WhatAmI(), AbstractType::Unit, AbstractType::Aircraft))
		{
			return false;
		}

		auto const pTargetType = pTarget->GetTechnoType();
		auto const pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType);

		// because these tags kinda have negative meaning, less means better.
		// if the driver is protected, he can by default only be killed if
		// health is below 0.0, while 1.0 means always killable.
		auto const maxKillHealth = Math::min(
			pTargetTypeExt->ProtectedDriver_MinHealth.Get(
				pTargetTypeExt->ProtectedDriver ? 0.0 : 1.0),
			this->KillDriver_KillBelowPercent);

		// conditions: not protected and not a living being
		if(!pTargetType->Natural && !pTargetType->Organic
			&& pTarget->GetHealthPercentage() <= maxKillHealth)
		{
			// if this aircraft is expected to dock to anything, don't allow killing its pilot
			// (reason being: the game thinks you lost the aircraft that just turned, and assumes you have free aircraft space,
			// allowing you to build more aircraft, for the docking spot that is still occupied by the previous plane.)
			if(auto const pAircraftType = abstract_cast<AircraftTypeClass*>(pTargetType)) {
				if(pAircraftType->AirportBound || pAircraftType->Dock.Count) {
					return false;
				}
			}

			// get the new owner
			auto const pInvoker = pSource->Owner;
			auto pOwner = HouseExt::GetHouseKind(this->KillDriver_Owner, false,
				nullptr, pInvoker, pInvoker, pTarget->Owner);
			if(!pOwner) {
				pOwner = HouseClass::FindSpecial();
			}

			auto const passive = pOwner->Type->MultiplayPassive;

			auto const TargetExt = TechnoExt::ExtMap.Find(pTarget);
			TargetExt->DriverKilled = passive;

			// exit if owner would not change
			if(pTarget->Owner == pOwner) {
				return false;
			}

			// If this vehicle uses Operator=, we have to take care of actual "physical" drivers, rather than theoretical ones
			if(pTargetTypeExt->IsAPromiscuousWhoreAndLetsAnyoneRideIt && pTarget->Passengers.GetFirstPassenger()) {
				// kill first passenger
				auto const pPassenger = pTarget->RemoveFirstPassenger();
				pPassenger->RegisterDestruction(pSource);
				pPassenger->UnInit();

			} else if(auto const pOperatorType = pTargetTypeExt->Operator) {
				// find the driver cowardly hiding among the passengers, then kill him
				for(NextObject passenger(pTarget->Passengers.GetFirstPassenger()); passenger; ++passenger) {
					auto const pPassenger = static_cast<FootClass*>(*passenger);
					if(pPassenger->GetTechnoType() == pOperatorType) {
						pTarget->RemovePassenger(pPassenger);
						pPassenger->RegisterDestruction(pSource);
						pPassenger->UnInit();
						break;
					}
				}
			}

			// if passengers remain in the vehicle, operator-using or not, they should leave
			if(pTarget->Passengers.GetFirstPassenger()) {
				TechnoExt::EjectPassengers(pTarget, -1);
			}

			// remove the hijacker
			pTarget->HijackerInfantryType = -1;

			// If this unit is driving under influence, we have to free it first
			if(auto const pController = pTarget->MindControlledBy) {
				if(auto const pCaptureManager = pController->CaptureManager) {
					pCaptureManager->FreeUnit(pTarget);
				}
			}
			pTarget->MindControlledByAUnit = false;
			pTarget->MindControlledByHouse = nullptr;

			// remove the mind-control ring anim
			if(pTarget->MindControlRingAnim) {
				pTarget->MindControlRingAnim->UnInit();
				pTarget->MindControlRingAnim = nullptr;
			}

			// If this unit mind controls stuff, we should free the controllees, since they still belong to the previous owner
			if(pTarget->CaptureManager) {
				pTarget->CaptureManager->FreeAll();
			}

			// This unit will be freed of its duties
			if(auto const pFoot = abstract_cast<FootClass*>(pTarget)) {
				if(pFoot->BelongsToATeam()) {
					pFoot->Team->LiberateMember(pFoot);
				}
			}

			// If this unit spawns stuff, we should kill the spawns, since they still belong to the previous owner
			if(auto const pSpawnManager = pTarget->SpawnManager) {
				pSpawnManager->KillNodes();
				pSpawnManager->ResetTarget();
			}

			// If this unit enslaves stuff, we should free the slaves, since they still belong to the previous owner
			// <DCoder> SlaveManagerClass::Killed() sets the manager's Owner to NULL
			// <Renegade> okay, does Killed() also destroy the slave manager, or just unlink it from the unit?
			// <DCoder> unlink
			// <Renegade> so on principle, I could just re-link it?
			// <DCoder> yes you can
			if(auto const pSlaveManager = pTarget->SlaveManager) {
				pSlaveManager->Killed(pSource);
				pSlaveManager->ZeroOutSlaves();
				pSlaveManager->Owner = pTarget;
				if(passive) {
					pSlaveManager->SuspendWork();
				} else {
					pSlaveManager->ResumeWork();
				}
			}

			// Hand over to a different house
			pTarget->SetOwningHouse(pOwner);

			if(passive) {
				pTarget->QueueMission(Mission::Harmless, true);
			}

			pTarget->SetTarget(nullptr);
			pTarget->SetDestination(nullptr, false);
			return true;
		}
	}
	return false;
}

//AttachedEffects, request #1573, #255
//copy-pasted from AlexB's applyIC
//since CellSpread effect is needed due to MO's proposed cloak SW (which is the reason why I was bugged with this), it has it.
//Graion Dilach, ~2011-10-14... I forgot the exact date :S

void WarheadTypeExt::ExtData::applyAttachedEffect(const CoordStruct &coords, TechnoClass* const Owner) {
	if(this->AttachedEffect.Duration != 0) {
		// set of affected objects. every object can be here only once.
		const auto items = Helpers::Alex::getCellSpreadItems(coords, this->OwnerObject()->CellSpread, true);

		// affect each object
		for(const auto curTechno : items) {
			// don't attach to dead
			if(!curTechno || curTechno->InLimbo || !curTechno->IsAlive || !curTechno->Health) {
				continue;
			}

			if(Owner && !WarheadTypeExt::CanAffectTarget(curTechno, Owner->Owner, this->OwnerObject())) {
				continue;
			}

			if(std::abs(this->GetVerses(curTechno->GetTechnoType()->Armor).Verses) < 0.001) {
				continue;
			}

			this->AttachedEffect.Attach(curTechno, this->AttachedEffect.Duration, Owner);
		}
	}
}

// =============================
// load / save

template <typename T>
void WarheadTypeExt::ExtData::Serialize(T& Stm) {
	Stm
		.Process(this->MindControl_Permanent)
		.Process(this->Ripple_Radius)
		.Process(this->EMP_Duration)
		.Process(this->EMP_Cap)
		.Process(this->EMP_Sparkles)
		.Process(this->IC_Duration)
		.Process(this->IC_Cap)
		.Process(this->Verses)
		.Process(this->DeployedDamage)
		.Process(this->Temporal_WarpAway)
		.Process(this->AffectsEnemies)
		.Process(this->InfDeathAnim)
		.Process(this->PreImpactAnim)
		.Process(this->NukeFlashDuration)
		.Process(this->KillDriver)
		.Process(this->KillDriver_KillBelowPercent)
		.Process(this->KillDriver_Owner)
		.Process(this->Malicious)
		.Process(this->PreventScatter)
		.Process(this->CellSpread_MaxAffect)
		.Process(this->AttachedEffect)
		.Process(this->DamageAirThreshold)
		.Process(this->SuppressDeathWeapon_Vehicles)
		.Process(this->SuppressDeathWeapon_Infantry)
		.Process(this->SuppressDeathWeapon);
}

void WarheadTypeExt::ExtData::LoadFromStream(AresStreamReader &Stm) {
	Extension<WarheadTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void WarheadTypeExt::ExtData::SaveToStream(AresStreamWriter &Stm) {
	Extension<WarheadTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool WarheadTypeExt::LoadGlobals(AresStreamReader& Stm) {
	return Stm
		.Process(Temporal_WH)
		.Process(EMP_WH)
		.Process(IonExt)
		.Success();
}

bool WarheadTypeExt::SaveGlobals(AresStreamWriter& Stm) {
	return Stm
		.Process(Temporal_WH)
		.Process(EMP_WH)
		.Process(IonExt)
		.Success();
}

// =============================
// container

WarheadTypeExt::ExtContainer::ExtContainer() : Container("WarheadTypeClass") {
}

WarheadTypeExt::ExtContainer::~ExtContainer() = default;

void WarheadTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) {
	AnnounceInvalidPointer(WarheadTypeExt::Temporal_WH, ptr);
}

// =============================
// container hooks

DEFINE_HOOK(75D1A9, WarheadTypeClass_CTOR, 7)
{
	GET(WarheadTypeClass*, pItem, EBP);

	WarheadTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(75E5C8, WarheadTypeClass_SDDTOR, 6)
{
	GET(WarheadTypeClass*, pItem, ESI);

	WarheadTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(75E2C0, WarheadTypeClass_SaveLoad_Prefix, 5)
DEFINE_HOOK(75E0C0, WarheadTypeClass_SaveLoad_Prefix, 8)
{
	GET_STACK(WarheadTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	WarheadTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(75E2AE, WarheadTypeClass_Load_Suffix, 7)
{
	WarheadTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(75E39C, WarheadTypeClass_Save_Suffix, 5)
{
	WarheadTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(75DEAF, WarheadTypeClass_LoadFromINI, 5)
DEFINE_HOOK(75DEA0, WarheadTypeClass_LoadFromINI, 5)
{
	GET(WarheadTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x150);

	WarheadTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
