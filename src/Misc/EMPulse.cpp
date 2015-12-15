#include "EMPulse.h"
#include "../Ext/Building/Body.h"
#include "../Ext/Techno/Body.h"
#include "../Ext/TechnoType/Body.h"
#include "../Ext/BuildingType/Body.h"
#include "../Utilities/Helpers.Alex.h"
#include <set>

//! Enables verbose debug output for some WarheadTypeExt functions.
bool EMPulse::verbose = false;
bool EMPulse::supportVerses = false;

template <typename... TArgs>
void EMP_Log(const char* pFormat, TArgs&&... args) {
	//Debug::Log(EMPulse::verbose, pFormat, std::forward<TArgs>(args)...);
}

//! Paralyses all units using an EMP cellspread weapon.
/*!
	All Technos in the EMPulse's target cells get affected by an EMP and get
	deactivated temporarily. Their special functions stop working until the
	EMP ceases. Flying Aircraft crashes.

	\param pWarhead The warhead that creates the electromagnetic pulse.
	\param coords The location the projectile detonated.
	\param pFirer The Techno that fired the pulse.

	\author AlexB
	\date 2010-05-20
*/
void EMPulse::CreateEMPulse(
	WarheadTypeExt::ExtData* const pWarhead, CoordStruct const& coords,
	TechnoClass* const pFirer)
{
	if(!pWarhead) {
		Debug::Log(Debug::Severity::Error,
			"Trying to CreateEMPulse() with Warhead pointing to NULL.\n");
		return;
	}

	// set of affected objects. every object can be here only once.
	auto const items = Helpers::Alex::getCellSpreadItems(
		coords, pWarhead->OwnerObject()->CellSpread, true);

	EMP_Log("[CreateEMPulse] Duration: %d, Cap: %d on %u objects\n",
		pWarhead->EMP_Duration, pWarhead->EMP_Cap, items.size());

	// affect each object
	for(const auto& pItem : items) {
		deliverEMPDamage(pItem, pFirer, pWarhead);
	}

	EMP_Log("[CreateEMPulse] Done.\n");
}

//! Deals EMP damage the object.
/*!
	Applies, removes or alters the EMP effect on a given unit.

	\param pTechno The Techno that should get affected by EMP.
	\param pFirer The object that fired this EMP.
	\param pWarhead The warhead causing the EMP.

	\author AlexB
	\date 2010-06-30
*/
void EMPulse::deliverEMPDamage(
	TechnoClass* const pTechno, TechnoClass* const pFirer,
	WarheadTypeExt::ExtData* pWarhead)
{
	EMP_Log("[deliverEMPDamage] Step 1: %s => %s\n",
		(pFirer ? pFirer->get_ID() : nullptr), pTechno->get_ID());

	auto const pHouse = pFirer ? pFirer->Owner : nullptr;
	if(isEligibleEMPTarget(pTechno, pHouse, pWarhead->OwnerObject())) {
		auto const pType = pTechno->GetTechnoType();
		EMP_Log("[deliverEMPDamage] Step 2: %s\n", pType->get_ID());

		// get the target-specific multiplier
		auto modifier = 1.0;
		if(auto const pExt = TechnoTypeExt::ExtMap.Find(pType)) {
			// modifier only affects bad things
			if(pWarhead->EMP_Duration > 0) {
				modifier = pExt->EMP_Modifier;
			}
		}

		// respect verses
		auto const& Verses = pWarhead->GetVerses(pType->Armor).Verses;
		auto duration = static_cast<int>(pWarhead->EMP_Duration * modifier);
		if(supportVerses) {
			duration = static_cast<int>(duration * Verses);
		} else if(std::abs(Verses) < 0.001) {
			return;
		}

		// get the new capped value
		auto const oldValue = static_cast<int>(pTechno->EMPLockRemaining);
		auto const newValue = Helpers::Alex::getCappedDuration(
			oldValue, duration, pWarhead->EMP_Cap);

		EMP_Log("[deliverEMPDamage] Step 3: %d\n", newValue);

		// can not be less than zero
		pTechno->EMPLockRemaining = static_cast<DWORD>(Math::max(newValue, 0));
		EMP_Log("[deliverEMPDamage] Step 4: %d\n", newValue);

		auto diedFromPulse = false;

		auto const underEMPBefore = (oldValue > 0);
		auto const underEMPAfter = (pTechno->EMPLockRemaining > 0);
		auto const newlyUnderEMP = !underEMPBefore && underEMPAfter;
		if(underEMPBefore && !underEMPAfter) {
			// newly de-paralyzed
			EMP_Log("[deliverEMPDamage] Step 5a\n");
			DisableEMPEffect(pTechno);
		} else if(newlyUnderEMP) {
			// newly paralyzed unit
			EMP_Log("[deliverEMPDamage] Step 5b\n");
			diedFromPulse = enableEMPEffect(pTechno, pFirer);
		} else if(oldValue == newValue) {
			// no relevant change
			EMP_Log("[deliverEMPDamage] Step 5c\n");
			return;
		}

		// is techno destroyed by EMP?
		if(diedFromPulse || (underEMPAfter && thresholdExceeded(pTechno))) {
			TechnoExt::Destroy(pTechno, pFirer);
		} else if(newlyUnderEMP || pWarhead->EMP_Sparkles) {
			// set the sparkle animation
			UpdateSparkleAnim(pTechno, pWarhead->EMP_Sparkles);
		}
	}
}

