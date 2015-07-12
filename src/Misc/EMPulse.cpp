#include "EMPulse.h"
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
	//if(EMPulse::verbose) {
	//	Debug::Log(pFormat, std::forward<TArgs>(args)...);
	//}
}

//! Paralyses all units using an EMP cellspread weapon.
/*!
	All Technos in the EMPulse's target cells get affected by an EMP and get
	deactivated temporarily. Their special functions stop working until the
	EMP ceases. Flying Aircraft crashes.

	\param EMPulse The electromagnetic pulse to create.
	\param Coords The location the projectile detonated.
	\param Firer The Techno that fired the pulse.

	\author AlexB
	\date 2010-05-20
*/
void EMPulse::CreateEMPulse(WarheadTypeExt::ExtData *Warhead, const CoordStruct &Coords, TechnoClass *Firer) {
	if (!Warhead) {
		Debug::DevLog(Debug::Error, "Trying to CreateEMPulse() with Warhead pointing to NULL. Funny.\n");
		return;
	}

	EMP_Log("[CreateEMPulse] Duration: %d, Cap: %d\n", Warhead->EMP_Duration, Warhead->EMP_Cap);

	// set of affected objects. every object can be here only once.
	auto items = Helpers::Alex::getCellSpreadItems(Coords, Warhead->OwnerObject()->CellSpread, true);

	// affect each object
	for(const auto& Item : items) {
		deliverEMPDamage(Item, Firer, Warhead);
	}

	EMP_Log("[CreateEMPulse] Done.\n");
}

