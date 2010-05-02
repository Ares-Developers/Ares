#include "EMPulse.h"
#include "../Ext/Techno/Body.h"
#include "../Ext/TechnoType/Body.h"

//! Enables verbose debug output for some WarheadTypeExt functions.
bool EMPulse::verbose = true;

//! Paralyses all units using an EMP cellspread weapon.
/*!
	Obsolete. Use CreateEMPulse(WarheadTypeExt::ExtData *, CellStruct, TechnoClass *)
	instead.

	All Technos in the EMPulse's target cells get affected by an EMP and get
	deactivated temporarily. Their special functions stop working until the
	EMP ceases. Flying Aircraft crashes.

	\param EMPulse The electromagnetic pulse to create.
	\param Firer The Techno that fired the pulse.

	\author AlexB
	\date 2010-04-27
*/
void EMPulse::CreateEMPulse(EMPulseClass *Legacy, TechnoClass *Firer) {
	WarheadTypeExt::ExtData *Warhead = WarheadTypeExt::ExtMap.Find(WarheadTypeExt::EMP_WH);
	CreateEMPulse(Warhead, Legacy->BaseCoords, Firer);
}

//! Paralyses all units using an EMP cellspread weapon.
/*!
	All Technos in the EMPulse's target cells get affected by an EMP and get
	deactivated temporarily. Their special functions stop working until the
	EMP ceases. Flying Aircraft crashes.

	\param EMPulse The electromagnetic pulse to create.
	\param Firer The Techno that fired the pulse.

	\author AlexB
	\date 2010-04-27
*/
void EMPulse::CreateEMPulse(WarheadTypeExt::ExtData * Warhead, CellStruct Coords, TechnoClass *Firer) {
	if (!Warhead) {
		Debug::DevLog(Debug::Error, "Trying to CreateEMPulse() with Warhead pointing to NULL. Funny.\n");
		return;
	}

	// fill the gaps
	HouseClass *pHouse = (Firer ? Firer->Owner : NULL);

	if (verbose)
		Debug::Log("[CreateEMPulse] Duration: %d, Cap: %d\n", Warhead->EMP_Duration, Warhead->EMP_Cap);

	CellStruct cellCoords = MapClass::Instance->GetCellAt(&Coords)->MapCoords;
	int countCells = CellSpread::NumCells((int)(Warhead->AttachedToObject->CellSpread + 0.99));
	for (int i = 0; i < countCells; ++i) {
		CellStruct tmpCell = CellSpread::GetCell(i);
		tmpCell += cellCoords;
		CellClass *c = MapClass::Instance->GetCellAt(&tmpCell);
		for (ObjectClass *curObj = c->GetContent(); curObj; curObj = curObj->NextObject) {
			if (TechnoClass * curTechno = generic_cast<TechnoClass *> (curObj)) {
				if (verbose)
					Debug::Log("[CreateEMPulse] Step 1: %s => %s\n",
							(Firer ? Firer->get_ID() : NULL),
							curTechno->get_ID());

				if (isEligibleEMPTarget(curTechno, pHouse, Warhead->AttachedToObject)) {
					if (verbose)
						Debug::Log("[CreateEMPulse] Step 2: %s\n",
								curTechno->get_ID());
					// get the new capped value
					int oldValue = curTechno->EMPLockRemaining;
					int newValue = getCappedDuration(oldValue, Warhead->EMP_Duration, Warhead->EMP_Cap);

					if (verbose)
						Debug::Log("[CreateEMPulse] Step 3: %d\n",
								newValue);

					// can not be less than zero
					curTechno->EMPLockRemaining = max(0, newValue);
					if (verbose)
						Debug::Log("[CreateEMPulse] Step 4: %d\n",
								newValue);

					// newly de-paralyzed
					if ((oldValue > 0) && (curTechno->EMPLockRemaining <= 0)) {
						if (verbose)
							Debug::Log("[CreateEMPulse] Step 5a\n");
						DisableEMPEffect(curTechno);
					} else if ((oldValue <= 0) && (curTechno->EMPLockRemaining > 0)) {
						// newly paralyzed unit
						if (verbose)
							Debug::Log("[CreateEMPulse] Step 5b\n");
						if (enableEMPEffect(curTechno, Firer)) {
							continue;
						}
					} else if (oldValue != newValue) {
						// At least update the radar, if this is one.
						updateRadarBlackout(curTechno);
						if (verbose)
							Debug::Log("[CreateEMPulse] Step 5c\n");
					}
				}
			}
		}
	}
}