//! Gets whether a Techno is type immune to EMP.
/*!
	Type immunity does work a little different for EMP weapons than for ordinary
	projectiles. Target is type immune to EMP if it has a weapon that uses a
	warhead also having EMP.Duration set. It is irrelevant which Techno fired
	the EMP.

	If Target is elite, only EliteWeapons are used to check for a non-zero
	EMP.Duration, otherwise only ordinary Weapons are used.

	\param Target The Techno to check EMP type immunity for.

	\returns True if Target is type immune to EMP, false otherwise.

	\author AlexB
	\date 2010-04-27, 2014-02-01
*/
bool EMPulse::isEMPTypeImmune(TechnoClass* Target) {
	auto pType = Target->GetTechnoType();
	if(!pType->TypeImmune) {
		return false;
	}

	auto isElite = Target->Veterancy.IsElite();
	auto const& Weapons = *(isElite ? &pType->EliteWeapon : &pType->Weapon);

	// find an emp weapon.
	for(auto const& Weapon : Weapons) {
		if(auto pWeaponType = Weapon.WeaponType) {
			auto pWarheadExt = WarheadTypeExt::ExtMap.Find(pWeaponType->Warhead);
			if(pWarheadExt->EMP_Duration != 0) {
				// this unit can fire emps and type immunity
				// grants it to never be affected.
				return true;
			}
		}
	}

	return false;
}

//! Gets whether a Techno is immune to an EMP fired by a house.
/*!
	Target can not be affected by EMPs if it is either immune to EMPs or type immune.
	It may also have recieved EMP immunity through it's veterancy.

	\param Target The Techno the EMP is fired at.
	\param SourceHouse The house that fired the EMP.

	\returns True if Target is immune to EMP, false otherwise.

	\author AlexB
	\date 2010-05-02
*/
bool EMPulse::isEMPImmune(TechnoClass * Target, HouseClass * SourceHouse) {
	// this can be overridden by a flag on the techno.
	auto pData = TechnoTypeExt::ExtMap.Find(Target->GetTechnoType());

	if(pData->ImmuneToEMP.Get(!IsTypeEMPProne(pData->OwnerObject()))) {
		EMP_Log("[isEMPImmune] \"%s\" is ImmuneToEMP.\n", Target->get_ID());
		return true;
	}

	// this may be immune because of veteran and elite abilities.
	if(Target->Veterancy.IsElite() && (pData->EliteAbilityEMPIMMUNE || pData->VeteranAbilityEMPIMMUNE)) {
		EMP_Log("[isEMPImmune] \"%s\" is immune because it is elite.\n", Target->get_ID());
		return true;
	} else if(Target->Veterancy.IsVeteran() && pData->VeteranAbilityEMPIMMUNE) {
		EMP_Log("[isEMPImmune] \"%s\" is immune because it is veteran.\n", Target->get_ID());
		return true;
	}

	// if houses differ, TypeImmune does not count.
	if(Target->Owner == SourceHouse) {
		// ignore if type immune. don't even try.
		if(isEMPTypeImmune(Target)) {
			// This unit can fire emps and type immunity
			// grants it to never be affected.
			EMP_Log("[isEMPImmune] \"%s\" is TypeImmune.\n", Target->get_ID());
			return true;
		}
	}

	return false;
}