//! Deals EMP damage the object.
/*!
	Applies, removes or alters the EMP effect on a given unit.

	\param object The Techno that should get affected by EMP.

	\author AlexB
	\date 2010-06-30
*/
void EMPulse::deliverEMPDamage(ObjectClass *object, TechnoClass *Firer, WarheadTypeExt::ExtData *Warhead) {
	// fill the gaps
	HouseClass *pHouse = (Firer ? Firer->Owner : nullptr);

	if (TechnoClass * curTechno = generic_cast<TechnoClass *> (object)) {
		EMP_Log("[deliverEMPDamage] Step 1: %s => %s\n", (Firer ? Firer->get_ID() : nullptr), curTechno->get_ID());

		if (isEligibleEMPTarget(curTechno, pHouse, Warhead->OwnerObject())) {
			EMP_Log("[deliverEMPDamage] Step 2: %s\n", curTechno->get_ID());

			// get the target-specific multiplier
			double modifier = 1.0;
			if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(curTechno->GetTechnoType())) {
				// modifier only affects bad things
				if(Warhead->EMP_Duration > 0) {
					modifier = pExt->EMP_Modifier;
				}
			}

			// respect verses
			int duration = static_cast<int>(Warhead->EMP_Duration * modifier);
			if(supportVerses) {
				duration = static_cast<int>(duration * Warhead->GetVerses(curTechno->GetTechnoType()->Armor).Verses);
			} else if(abs(Warhead->GetVerses(curTechno->GetTechnoType()->Armor).Verses) < 0.001) {
				return;
			}

			// get the new capped value
			int oldValue = curTechno->EMPLockRemaining;
			int newValue = Helpers::Alex::getCappedDuration(oldValue, duration, Warhead->EMP_Cap);

			EMP_Log("[deliverEMPDamage] Step 3: %d\n", newValue);

			// can not be less than zero
			curTechno->EMPLockRemaining = std::max(0, newValue);
			EMP_Log("[deliverEMPDamage] Step 4: %d\n", newValue);

			// newly de-paralyzed
			if ((oldValue > 0) && (curTechno->EMPLockRemaining <= 0)) {
				EMP_Log("[deliverEMPDamage] Step 5a\n");
				DisableEMPEffect(curTechno);
			} else if ((oldValue <= 0) && (curTechno->EMPLockRemaining > 0)) {
				// newly paralyzed unit
				EMP_Log("[deliverEMPDamage] Step 5b\n");
				if (enableEMPEffect(curTechno, Firer)) {
					return;
				}
			} else if (oldValue != newValue) {
				// At least update the radar, if this is one.
				EMP_Log("[deliverEMPDamage] Step 5c\n");
				updateRadarBlackout(curTechno);
			}

			// is techno destroyed by EMP?
			if (thresholdExceeded(curTechno)) {
				TechnoExt::Destroy(curTechno, Firer);
			}
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

	// find an emp weapon.
	for(int i = 0; i < 18; ++i) {
		if(auto pWeaponType = !isElite ? pType->get_Weapon(i) : pType->get_EliteWeapon(i)) {
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
	if (Target->Veterancy.IsElite() && (pData->EliteAbilityEMPIMMUNE || pData->VeteranAbilityEMPIMMUNE)) {
		EMP_Log("[isEMPImmune] \"%s\" is immune because it is elite.\n", Target->get_ID());
		return true;
	} else if (Target->Veterancy.IsVeteran() && pData->VeteranAbilityEMPIMMUNE) {
		EMP_Log("[isEMPImmune] \"%s\" is immune because it is veteran.\n", Target->get_ID());
		return true;
	}

	// if houses differ, TypeImmune does not count.
	if (Target->Owner == SourceHouse) {
		// ignore if type immune. don't even try.
		if (isEMPTypeImmune(Target)) {
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
	if (Target->BeingWarpedOut) {
		return true;
	}

	// iron curtained objects can not be affected by EMPs
	if (Target->IsIronCurtained()) {
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
bool EMPulse::isEligibleEMPTarget(TechnoClass * Target, HouseClass * SourceHouse, WarheadTypeClass *Warhead) {
	if (isCurrentlyEMPImmune(Target, SourceHouse)) {
		return false;
	}

	if (!WarheadTypeExt::CanAffectTarget(Target, SourceHouse, Warhead)) {
		EMP_Log("[isEligibleEMPTarget] \"%s\" does not affect target.\n", Warhead->ID);
		return false;
	}

	return true;
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

	\param Techno The Techno that might be a Radar or SpySat structure.

	\author AlexB
	\date 2010-11-28
*/
void EMPulse::updateRadarBlackout(TechnoClass* Techno) {
	if(auto pBuilding = abstract_cast<BuildingClass*>(Techno)) {
		if(!pBuilding->Type->InvisibleInGame) {
			if(pBuilding->Type->Radar || pBuilding->Type->SpySat) {
				pBuilding->Owner->RecheckRadar = true;
			}
		}
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

//! Updates the SlaveManager to account for the EMP effect.
/*!
	Stops the slaves where they are standing until the EMP effect is over.

	\param Techno The Techno that might be an enslaver.

	\author AlexB
	\date 2010-05-07
*/
void EMPulse::updateSlaveManager(TechnoClass* Techno) {
	if(auto pSM = Techno->SlaveManager) {
		if(Techno->EMPLockRemaining > 0) {
			// pause the timers so spawning and regenerating is deferred.
			pSM->SuspendWork();
		} else {
			// resume slaving around.
			pSM->ResumeWork();
		}
	}
}

//! Updates the sparkle animation of Techno.
/*!
	Enables or disables the EMP sparkle animation.

	\param Techno The Techno to update.

	\author AlexB
	\date 2010-05-09
*/
void EMPulse::UpdateSparkleAnim(TechnoClass* const pTechno) {
	auto const pData = TechnoExt::ExtMap.Find(pTechno);
	auto& Anim = pData->EMPSparkleAnim;

	if(pTechno->IsUnderEMP()) {
		if(!Anim) {
			auto const pType = pTechno->GetTechnoType();
			auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
			auto const pAnimType = pTypeExt->EMP_Sparkles.Get(
				RulesClass::Instance->EMPulseSparkles);

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

	\param Victim The Techno that is under EMP effect.
	\param Source The house to credit kills to.

	\returns True if Victim has been destroyed by the EMP effect, False otherwise.

	\author AlexB
	\date 2010-05-21
*/
bool EMPulse::enableEMPEffect(TechnoClass * Victim, ObjectClass * Source) {
	Victim->Owner->RecheckTechTree = true;
	Victim->Owner->RecheckPower = true;

	if (BuildingClass * Building = specific_cast<BuildingClass *>(Victim)) {
		Building->DisableStuff();
		updateRadarBlackout(Building);

		BuildingTypeClass * pType = Building->Type;
		if (pType->Factory != AbstractType::None) {
			Building->Owner->Update_FactoriesQueues(pType->Factory, pType->Naval, BuildCat::DontCare);
		}
	} else {
		if (AircraftClass * Aircraft = specific_cast<AircraftClass *>(Victim)) {
			// crash flying aircraft
			if (Aircraft->GetHeight() > 0) {
				EMP_Log("[enableEMPEffect] Plane crash: %s\n", Aircraft->get_ID());
				TechnoExt::Destroy(Victim, generic_cast<TechnoClass*>(Source));
				return true;
			}
		}
	}

	// cache the last mission this thing did
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(Victim);
	pData->EMPLastMission = Victim->CurrentMission;

	// detach temporal
	if(Victim->IsWarpingSomethingOut()) {
		Victim->TemporalImUsing->LetGo();
	}
	
	// remove the unit from its team
	if (FootClass * Foot = generic_cast<FootClass *>(Victim)) {
		if (Foot->BelongsToATeam()) {
			Foot->Team->LiberateMember(Foot);
		}
	}

	// deactivate and sparkle
	if (!Victim->Deactivated && IsDeactivationAdvisable(Victim)) {
		bool selected = Victim->IsSelected;
		Victim->Deactivate();
		if(selected) {
			bool feedback = Unsorted::MoveFeedback;
			Unsorted::MoveFeedback = false;
			Victim->Select();
			Unsorted::MoveFeedback = feedback;
		}
	}

	// release all captured units.
	if (Victim->CaptureManager) {
		Victim->CaptureManager->FreeAll();
	}

	// update managers.
	updateSpawnManager(Victim, Source);
	updateSlaveManager(Victim);

	// set the sparkle animation.
	UpdateSparkleAnim(Victim);

	// warn the player
	announceAttack(Victim);

	// the unit still lives.
	return false;
}

//! Sets all properties to re-enable a Techno.
/*!
	Reactivates the Techno. The EMP sparkle animation is stopped.

	This function may be called only once, and only if the EMP effect
	is on.

	\param Victim The Techno that shall have its EMP effects removed.

	\author AlexB
	\date 2010-05-03
*/
void EMPulse::DisableEMPEffect(TechnoClass * Victim) {
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(Victim);
	bool HasPower = true;

	if (BuildingClass * Building = specific_cast<BuildingClass *>(Victim)) {
		HasPower = HasPower && Building->IsPowerOnline();

		if (!Building->Type->InvisibleInGame) {
			if (HasPower || Building->Type->LaserFencePost) {
				Building->EnableStuff();
			}
			updateRadarBlackout(Building);

			BuildingTypeClass * pType = Building->Type;
			if (pType->Factory != AbstractType::None) {
				Building->Owner->Update_FactoriesQueues(pType->Factory, pType->Naval, BuildCat::DontCare);
			}
		}
	}

	Victim->Owner->RecheckTechTree = true;
	Victim->Owner->RecheckPower = true;

	if (Victim->Deactivated && HasPower) {
		Victim->Reactivate();
	}

	// allow to spawn units again.
	updateSpawnManager(Victim);
	updateSlaveManager(Victim);

	// update the animation
	UpdateSparkleAnim(Victim);

	// get harvesters back to work and ai units to hunt
	if (FootClass * Foot = generic_cast<FootClass *>(Victim)) {
		bool hasMission = false;
		if (UnitClass * Unit = specific_cast<UnitClass *>(Victim)) {
			if (Unit->Type->Harvester || Unit->Type->ResourceGatherer) {
				// prevent unloading harvesters from being irritated.
				if (pData->EMPLastMission == Mission::Guard) {
					pData->EMPLastMission = Mission::Enter;
				}

				Unit->QueueMission(pData->EMPLastMission, true);
				hasMission = true;
			}
		}

		if(!hasMission && !Foot->Owner->ControlledByHuman()) {
			Foot->QueueMission(Mission::Hunt, false);
		}
	}
}

// the functions below are not related to EMP. they aren't official
// and certainly don't endorse you to use them. 2011-05-14 AlexB

bool EMPulse::EnableEMPEffect2(TechnoClass * Victim) {
	Victim->Owner->RecheckTechTree = true;
	Victim->Owner->RecheckPower = true;

	if (BuildingClass * Building = specific_cast<BuildingClass *>(Victim)) {
		Building->DisableStuff();
		updateRadarBlackout(Building);

		BuildingTypeClass * pType = Building->Type;
		if (pType->Factory != AbstractType::None) {
			Building->Owner->Update_FactoriesQueues(pType->Factory, pType->Naval, BuildCat::DontCare);
		}
	} else {
		if (AircraftClass * Aircraft = specific_cast<AircraftClass *>(Victim)) {
			// crash flying aircraft
			if (Aircraft->GetHeight() > 0) {
				// this would a) happen every time it updates and b) possibly
				// crash because it frees the caller's memory (the PoweredUnitClass)
				// while it is executing.
				//TechnoExt::Destroy(Aircraft);
				return true;
			}
		}
	}

	// deactivate and sparkle
	if (!Victim->Deactivated && IsDeactivationAdvisable(Victim)) {
		// cache the last mission this thing did
		TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(Victim);
		pData->EMPLastMission = Victim->CurrentMission;

		// remove the unit from its team
		if (FootClass * Foot = generic_cast<FootClass *>(Victim)) {
			if (Foot->BelongsToATeam()) {
				Foot->Team->LiberateMember(Foot);
			}
		}

		bool selected = Victim->IsSelected;
		Victim->Deactivate();
		if(selected) {
			bool feedback = Unsorted::MoveFeedback;
			Unsorted::MoveFeedback = false;
			Victim->Select();
			Unsorted::MoveFeedback = feedback;
		}

		if(generic_cast<FootClass *>(Victim)) {
			Victim->QueueMission(Mission::Sleep, true);
		}

		// release all captured units.
		if (Victim->CaptureManager) {
			Victim->CaptureManager->FreeAll();
		}

		// update managers.
		updateSpawnManager(Victim, nullptr);
		updateSlaveManager(Victim);

		// set the sparkle animation.
		UpdateSparkleAnim(Victim);
	}

	// the unit still lives.
	return false;
}

void EMPulse::DisableEMPEffect2(TechnoClass * Victim) {
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(Victim);
	bool HasPower = pData->IsPowered() && pData->IsOperated();

	if (BuildingClass * Building = specific_cast<BuildingClass *>(Victim)) {
		HasPower = HasPower && Building->IsPowerOnline(); //Building->HasPower && !(Building->Owner->PowerDrain > Building->Owner->PowerOutput) ;

		if (!Building->Type->InvisibleInGame) {
			if (HasPower) {
				Building->EnableStuff();
			}
			updateRadarBlackout(Building);

			BuildingTypeClass * pType = Building->Type;
			if (pType->Factory != AbstractType::None) {
				Building->Owner->Update_FactoriesQueues(pType->Factory, pType->Naval, BuildCat::DontCare);
			}
		}
	}

	Victim->Owner->RecheckTechTree = true;
	Victim->Owner->RecheckPower = true;

	if (Victim->Deactivated && HasPower) {
		Victim->Reactivate();

		// allow to spawn units again.
		updateSpawnManager(Victim);
		updateSlaveManager(Victim);

		// update the animation
		UpdateSparkleAnim(Victim);

		// get harvesters back to work and ai units to hunt
		if (FootClass * Foot = generic_cast<FootClass *>(Victim)) {
			bool hasMission = false;
			if (UnitClass * Unit = specific_cast<UnitClass *>(Victim)) {
				if (Unit->Type->Harvester || Unit->Type->ResourceGatherer) {
					// prevent unloading harvesters from being irritated.
					if (pData->EMPLastMission == Mission::Guard) {
						pData->EMPLastMission = Mission::Enter;
					}

					Unit->QueueMission(pData->EMPLastMission, true);
					hasMission = true;
				}
			}

			if(!hasMission && !Foot->Owner->ControlledByHuman()) {
				Foot->QueueMission(Mission::Hunt, false);
			}
		}
	}
}