//! Gets whether a Techno is type immune to EMP.
/*!
	Type immunity does work a little different for EMP weapons than for ordinary
	projectiles. Target is type immune to EMP if it has a weapon that uses a
	warhead also having EMEffect set. It is irrelevant which Techno fired the EMP.

	If Target is elite, only EliteWeapons are used to check for EMEffect,
	otherwise only ordinary Weapons are used.

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
			if (WarheadType && WarheadType->EMEffect) {
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
	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(Target->GetTechnoType());

	// There is no other way to use (for example) BuildingTypeClass members from
	// TechnoTypeClass Initialize to check if this Techno is EMP-prone. Cache the result
	// here.
	if(!pData->ImmuneToEMPSet.Get()) {
		pData->ImmuneToEMP.Set(!EMPulse::IsTypeEMPProne(Target->GetTechnoType()));
		pData->ImmuneToEMPSet.Set(true);
	}

	if (pData->ImmuneToEMP.Get()) {
		if (verbose)
			Debug::Log("[isEMPImmune] \"%s\" is ImmuneToEMP.\n", Target->get_ID());
		return true;
	}

	// this may be immune because of veteran and elite abilities.
	if (Target->Veterancy.IsElite() && (pData->EliteAbilityEMPIMMUNE || pData->VeteranAbilityEMPIMMUNE)) {
		if (verbose)
			Debug::Log("[isEMPImmune] \"%s\" is immune because it is elite.\n", Target->get_ID());
		return true;
	} else if (Target->Veterancy.IsVeteran() && pData->VeteranAbilityEMPIMMUNE) {
		if (verbose)
			Debug::Log("[isEMPImmune] \"%s\" is immune because it is veteran.\n", Target->get_ID());
		return true;
	}

	// if houses differ, TypeImmune does not count.
	if (Target->Owner != SourceHouse) {
		// ignore if type immune. don't even try.
		if (isEMPTypeImmune(Target)) {
			// This unit can fire emps and type immunity
			// grants it to never be affected.
			if (verbose)
				Debug::Log("[isEMPImmune] \"%s\" is TypeImmune.\n", Target->get_ID());
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
	the Iron Curtain or still being constructed or sold.

	\param Target The Techno the EMP is fired at.
	\param SourceHouse The house that fired the EMP.

	\returns True if Target is immune to EMP, false otherwise.

	\author AlexB
	\date 2010-05-02
*/
bool EMPulse::isCurrentlyEMPImmune(TechnoClass * Target, HouseClass * SourceHouse) {
	// iron curtained objects can not be affected by EMPs
	if (Target->IsIronCurtained()) {
		return true;
	}

	// technos still being constructed/sold would display the buildup anim
	// over and over again.
	if ((Target->CurrentMission == mission_Construction) || (Target->CurrentMission == mission_Selling)) {
		return true;
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
	if (isCurrentlyEMPImmune(Target, SourceHouse))
		return false;

	if (!WarheadTypeExt::canWarheadAffectTarget(Target, SourceHouse, Warhead)) {
		if (verbose)
			Debug::Log("[isEligibleEMPTarget] \"%s\" does not affect target.\n", Warhead->ID);
		return false;
	}

	return true;
}

//! Gets the new duration the EMP effect will last.
/*!
	The new EMP lock frames count is calculated the following way:

	If Duration is positive it will inflict EMP damage. If Cap is larger than zero,
	the maximum amount of frames will be defined by Cap. If the current value
	already is larger than that, in will not be reduced. If Cap is zero, then
	the duration can add up infinitely. If Cap is less than zero, duration will
	be set to Duration, if the current value is not higher already.

	If Duration is negative, the EMP effect will be reduced. A negative Cap
	reduces the current value by Duration. A positive or zero Cap will do the
	same, but additionally shorten it to Cap if the result would be higher than
	that. Thus, a Cap of zero removes the current EMP effect altogether.

	\param CurrentValue The Technos current remaining time.
	\param Duration The duration the EMP uses.
	\param Cap The maximum Duration this EMP can cause.

	\returns The new EMP lock frames count.

	\author AlexB
	\date 2010-04-27
*/
int EMPulse::getCappedDuration(int CurrentValue, int Duration, int Cap) {
	// Usually, the new duration is just added.
	int ProposedDuration = CurrentValue + Duration;

	if (Duration > 0) {
		// Positive damage.
		if (Cap < 0) {
			// Do not stack. Use the maximum value.
			return max(Duration, CurrentValue);
		} else if (Cap > 0) {
			// Cap the duration.
			int cappedValue = min(ProposedDuration, Cap);
			return max(CurrentValue, cappedValue);
		} else {
			// There is no cap. Allow the duration to stack up.
			return ProposedDuration;
		}
	} else {
		// Negative damage.
		return (Cap < 0 ? ProposedDuration : min(ProposedDuration, Cap));
	}
}

//! Updates the radar outage for the owning player to match the current EMP effect duration.
/*!
	The radar outage duration will be set to the number of frames the current
	EMP effect lasts. If there is no EMP in effect, the radar is set to come
	back online.

	\param Techno The Techno that might be a Radar structure.

	\author AlexB
	\date 2010-04-27
*/
void EMPulse::updateRadarBlackout(TechnoClass * Techno) {
	if (BuildingClass * Building = specific_cast<BuildingClass *>(Techno)) {
		if (!Building->Type->InvisibleInGame) {
			if (Building->Type->Radar) {
				if (verbose)
					Debug::Log("[updateRadarBlackout] Before: %d, %d\n",
							Building->EMPLockRemaining,
							Building->Owner->RadarBlackoutTimer.TimeLeft);
				if (Building->EMPLockRemaining > 0) {
					Building->Owner->CreateRadarOutage(Building->EMPLockRemaining);
					if (verbose)
						Debug::Log("[enableEMPEffect] Radar down for: %d\n", Building->EMPLockRemaining);
				} else {
					Building->Owner->RadarBlackoutTimer.TimeLeft = 0;
					Building->Owner->RadarBlackout = true; // trigger the radar outage check
				}
				if (verbose)
					Debug::Log("[updateRadarBlackout] After: %d\n",
							Building->Owner->RadarBlackoutTimer.TimeLeft);
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

//! Sets all properties to disable a Techno.
/*!
	Disables Buildings and crashes flying Aircrafts. Foots get deactivated.
	An EMP sparkle animation is created and played until the EMP effect
	ceases.

	Contains special handling to create a radar outage equalling the length
	of the EMP effect, if Victim is a Building with Radar capabilities.

	If Victim mind controls any units, they are freed. Spawned units are
	killed. Draining is stopped.

	\param Victim The Techno that is under EMP effect.
	\param Source The house to credit kills to.

	\returns True if Victim has been destroyed by the EMP effect, False otherwise.

	\author AlexB
	\date 2010-04-28
*/
bool EMPulse::enableEMPEffect(TechnoClass * Victim, ObjectClass * Souce) {
	Victim->Owner->ShouldRecheckTechTree = true;
	Victim->Owner->PowerBlackout = true;

	if (BuildingClass * Building = specific_cast<BuildingClass *>(Victim)) {
		Building->DisableStuff();
		updateRadarBlackout(Building);
	} else {
		if (AircraftClass * Aircraft = specific_cast<AircraftClass *>(Victim)) {
			// crash flying aircraft
			if (Aircraft->InAir) {
				if (EMPulse::verbose)
					Debug::Log("[enableEMPEffect] Plane crash: %s\n", Aircraft->get_ID());
				Aircraft->Crash(Souce);
				return true;
			}
		}
	}

	// cache the last mission this thing did
	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(Victim);
	pData->EMPLastMission = Victim->CurrentMission;

	// deactivate and sparkle.
	if (!Victim->Deactivated)
		Victim->Deactivate();

	// release all captured units.
	if (Victim->CaptureManager)
		Victim->CaptureManager->FreeAll();

	// crash all spawned units.
	if (Victim->SpawnManager) {
		Victim->SpawnManager->KillNodes();
	}

	//// stop draining.
	//TechnoExt::StopDraining(Victim->DrainingMe, NULL);
	//TechnoExt::StopDraining(NULL, Victim->DrainTarget);

	// set the sparkle animation.
	if (!pData->EMPSparkleAnim && RulesClass::Instance->EMPulseSparkles) {
		GAME_ALLOC(AnimClass, pData->EMPSparkleAnim, RulesClass::Instance->EMPulseSparkles, &Victim->Location);
		pData->EMPSparkleAnim->SetOwnerObject(Victim);
	}

	// warn the player
	announceAttack(Victim);

	// the unit still lives.
	return false;
}

//! Sets all properties to re-enable a Techno.
/*!
	Reactivates the Techno. The EMP sparkle animation is stopped.

	Radars come back online.

	If Victim mind controls any units, they are freed. Spawned units are
	killed. Draining is stopped.

	\param Victim The Techno that shall have its EMP effects removed.

	\author AlexB
	\date 2010-04-28
*/
void EMPulse::DisableEMPEffect(TechnoClass * Victim) {
	if (BuildingClass * Building = specific_cast<BuildingClass *>(Victim)) {
		if (!Building->Type->InvisibleInGame) {
			Building->EnableStuff();
			updateRadarBlackout(Building);
		}
	}

	Victim->Owner->ShouldRecheckTechTree = true;
	Victim->Owner->PowerBlackout = true;

	if (Victim->Deactivated)
		Victim->Reactivate();

	TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(Victim);
	if (pData && pData->EMPSparkleAnim) {
		pData->EMPSparkleAnim->RemainingIterations = 0; // basically "you don't need to show up anymore"
		pData->EMPSparkleAnim = NULL;
	}

	// get harvesters back to work.
	if (UnitClass * Unit = specific_cast<UnitClass *>(Victim)) {
		if (Unit->Type->Harvester || Unit->Type->ResourceGatherer)
			Unit->QueueMission(pData->EMPLastMission, true);
	}
}