//! Gets whether a Techno is currently immune to EMPs fired by a house.
/*!
	Target may not be immune to EMP but its current status still prevents it from
	being affected by them.

	Currently, EMPs can be temporarily averted if Target is under the influence of
	the Iron Curtain or it is attacked by a Temporal warhead.

	\param Target The Techno the EMP is fired at.
	\param SourceHouse The house that fired the EMP.

	\returns True if Target is immune to EMP, false otherwise.

	\author AlexB
	\date 2010-05-26
*/
bool EMPulse::isCurrentlyEMPImmune(TechnoClass * Target, HouseClass * SourceHouse) {
	// objects currently doing some time travel are exempt
	if(Target->BeingWarpedOut) {
		return true;
	}

	// iron curtained objects can not be affected by EMPs
	if(Target->IsIronCurtained()) {
		return true;
	}

	if(Target->WhatAmI() == AbstractType::Unit) {
		if(BuildingClass* pBld = MapClass::Instance->GetCellAt(Target->Location)->GetBuilding()) {
			if(pBld->Type->WeaponsFactory) {
				if(pBld->IsUnderEMP() || pBld == Target->GetNthLink(0)) {
					EMP_Log("[isCurrentlyEMPImmune] %s should not be disabled. Still in war factory: %s\n", Target->get_ID(), pBld->get_ID());
					return true;
				}

				// units requiring an operator can't deactivate on the bib
				// because nobody could enter it afterwards.
				TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(Target);
				if(!pData->IsOperated()) {
					EMP_Log("[isCurrentlyEMPImmune] %s should not be disabled. Would be unoperated on bib: %s\n", Target->get_ID(), pBld->get_ID());
					return true;
				}
			}
		}
	}

	// the current status does allow this target to
	// be affected by EMPs. It may be immune, though.
	return isEMPImmune(Target, SourceHouse);
}

//! Gets whether an EMP would have an effect on a TechnoType.
/*!
	Infantry that has Cyborg set is prone to EMP.

	Non-Infantry Foots are prone only if they aren't organic.

	Buildings with special functions like Radar, SuperWeapon, PowersUnit,
	Sensors, LaserFencePost, GapGenerator and UndeploysInto are prone. Otherwise
	a Building is prone if it consumes power and is Powered.

	\param Type The TechnoType to get the susceptibility to EMP of.

	\returns True if Type is prone to EMPs, false otherwise.

	\author AlexB
	\date 2010-04-30, 2014-02-01
*/
bool EMPulse::IsTypeEMPProne(TechnoTypeClass const* const pType) {
	auto const abs = pType->WhatAmI();

	if(abs == AbstractType::BuildingType) {
		auto const pBldType = static_cast<BuildingTypeClass const*>(pType);

		// exclude invisible buildings
		if(pBldType->InvisibleInGame) {
			return false;
		}

		// buildings are prone if they consume power and need it to function
		if(pBldType->Powered && pBldType->PowerDrain > 0) {
			return true;
		}

		// may have a special function.
		return pBldType->Radar
			|| pBldType->HasSuperWeapon()
			|| pBldType->UndeploysInto
			|| pBldType->PowersUnit
			|| pBldType->Sensors
			|| pBldType->LaserFencePost
			|| pBldType->GapGenerator;

	} else if(abs == AbstractType::InfantryType) {
		// affected only if this is a cyborg.
		auto const pInfType = static_cast<InfantryTypeClass const*>(pType);
		return pInfType->Cyborg;
	} else {
		// if this is a vessel or vehicle that is organic: no effect.
		return !pType->Organic;
	}
}

