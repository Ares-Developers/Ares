#include "EMPulse.h"
#include "../Ext/Techno/Body.h"
#include "../Ext/TechnoType/Body.h"
#include "../Ext/BuildingType/Body.h"
#include "../Utilities/Helpers.Alex.h"
#include <set>

//! Enables verbose debug output for some WarheadTypeExt functions.
bool EMPulse::verbose = false;
bool EMPulse::supportVerses = false;

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
void EMPulse::CreateEMPulse(WarheadTypeExt::ExtData *Warhead, CoordStruct *Coords, TechnoClass *Firer) {
	if (!Warhead) {
		Debug::DevLog(Debug::Error, "Trying to CreateEMPulse() with Warhead pointing to NULL. Funny.\n");
		return;
	}

	if (verbose) {
		Debug::Log("[CreateEMPulse] Duration: %d, Cap: %d\n", Warhead->EMP_Duration, Warhead->EMP_Cap);
	}

	// set of affected objects. every object can be here only once.
	DynamicVectorClass<TechnoClass*> *items = Helpers::Alex::getCellSpreadItems(Coords,
		Warhead->AttachedToObject->CellSpread, true);

	// affect each object
	for(int i=0; i<items->Count; ++i) {
		deliverEMPDamage(items->GetItem(i), Firer, Warhead);
	}

	// tidy up
	items->Clear();
	delete items;

	if (verbose) {
		Debug::Log("[CreateEMPulse] Done.\n");
	}
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
	HouseClass *pHouse = (Firer ? Firer->Owner : NULL);

	if (TechnoClass * curTechno = generic_cast<TechnoClass *> (object)) {
		if (verbose) {
			Debug::Log("[deliverEMPDamage] Step 1: %s => %s\n",
					(Firer ? Firer->get_ID() : NULL),
					curTechno->get_ID());
		}

		if (isEligibleEMPTarget(curTechno, pHouse, Warhead->AttachedToObject)) {
			if (verbose) {
				Debug::Log("[deliverEMPDamage] Step 2: %s\n",
						curTechno->get_ID());
			}

			// get the target-specific multiplier
			float modifier = 1.0F;
			if(TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(curTechno->GetTechnoType())) {
				// modifier only affects bad things
				if(Warhead->EMP_Duration > 0) {
					modifier = pExt->EMP_Modifier;
				}
			}

			// respect verses
			int duration = (int)(Warhead->EMP_Duration * modifier);
			if(supportVerses) {
				duration = (int)(duration * Warhead->Verses[curTechno->GetTechnoType()->Armor].Verses);
			} else if(abs(Warhead->Verses[curTechno->GetTechnoType()->Armor].Verses) < 0.001) {
				return;
			}

			// get the new capped value
			int oldValue = curTechno->EMPLockRemaining;
			int newValue = Helpers::Alex::getCappedDuration(oldValue, duration, Warhead->EMP_Cap);

			if (verbose) {
				Debug::Log("[deliverEMPDamage] Step 3: %d\n",
						newValue);
			}

			// can not be less than zero
			curTechno->EMPLockRemaining = std::max(0, newValue);
			if (verbose) {
				Debug::Log("[deliverEMPDamage] Step 4: %d\n",
						newValue);
			}

			// newly de-paralyzed
			if ((oldValue > 0) && (curTechno->EMPLockRemaining <= 0)) {
				if (verbose) {
					Debug::Log("[deliverEMPDamage] Step 5a\n");
				}
				DisableEMPEffect(curTechno);
			} else if ((oldValue <= 0) && (curTechno->EMPLockRemaining > 0)) {
				// newly paralyzed unit
				if (verbose) {
					Debug::Log("[deliverEMPDamage] Step 5b\n");
				}
				if (enableEMPEffect(curTechno, Firer)) {
					return;
				}
			} else if (oldValue != newValue) {
				// At least update the radar, if this is one.
				if (verbose) {
					Debug::Log("[deliverEMPDamage] Step 5c\n");
				}
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
	\date 2010-04-27
*/
bool EMPulse::isEMPTypeImmune(TechnoClass * Target) {
	// find an emp weapon.
	TechnoTypeClass *TT = Target->GetTechnoType();
	if (TT->TypeImmune) {
		for (int i = 0; i < 18; ++i) {
			WeaponTypeClass *WeaponType = (!Target->Veterancy.IsElite())
				? TT->get_Weapon(i)
				: TT->get_EliteWeapon(i);
			if (!WeaponType) {
				continue;
			}

			WarheadTypeClass *WarheadType = WeaponType->Warhead;
			if (WarheadTypeExt::ExtData *pWH = WarheadTypeExt::ExtMap.Find(WarheadType)) {
				if (pWH->EMP_Duration != 0) {
					// this unit can fire emps and type immunity
					// grants it to never be affected.
					return true;
				}
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
	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(Target->GetTechnoType());

	if (pData->ImmuneToEMP.Get()) {
		if (verbose) {
			Debug::Log("[isEMPImmune] \"%s\" is ImmuneToEMP.\n", Target->get_ID());
		}
		return true;
	}

	// this may be immune because of veteran and elite abilities.
	if (Target->Veterancy.IsElite() && (pData->EliteAbilityEMPIMMUNE || pData->VeteranAbilityEMPIMMUNE)) {
		if (verbose) {
			Debug::Log("[isEMPImmune] \"%s\" is immune because it is elite.\n", Target->get_ID());
		}
		return true;
	} else if (Target->Veterancy.IsVeteran() && pData->VeteranAbilityEMPIMMUNE) {
		if (verbose) {
			Debug::Log("[isEMPImmune] \"%s\" is immune because it is veteran.\n", Target->get_ID());
		}
		return true;
	}

	// if houses differ, TypeImmune does not count.
	if (Target->Owner == SourceHouse) {
		// ignore if type immune. don't even try.
		if (isEMPTypeImmune(Target)) {
			// This unit can fire emps and type immunity
			// grants it to never be affected.
			if (verbose) {
				Debug::Log("[isEMPImmune] \"%s\" is TypeImmune.\n", Target->get_ID());
			}
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

	if(Target->WhatAmI() == abs_Unit) {
		if(BuildingClass* pBld = MapClass::Instance->GetCellAt(&Target->Location)->GetBuilding()) {
			if(pBld->Type->WeaponsFactory) {
				if(pBld->IsUnderEMP() || pBld == Target->GetNthLink(0)) {
					if (EMPulse::verbose) {
						Debug::Log("[isCurrentlyEMPImmune] %s should not be disabled. Still in war factory: %s\n", Target->get_ID(), pBld->get_ID());
					}
					return true;
				}

				// units requiring an operator can't deactivate on the bib
				// because nobody could enter it afterwards.
				TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(Target);
				if(!pData->IsOperated()) {
					if (EMPulse::verbose) {
						Debug::Log("[isCurrentlyEMPImmune] %s should not be disabled. Would be unoperated on bib: %s\n", Target->get_ID(), pBld->get_ID());
					}
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
	\date 2010-04-30
*/
bool EMPulse::IsTypeEMPProne(TechnoTypeClass * Type) {
	bool prone = true;

	// buildings are emp prone if they consume power and need it to function
	if (BuildingTypeClass * BuildingType = specific_cast<BuildingTypeClass *>(Type)) {
		prone = (BuildingType->Powered && (BuildingType->PowerDrain > 0));

		// may have a special function.
		if (BuildingType->Radar ||
			(BuildingType->SuperWeapon> -1) || (BuildingType->SuperWeapon2 > -1)
			|| BuildingType->UndeploysInto
			|| BuildingType->PowersUnit
			|| BuildingType->Sensors
			|| BuildingType->LaserFencePost
			|| BuildingType->GapGenerator) {
				prone = true;
		}
	} else if (InfantryTypeClass * InfantryType = specific_cast<InfantryTypeClass *>(Type)) {
		// affected only if this is a cyborg.
		prone = InfantryType->Cyborg_;
	} else {
		// if this is a vessel or vehicle that is organic: no effect.
		prone = !Type->Organic;
	}

	return prone;
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

	if (!WarheadTypeExt::canWarheadAffectTarget(Target, SourceHouse, Warhead)) {
		if (verbose) {
			Debug::Log("[isEligibleEMPTarget] \"%s\" does not affect target.\n", Warhead->ID);
		}
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
bool EMPulse::IsDeactivationAdvisable(TechnoClass * Target) {
	switch(Target->CurrentMission)
	{
	case mission_Selling:
	case mission_Construction:
	case mission_Unload:
		if (EMPulse::verbose) {
			Debug::Log("[IsDeactivationAdvisable] %s should not be disabled. Mission: %d\n", Target->get_ID(), Target->CurrentMission);
		}
		return false;
	}

	if (EMPulse::verbose) {
		Debug::Log("[IsDeactivationAdvisable] %s should be disabled. Mission: %d\n", Target->get_ID(), Target->CurrentMission);
	}
	return true;
}

//! Updates the radar outage for the owning player.
/*!
	If this is a structure providing radar or spy satellite abilities the
	original check for radar facilities is invoked by setting a flag.

	\param Techno The Techno that might be a Radar or SpySat structure.

	\author AlexB
	\date 2010-11-28
*/
void EMPulse::updateRadarBlackout(TechnoClass * Techno) {
	if (BuildingClass * Building = specific_cast<BuildingClass *>(Techno)) {
		if (!Building->Type->InvisibleInGame) {
			if (Building->Type->Radar || Building->Type->SpySat) {
				Building->Owner->RadarBlackout = true;
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

	\author AlexB
	\date 2010-05-03
*/
void EMPulse::updateSpawnManager(TechnoClass * Techno, ObjectClass * Source = NULL) {
	if (SpawnManagerClass *SM = Techno->SpawnManager) {

		if (Techno->EMPLockRemaining > 0) {
			// crash all spawned units that are visible. else, they'd land somewhere else.
			for (int i=0; i < SM->SpawnedNodes.Count; ++i) {
				SpawnNode *spawn = SM->SpawnedNodes.GetItem(i);
				// kill every spawned unit that is in the air. exempt missiles.
				if(!spawn->IsSpawnMissile && spawn->Unit && spawn->Status >= 1 && spawn->Status <= 4) {
					TechnoExt::Destroy(spawn->Unit, generic_cast<TechnoClass*>(Source));
				}
			}

			// pause the timers so spawning and regenerating is deferred.
			SM->SpawnTimer.StartTime = -1;
			SM->UnknownTimer.StartTime = -1;
		} else {
			// resume counting.
			SM->SpawnTimer.StartIfEmpty();
			SM->UnknownTimer.StartIfEmpty();
		}
	}
}

//! Updates the SlaveManager to account for the EMP effect.
/*!
	Stops the slaves where they are standing until the EMP effect is over.

	\param Techno The Techno that might be an enslaver.

	\author AlexB
	\date 2010-05-07
*/
void EMPulse::updateSlaveManager(TechnoClass * Techno) {
	if (SlaveManagerClass *SM = Techno->SlaveManager) {
		if (Techno->EMPLockRemaining > 0) {
			// pause the timers so spawning and regenerating is deferred.
			SM->SuspendWork();
		} else {
			// resume slaving around.
			SM->ResumeWork();
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
void EMPulse::UpdateSparkleAnim(TechnoClass * Techno) {
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(Techno);
	if (pData) {
		if (Techno->IsUnderEMP()) {
			if (!pData->EMPSparkleAnim && RulesClass::Instance->EMPulseSparkles) {
				GAME_ALLOC(AnimClass, pData->EMPSparkleAnim, RulesClass::Instance->EMPulseSparkles, &Techno->Location);
				pData->EMPSparkleAnim->SetOwnerObject(Techno);
			}
		} else {
			if (pData->EMPSparkleAnim) {
				pData->EMPSparkleAnim->RemainingIterations = 0; // basically "you don't need to show up anymore"
				pData->EMPSparkleAnim = NULL;
			}
		}
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
void EMPulse::announceAttack(TechnoClass * Techno) {
	enum AttackEvents {None = 0, Base = 1, Harvester = 2};
	AttackEvents rEvent = None;

	// find out what event is the most appropriate.
	if (Techno && (Techno->Owner == HouseClass::Player)) {
		if (BuildingClass * Building = specific_cast<BuildingClass *>(Techno)) {
			if(Building->Type->ResourceGatherer) {
				// slave miner, for example
				rEvent = Harvester;
			} else if(!Building->Type->Insignificant && !Building->Type->BaseNormal) {
				rEvent = Base;
			}
		} else if (UnitClass * Unit = specific_cast<UnitClass *>(Techno)) {
			if (Unit->Type->Harvester || Unit->Type->ResourceGatherer) {
				rEvent = Harvester;
			}
		}
	}

	// handle the event.
	if (rEvent != None) {
		CellStruct xy;
		Techno->GetMapCoords(&xy);

		switch (rEvent) {
			case Harvester:
				if (RadarEventClass::Create(RADAREVENT_OREMINERUNDERATTACK, xy))
					VoxClass::Play("EVA_OreMinerUnderAttack", -1, -1);
				break;
			case Base:
				HouseClass::Player->BuildingUnderAttack(specific_cast<BuildingClass *>(Techno));
				break;
		}
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
	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(Victim->GetTechnoType());

	if (verbose) {
		Debug::Log("[thresholdExceeded] %s: %d %d\n", Victim->get_ID(), pData->EMP_Threshold, Victim->EMPLockRemaining);
	}

	if ((pData->EMP_Threshold != 0) && (Victim->EMPLockRemaining > (DWORD)abs(pData->EMP_Threshold))) {
		if ((pData->EMP_Threshold > 0) || (Victim->IsInAir() && !Victim->HasParachute)) {
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
	Victim->Owner->ShouldRecheckTechTree = true;
	Victim->Owner->PowerBlackout = true;

	if (BuildingClass * Building = specific_cast<BuildingClass *>(Victim)) {
		Building->DisableStuff();
		updateRadarBlackout(Building);

		BuildingTypeClass * pType = Building->Type;
		if (pType->Factory) {
			Building->Owner->Update_FactoriesQueues(pType->Factory, pType->Naval, 0);
		}
	} else {
		if (AircraftClass * Aircraft = specific_cast<AircraftClass *>(Victim)) {
			// crash flying aircraft
			if (Aircraft->GetHeight() > 0) {
				if (EMPulse::verbose) {
					Debug::Log("[enableEMPEffect] Plane crash: %s\n", Aircraft->get_ID());
				}
				TechnoExt::Destroy(Victim, generic_cast<TechnoClass*>(Source));
				return true;
			}
		}
	}

	// cache the last mission this thing did
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(Victim);
	pData->EMPLastMission = Victim->CurrentMission;
	
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
			if (HasPower) {
				Building->EnableStuff();
			}
			updateRadarBlackout(Building);

			BuildingTypeClass * pType = Building->Type;
			if (pType->Factory) {
				Building->Owner->Update_FactoriesQueues(pType->Factory, pType->Naval, 0);
			}
		}
	}

	Victim->Owner->ShouldRecheckTechTree = true;
	Victim->Owner->PowerBlackout = true;

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
				if (pData->EMPLastMission == mission_Guard) {
					pData->EMPLastMission = mission_Enter;
				}

				Unit->QueueMission(pData->EMPLastMission, true);
				hasMission = true;
			}
		}

		if(!hasMission && !Foot->Owner->ControlledByHuman()) {
			Foot->QueueMission(mission_Hunt, false);
		}
	}
}

// the functions below are not related to EMP. they aren't official
// and certainly don't endorse you to use them. 2011-05-14 AlexB

bool EMPulse::EnableEMPEffect2(TechnoClass * Victim) {
	Victim->Owner->ShouldRecheckTechTree = true;
	Victim->Owner->PowerBlackout = true;

	if (BuildingClass * Building = specific_cast<BuildingClass *>(Victim)) {
		Building->DisableStuff();
		updateRadarBlackout(Building);

		BuildingTypeClass * pType = Building->Type;
		if (pType->Factory) {
			Building->Owner->Update_FactoriesQueues(pType->Factory, pType->Naval, 0);
		}
	} else {
		if (AircraftClass * Aircraft = specific_cast<AircraftClass *>(Victim)) {
			// crash flying aircraft
			if (Aircraft->IsInAir()) {
				if (EMPulse::verbose) {
					Debug::Log("[EnableEMPEffect2] Plane crash: %s\n", Aircraft->get_ID());
				}
				if (Victim->Owner == HouseClass::Player) {
					VocClass::PlayAt(Aircraft->Type->VoiceCrashing, &Aircraft->Location, NULL);
				}
				Aircraft->Crash(NULL);
				Aircraft->Destroyed(NULL);
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
			Victim->QueueMission(mission_Sleep, true);
		}

		// release all captured units.
		if (Victim->CaptureManager) {
			Victim->CaptureManager->FreeAll();
		}

		// update managers.
		updateSpawnManager(Victim, NULL);
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
			if (pType->Factory) {
				Building->Owner->Update_FactoriesQueues(pType->Factory, pType->Naval, 0);
			}
		}
	}

	Victim->Owner->ShouldRecheckTechTree = true;
	Victim->Owner->PowerBlackout = true;

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
					if (pData->EMPLastMission == mission_Guard) {
						pData->EMPLastMission = mission_Enter;
					}

					Unit->QueueMission(pData->EMPLastMission, true);
					hasMission = true;
				}
			}

			if(!hasMission && !Foot->Owner->ControlledByHuman()) {
				Foot->QueueMission(mission_Hunt, false);
			}
		}
	}
}
