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
Container<WarheadTypeExt> WarheadTypeExt::ExtMap;

AresMap<IonBlastClass*, const WarheadTypeExt::ExtData*> WarheadTypeExt::IonExt;

WarheadTypeClass * WarheadTypeExt::Temporal_WH = nullptr;

WarheadTypeClass * WarheadTypeExt::EMP_WH = nullptr;

void WarheadTypeExt::ExtData::Initialize() {
	if(!_strcmpi(this->OwnerObject()->ID, "NUKE")) {
		this->PreImpactAnim = AnimTypeClass::FindIndex("NUKEBALL");
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
	if(pINI->ReadString(section, "Verses", "", Ares::readBuffer, Ares::readLength)) {
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

	this->KillDriver = pINI->ReadBool(section, "KillDriver", this->KillDriver);

	this->KillDriver_KillBelowPercent.Read(exINI, section, "KillDriver.KillBelowPercent");

	this->KillDriver_Owner.Read(exINI, section, "KillDriver.Owner");

	this->Malicious.Read(exINI, section, "Malicious");

	this->PreventScatter.Read(exINI, section, "PreventScatter");

	this->CellSpread_MaxAffect.Read(exINI, section, "CellSpread.MaxAffect");

	this->AttachedEffect.Read(exINI);
};

void Container<WarheadTypeExt>::InvalidatePointer(void *ptr, bool bRemoved) {
	AnnounceInvalidPointer(WarheadTypeExt::Temporal_WH, ptr);
}

/*!
	This function checks if the passed warhead has Ripple.Radius set, and, if so, applies the effect.
	\note Moved here from hook BulletClass_Fire.
	\param coords The coordinates of the warhead impact, the center of the Ripple area.
*/
void WarheadTypeExt::ExtData::applyRipples(const CoordStruct &coords) {
	if (this->Ripple_Radius) {
		IonBlastClass *IB = GameCreate<IonBlastClass>(coords);
		IB->DisableIonBeam = TRUE;
		WarheadTypeExt::IonExt[IB] = this;
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
			if(!WarheadTypeExt::canWarheadAffectTarget(curTechno, Owner, this->OwnerObject())) {
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
	\param coords The coordinates of the warhead impact, the center of the Mind Control animation.
	\param Owner Owner of the Mind Control effect, i.e. the one controlling the target afterwards.
	\param Target Target of the Mind Control effect, i.e. the one being controlled by the owner afterwards.
	\return false if effect wasn't applied, true if it was.
		This is important for the chain of damage effects, as, in case of true, the target is now a friendly unit.
*/
bool WarheadTypeExt::ExtData::applyPermaMC(const CoordStruct &coords, HouseClass* Owner, AbstractClass* Target) {
	if (this->MindControl_Permanent && Target) {
		if (TechnoClass *pTarget = generic_cast<TechnoClass *>(Target)) {
			TechnoTypeClass *pType = pTarget->GetTechnoType();

			if (!pType || pType->ImmuneToPsionics) {
				return false; // should return 0 in hook
			}
			if (pTarget->MindControlledBy) {
				pTarget->MindControlledBy->CaptureManager->FreeUnit(pTarget);
			}
			pTarget->SetOwningHouse(Owner, 1);
			pTarget->MindControlledByAUnit = 1;
			pTarget->QueueMission(Mission::Guard, 0);

			CoordStruct XYZ = coords;
			XYZ.Z += pType->MindControlRingOffset;

			AnimClass *MCAnim = GameCreate<AnimClass>(RulesClass::Instance->PermaControlledAnimationType, XYZ);
			AnimClass *oldMC = pTarget->MindControlRingAnim;
			if (oldMC) {
				oldMC->UnInit();
			}
			pTarget->MindControlRingAnim = MCAnim;
			MCAnim->SetOwnerObject(pTarget);
			return true; // should return 0x469AA4 in hook
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
void WarheadTypeExt::applyOccupantDamage(BulletClass* Bullet) {
	if (Bullet->Target) {
		BulletExt::ExtData* TheBulletExt = BulletExt::ExtMap.Find(Bullet);
		if (TheBulletExt->DamageOccupants()) {
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

	\param Target The Techno WH is fired at.
	\param SourceHouse The house that fired WH.
	\param WH The fired warhead.

	\returns True if WH can affect Target, false otherwise.

	\author AlexB
	\date 2010-04-27
*/
bool WarheadTypeExt::canWarheadAffectTarget(TechnoClass * Target, HouseClass * SourceHouse, WarheadTypeClass *WH) {
	if (SourceHouse && Target && WH) {
		// owner and target house are allied and this warhead
		// is set to not hurt any allies.
		bool alliedWithTarget = SourceHouse->IsAlliedWith(Target->Owner);
		if (alliedWithTarget && !WH->AffectsAllies) {
			return false;
		}

		// this warhead's pulse is designed to fly around
		// enemy units. useful for healing.
		WarheadTypeExt::ExtData *pWHdata = WarheadTypeExt::ExtMap.Find(WH);
		if (!alliedWithTarget && !pWHdata->AffectsEnemies) {
			return false;
		}
	}

	return true;
}

// Request #733: KillDriver/"Jarmen Kell"
/*! This function checks if the KillDriver effect should be applied, and, if so, applies it.
	\param Bullet Pointer to the bullet
	\return true if the effect was applied, false if not
	\author Renegade & AlexB
	\date 05.04.10
	\todo This needs to be refactored to work with the generic warhead SW. I want to create a generic cellspread function first.
*/
bool WarheadTypeExt::ExtData::applyKillDriver(BulletClass* Bullet) {
	if(!Bullet->Target || !this->KillDriver) {
		return false;
	}

	if(auto pTarget = abstract_cast<FootClass*>(Bullet->Target)) {
		// don't penetrate the Iron Curtain // typedef IronCurtain ChastityBelt
		if(pTarget->IsIronCurtained()) {
			return false;
		}
		auto pTargetType = pTarget->GetTechnoType();
		auto pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType);

		// conditions: Warhead is KillDriver, target is Vehicle or Aircraft, but not protected and not a living being
		if(((pTarget->WhatAmI() == AbstractType::Unit) || (pTarget->WhatAmI() == AbstractType::Aircraft))
			&& !(pTarget->BeingWarpedOut || pTargetTypeExt->ProtectedDriver || pTargetType->Organic || pTargetType->Natural)
			&& (pTarget->GetHealthPercentage() <= this->KillDriver_KillBelowPercent)) {

			// if this aircraft is expected to dock to anything, don't allow killing its pilot
			// (reason being: the game thinks you lost the aircraft that just turned, and assumes you have free aircraft space,
			// allowing you to build more aircraft, for the docking spot that is still occupied by the previous plane.)
			if(auto pAircraftType = abstract_cast<AircraftTypeClass*>(pTargetType)) {
				if(pAircraftType->AirportBound || pAircraftType->Dock.Count) {
					return false;
				}
			}

			// If this vehicle uses Operator=, we have to take care of actual "physical" drivers, rather than theoretical ones
			if(pTargetTypeExt->IsAPromiscuousWhoreAndLetsAnyoneRideIt && pTarget->Passengers.GetFirstPassenger()) {
				// kill first passenger
				auto pPassenger = pTarget->RemoveFirstPassenger();
				pPassenger->RegisterDestruction(Bullet->Owner);
				pPassenger->UnInit();

			} else if(auto pOperatorType = pTargetTypeExt->Operator) {
				// find the driver cowardly hiding among the passengers, then kill him
				for(auto pPassenger = pTarget->Passengers.GetFirstPassenger(); pPassenger; pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject)) {
					if(pPassenger->GetTechnoType() == pOperatorType) {
						pTarget->RemovePassenger(pPassenger);
						pPassenger->RegisterDestruction(Bullet->Owner);
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
			if(auto pController = pTarget->MindControlledBy) {
				if(auto pCaptureManager = pController->CaptureManager) {
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
			if(auto pFoot = abstract_cast<FootClass*>(pTarget)) {
				if(pFoot->BelongsToATeam()) {
					pFoot->Team->LiberateMember(pFoot);
				}
			}

			// If this unit spawns stuff, we should kill the spawns, since they still belong to the previous owner
			if(auto pSpawnManager = pTarget->SpawnManager) {
				pSpawnManager->KillNodes();
				pSpawnManager->ResetTarget();
			}

			// If this unit enslaves stuff, we should free the slaves, since they still belong to the previous owner
			// <DCoder> SlaveManagerClass::Killed() sets the manager's Owner to NULL
			// <Renegade> okay, does Killed() also destroy the slave manager, or just unlink it from the unit?
			// <DCoder> unlink
			// <Renegade> so on principle, I could just re-link it?
			// <DCoder> yes you can
			if(auto pSlaveManager = pTarget->SlaveManager) {
				pSlaveManager->Killed(Bullet->Owner);
				pSlaveManager->ZeroOutSlaves();
				pSlaveManager->Owner = pTarget;
				pSlaveManager->SuspendWork();
			}

			auto TargetExt = TechnoExt::ExtMap.Find(pTarget);
			TargetExt->DriverKilled = true;

			// Hand over to a different house
			auto pOwner = HouseExt::GetHouseKind(this->KillDriver_Owner, false,
				nullptr, nullptr, nullptr, nullptr);
			if(!pOwner) {
				pOwner = HouseClass::FindSpecial();
			}

			pTarget->SetOwningHouse(pOwner);
			pTarget->QueueMission(Mission::Harmless, true);
			return true;
		}
	}
	return false;
}

//AttachedEffects, request #1573, #255
//copy-pasted from AlexB's applyIC
//since CellSpread effect is needed due to MO's proposed cloak SW (which is the reason why I was bugged with this), it has it.
//Graion Dilach, ~2011-10-14... I forgot the exact date :S

void WarheadTypeExt::ExtData::applyAttachedEffect(const CoordStruct &coords, TechnoClass* Owner) {
	if (this->AttachedEffect.Duration != 0) {
		CellStruct cellCoords = MapClass::Instance->GetCellAt(coords)->MapCoords;
		// set of affected objects. every object can be here only once.
		auto items = Helpers::Alex::getCellSpreadItems(coords, this->OwnerObject()->CellSpread, true);

		// affect each object
		for(auto curTechno : items) {
			// don't attach to dead
			if(!curTechno || curTechno->InLimbo || !curTechno->IsAlive || !curTechno->Health) {
				continue;
			}

			if (Owner) {
				if(WarheadTypeExt::canWarheadAffectTarget(curTechno, Owner->Owner, this->OwnerObject())) {
					if(abs(this->GetVerses(curTechno->GetTechnoType()->Armor).Verses) < 0.001) {
						continue;
					}
					//this->AttachedEffect.Attach(curTechno, this->AttachedEffect.Duration, Owner, this->AttachedEffect.DamageDelay);
					this->AttachedEffect.Attach(curTechno, this->AttachedEffect.Duration, Owner);
				}
			} else {
				//this->AttachedEffect.Attach(curTechno, this->AttachedEffect.Duration, nullptr, this->AttachedEffect.DamageDelay);
				this->AttachedEffect.Attach(curTechno, this->AttachedEffect.Duration, nullptr);
			}
		}
	}
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