//! Gets whether a Techno is a valid target for EMPs fired by a house.
/*!
	An eligible target is not currently immune to EMPs and the EMP would have an
	effect.

	\param Target The Techno the EMP will be fired at.
	\param SourceHouse The house that fired the EMP.

	\returns True Target is eligible, false otherwise.

	\author AlexB
	\date 2010-04-30
*/
bool EMPulse::isEligibleEMPTarget(
	TechnoClass* const pTarget, HouseClass* const pSourceHouse,
	WarheadTypeClass* const pWarhead)
{
	if(!WarheadTypeExt::CanAffectTarget(pTarget, pSourceHouse, pWarhead)) {
		EMP_Log("[isEligibleEMPTarget] \"%s\" does not affect target.\n",
			pWarhead->ID);
		return false;
	}

	return !isCurrentlyEMPImmune(pTarget, pSourceHouse);
}

//! Gets whether this Techno should be deactivated right now.
/*!
	Objects that are currently deploying, constructing, or selling should
	not be deactivated.

	\param Target The Techno to be deactivated.

	\returns True if Target should be deactivated, false otherwise.

	\author AlexB
	\date 2010-05-09
*/
bool EMPulse::IsDeactivationAdvisable(TechnoClass* Target) {
	switch(Target->CurrentMission)
	{
	case Mission::Selling:
	case Mission::Construction:
	case Mission::Unload:
		EMP_Log("[IsDeactivationAdvisable] %s should not be disabled. Mission: %d\n", Target->get_ID(), Target->CurrentMission);
		return false;
	default:
		EMP_Log("[IsDeactivationAdvisable] %s should be disabled. Mission: %d\n", Target->get_ID(), Target->CurrentMission);
		return true;
	}
}

//! Updates the radar outage for the owning player.
/*!
	If this is a structure providing radar or spy satellite abilities the
	original check for radar facilities is invoked by setting a flag.

	\param pBuilding The building that might be a Radar or SpySat structure.

	\author AlexB
	\date 2010-11-28
*/
void EMPulse::updateRadarBlackout(BuildingClass* const pBuilding) {
	auto const pType = pBuilding->Type;
	if(pType->Radar || pType->SpySat) {
		pBuilding->Owner->RecheckRadar = true;
	}
}

//! Updates the SpawnManager to account for the EMP effect.
/*!
	Defers all regeneration and spawning until after the EMP effect is over.
	If spawned units are on the go, that is, launching or flying, they are
	destroyed.

	\param Techno The Techno that might be a spawner.
	\param Source The Object possibly destroying the spawns.

	\author AlexB
	\date 2010-05-03
*/
void EMPulse::updateSpawnManager(TechnoClass* Techno, ObjectClass* Source = nullptr) {
	auto pSM = Techno->SpawnManager;

	if(!pSM) {
		return;
	}

	if(Techno->EMPLockRemaining > 0) {
		// crash all spawned units that are visible. else, they'd land somewhere else.
		for(auto pSpawn : pSM->SpawnedNodes) {
			// kill every spawned unit that is in the air. exempt missiles.
			if(pSpawn->IsSpawnMissile == FALSE && pSpawn->Unit) {
				auto Status = pSpawn->Status;
				if(Status >= SpawnNodeStatus::TakeOff && Status <= SpawnNodeStatus::Returning) {
					TechnoExt::Destroy(pSpawn->Unit, abstract_cast<TechnoClass*>(Source));
				}
			}
		}

		// pause the timers so spawning and regenerating is deferred.
		pSM->SpawnTimer.Pause();
		pSM->UpdateTimer.Pause();
	} else {
		// resume counting.
		pSM->SpawnTimer.Resume();
		pSM->UpdateTimer.Resume();
	}
}

//! Returns the default sparkle anim type for a techno.
/*!
	\param pTechno The techno to get the animation for.

	\author AlexB
	\date 2015-07-16
*/
AnimTypeClass* EMPulse::getSparkleAnimType(TechnoClass const* const pTechno) {
	auto const pType = pTechno->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	return pTypeExt->EMP_Sparkles.Get(RulesClass::Instance->EMPulseSparkles);
}

//! Updates the sparkle animation of Techno.
/*!
	Enables or disables the EMP sparkle animation.

	\param pTechno The Techno to update.
	\param pSpecific Optional anim type to create.

	\author AlexB
	\date 2010-05-09, 2015-07-16
*/
void EMPulse::UpdateSparkleAnim(
	TechnoClass* const pTechno, AnimTypeClass* const pSpecific)
{
	auto const pData = TechnoExt::ExtMap.Find(pTechno);
	auto& Anim = pData->EMPSparkleAnim;

	if(pTechno->IsUnderEMP()) {
		if(!Anim) {
			auto const pAnimType = pSpecific ? pSpecific
				: getSparkleAnimType(pTechno);

			if(pAnimType) {
				Anim = GameCreate<AnimClass>(pAnimType, pTechno->Location);
				Anim->SetOwnerObject(pTechno);
				if(auto const pBld = abstract_cast<BuildingClass*>(pTechno)) {
					Anim->ZAdjust = -1024;
				}
			}
		}
	} else if(Anim) {
		// finish this loop, then disappear
		Anim->RemainingIterations = 0;
		Anim = nullptr;
	}
}

//! Updates the sparkle animation of Techno, given another Techno.
/*!
	Recreates the proper animation given another Techno. This is used for
	swapping Technos on the battlefield between deployed and undeployed forms.

	\param pTechno The Techno to update.
	\param pFrom The Techno to copy the animation from.

	\author AlexB
	\date 2015-07-16
*/
void EMPulse::UpdateSparkleAnim(
	TechnoClass* const pTechno, TechnoClass const* const pFrom)
{
	AnimTypeClass* pSpecific = nullptr;

	auto const pExt = TechnoExt::ExtMap.Find(pFrom);
	if(auto const pAnim = pExt->EMPSparkleAnim) {
		// the current anim is not the default anim, thus was created from
		// a warhead. use the same type
		auto const pAnimType = getSparkleAnimType(pFrom);
		if(pAnimType != pAnim->Type) {
			pSpecific = pAnim->Type;
		}
	}

	UpdateSparkleAnim(pTechno, pSpecific);
}

//! If the victim is owned by the human player creates radar events and EVA warnings.
/*!
	Creates a radar event and makes EVA tell you so if the Techno is a resource
	gatherer or harvester. If it is a building that can not be undeployed and
	counts as BaseNormal a base under attack event is raised.

	\param Techno The Techno that has been attacked.

	\author AlexB
	\date 2010-04-29
*/
void EMPulse::announceAttack(TechnoClass* Techno) {
	enum class AttackEvents { None = 0, Base = 1, Harvester = 2 };
	AttackEvents Event = AttackEvents::None;

	// find out what event is the most appropriate.
	if(Techno && Techno->Owner == HouseClass::Player) {
		if(auto pBuilding = abstract_cast<BuildingClass*>(Techno)) {
			if(pBuilding->Type->ResourceGatherer) {
				// slave miner, for example
				Event = AttackEvents::Harvester;
			} else if(!pBuilding->Type->Insignificant && !pBuilding->Type->BaseNormal) {
				Event = AttackEvents::Base;
			}
		} else if(auto pUnit = abstract_cast<UnitClass*>(Techno)) {
			if(pUnit->Type->Harvester || pUnit->Type->ResourceGatherer) {
				Event = AttackEvents::Harvester;
			}
		}
	}

	// handle the event.
	switch(Event) {
	case AttackEvents::Harvester:
		if(RadarEventClass::Create(RadarEventType::HarvesterAttacked, Techno->GetMapCoords()))
			VoxClass::Play("EVA_OreMinerUnderAttack", -1, -1);
		break;
	case AttackEvents::Base:
		HouseClass::Player->BuildingUnderAttack(abstract_cast<BuildingClass*>(Techno));
		break;
	case AttackEvents::None:
	default:
		break;
	}
}

//! Checks whether a Techno should be considered destroyed.
/*!
	A techno can be destroyed by excessive EMP damage. If the treshold
	is positive it acts as a upper limit. If it is negative it is only
	a limit if the unit is in the air (parachuting doesn't count).

	\param Victim The Techno that is under EMP effect.

	\returns True if Victim has taken too much EMP damage, False otherwise.

	\author AlexB
	\date 2010-05-20
*/
bool EMPulse::thresholdExceeded(TechnoClass * Victim) {
	auto const pData = TechnoTypeExt::ExtMap.Find(Victim->GetTechnoType());

	EMP_Log("[thresholdExceeded] %s: %d %d\n", Victim->get_ID(), pData->EMP_Threshold, Victim->EMPLockRemaining);

	if(pData->EMP_Threshold != 0 && Victim->EMPLockRemaining > static_cast<DWORD>(std::abs(pData->EMP_Threshold))) {
		if(pData->EMP_Threshold > 0 || (Victim->IsInAir() && !Victim->Parachute)) {
			return true;
		}
	}

	return false;
}

//! Sets all properties to disable a Techno.
/*!
	Disables Buildings and crashes flying Aircrafts. Foots get deactivated.
	An EMP sparkle animation is created and played until the EMP effect
	ceases.

	Contains special handling to create a radar outage equalling the length
	of the EMP effect, if Victim is a Building with Radar capabilities.

	If Victim mind controls any units, they are freed. Spawned units are
	killed.

	This function may be called only once, and only if the victim hasn't been
	EMP-disabled yet. This function might not deactivate the victim right away,
	but it has to do all the other stuff now, or Slave Miners won't stop working.
	Or other bad stuff happens.

	\param pVictim The Techno that is under EMP effect.
	\param pSource The house to credit kills to.

	\returns True if pVictim has been destroyed by the EMP effect, False otherwise.

	\author AlexB
	\date 2010-05-21
*/
bool EMPulse::enableEMPEffect(
	TechnoClass* const pVictim, ObjectClass* const pSource)
{
	auto const abs = pVictim->WhatAmI();

	if(abs == AbstractType::Building) {
		auto const pBuilding = static_cast<BuildingClass*>(pVictim);
		auto const pOwner = pBuilding->Owner;

		pOwner->RecheckTechTree = true;
		pOwner->RecheckPower = true;

		pBuilding->DisableStuff();
		updateRadarBlackout(pBuilding);
	} else if(abs == AbstractType::Aircraft) {
		// crash flying aircraft
		auto const pAircraft = static_cast<AircraftClass*>(pVictim);
		if(pAircraft->GetHeight() > 0) {
			EMP_Log("[enableEMPEffect] Plane crash: %s\n", pAircraft->get_ID());
			return true;
		}
	}

	// cache the last mission this thing did
	auto const pExt = TechnoExt::ExtMap.Find(pVictim);
	pExt->EMPLastMission = pVictim->CurrentMission;

	// detach temporal
	if(pVictim->IsWarpingSomethingOut()) {
		pVictim->TemporalImUsing->LetGo();
	}

	// remove the unit from its team
	if(auto const pFoot = abstract_cast<FootClass*>(pVictim)) {
		if(pFoot->BelongsToATeam()) {
			pFoot->Team->LiberateMember(pFoot);
		}
	}

	// deactivate and sparkle
	if(!pVictim->Deactivated && IsDeactivationAdvisable(pVictim)) {
		auto const selected = pVictim->IsSelected;
		auto const pFocus = pVictim->Focus;

		pVictim->Deactivate();

		if(selected) {
			auto const feedback = Unsorted::MoveFeedback;
			Unsorted::MoveFeedback = false;
			pVictim->Select();
			Unsorted::MoveFeedback = feedback;
		}

		if(abs == AbstractType::Building) {
			pVictim->Focus = pFocus;
		}
	}

	// release all captured units.
	if(pVictim->CaptureManager) {
		pVictim->CaptureManager->FreeAll();
	}

	// update managers.
	updateSpawnManager(pVictim, pSource);

	if(auto const pSlaveManager = pVictim->SlaveManager) {
		pSlaveManager->SuspendWork();
	}

	// warn the player
	announceAttack(pVictim);

	// the unit still lives.
	return false;
}

//! Sets all properties to re-enable a Techno.
/*!
	Reactivates the Techno. The EMP sparkle animation is stopped.

	This function may be called only once, and only if the EMP effect
	is on.

	\param pVictim The Techno that shall have its EMP effects removed.

	\author AlexB
	\date 2010-05-03
*/
void EMPulse::DisableEMPEffect(TechnoClass* const pVictim) {
	auto const abs = pVictim->WhatAmI();

	auto hasPower = true;

	if(abs == AbstractType::Building) {
		auto const pBuilding = static_cast<BuildingClass*>(pVictim);
		hasPower = pBuilding->IsPowerOnline();

		auto const pOwner = pBuilding->Owner;
		pOwner->RecheckTechTree = true;
		pOwner->RecheckPower = true;

		auto const pType = pBuilding->Type;
		if(hasPower || pType->LaserFencePost) {
			pBuilding->EnableStuff();
		}
		updateRadarBlackout(pBuilding);
	}

	if(hasPower && pVictim->Deactivated) {
		auto const pFocus = pVictim->Focus;
		pVictim->Reactivate();
		if(abs == AbstractType::Building) {
			pVictim->Focus = pFocus;
		}
	}

	// allow to spawn units again.
	updateSpawnManager(pVictim);

	if(auto const pSlaveManager = pVictim->SlaveManager) {
		pSlaveManager->ResumeWork();
	}

	// update the animation
	UpdateSparkleAnim(pVictim);

	// get harvesters back to work and ai units to hunt
	if(auto const pFoot = abstract_cast<FootClass*>(pVictim)) {
		auto hasMission = false;
		if(abs == AbstractType::Unit) {
			auto const pUnit = static_cast<UnitClass*>(pVictim);
			if(pUnit->Type->Harvester || pUnit->Type->ResourceGatherer) {
				// prevent unloading harvesters from being irritated.
				auto const pExt = TechnoExt::ExtMap.Find(pVictim);
				auto const mission = pExt->EMPLastMission != Mission::Guard
					? pExt->EMPLastMission : Mission::Enter;

				pUnit->QueueMission(mission, true);
				hasMission = true;
			}
		}

		if(!hasMission && !pFoot->Owner->ControlledByHuman()) {
			pFoot->QueueMission(Mission::Hunt, false);
		}
	}
}

// the functions below are not related to EMP. they aren't official
// and certainly don't endorse you to use them. 2011-05-14 AlexB

bool EMPulse::EnableEMPEffect2(TechnoClass* const pVictim) {
	auto const abs = pVictim->WhatAmI();

	if(abs == AbstractType::Building) {
		auto const pBuilding = static_cast<BuildingClass*>(pVictim);
		auto const pOwner = pBuilding->Owner;

		pOwner->RecheckTechTree = true;
		pOwner->RecheckPower = true;

		pBuilding->DisableStuff();
		updateRadarBlackout(pBuilding);
	} else if(abs == AbstractType::Aircraft) {
		// crash flying aircraft
		auto const pAircraft = static_cast<AircraftClass*>(pVictim);
		if(pAircraft->GetHeight() > 0) {
			return true;
		}
	}

	// deactivate and sparkle
	if(!pVictim->Deactivated && IsDeactivationAdvisable(pVictim)) {
		// cache the last mission this thing did
		auto const pExt = TechnoExt::ExtMap.Find(pVictim);
		pExt->EMPLastMission = pVictim->CurrentMission;

		// detach temporal
		if(pVictim->IsWarpingSomethingOut()) {
			pVictim->TemporalImUsing->LetGo();
		}

		// remove the unit from its team
		if(auto const pFoot = abstract_cast<FootClass*>(pVictim)) {
			if(pFoot->BelongsToATeam()) {
				pFoot->Team->LiberateMember(pFoot);
			}
		}

		auto const selected = pVictim->IsSelected;
		auto const pFocus = pVictim->Focus;

		pVictim->Deactivate();

		if(selected) {
			auto const feedback = Unsorted::MoveFeedback;
			Unsorted::MoveFeedback = false;
			pVictim->Select();
			Unsorted::MoveFeedback = feedback;
		}

		if(abs == AbstractType::Building) {
			pVictim->Focus = pFocus;
		}

		if(abstract_cast<FootClass*>(pVictim)) {
			pVictim->QueueMission(Mission::Sleep, true);
		}

		// release all captured units.
		if(pVictim->CaptureManager) {
			pVictim->CaptureManager->FreeAll();
		}

		// update managers.
		updateSpawnManager(pVictim, nullptr);

		if(auto const pSlaveManager = pVictim->SlaveManager) {
			pSlaveManager->SuspendWork();
		}
	}

	// the unit still lives.
	return false;
}

void EMPulse::DisableEMPEffect2(TechnoClass* const pVictim) {
	auto const abs = pVictim->WhatAmI();

	auto const pExt = TechnoExt::ExtMap.Find(pVictim);
	auto hasPower = pExt->IsPowered() && pExt->IsOperated();

	if(abs == AbstractType::Building) {
		auto const pBuilding = static_cast<BuildingClass*>(pVictim);
		hasPower = hasPower && pBuilding->IsPowerOnline();

		auto const pOwner = pBuilding->Owner;
		pOwner->RecheckTechTree = true;
		pOwner->RecheckPower = true;

		if(hasPower) {
			pBuilding->EnableStuff();
		}
		updateRadarBlackout(pBuilding);
	}

	if(hasPower && pVictim->Deactivated) {
		auto const pFocus = pVictim->Focus;
		pVictim->Reactivate();
		if(abs == AbstractType::Building) {
			pVictim->Focus = pFocus;
		}

		// allow to spawn units again.
		updateSpawnManager(pVictim);

		if(auto const pSlaveManager = pVictim->SlaveManager) {
			pSlaveManager->ResumeWork();
		}

		// get harvesters back to work and ai units to hunt
		if(auto const pFoot = abstract_cast<FootClass*>(pVictim)) {
			auto hasMission = false;
			if(abs == AbstractType::Unit) {
				auto const pUnit = static_cast<UnitClass*>(pVictim);
				if(pUnit->Type->Harvester || pUnit->Type->ResourceGatherer) {
					// prevent unloading harvesters from being irritated.
					auto const mission = pExt->EMPLastMission != Mission::Guard
						? pExt->EMPLastMission : Mission::Enter;

					pUnit->QueueMission(mission, true);
					hasMission = true;
				}
			}

			if(!hasMission && !pFoot->Owner->ControlledByHuman()) {
				pFoot->QueueMission(Mission::Hunt, false);
			}
		}
	}
}
