#include "Body.h"
#include "../Techno/Body.h"
#include "../TechnoType/Body.h"

bool WarheadTypeExt::verbose = true;

bool WarheadTypeExt::isEMPTypeImmune(TechnoClass * Unit, HouseClass * SourceHouse) {
	// if houses differ, TypeImmune does not count.
	if (Unit->Owner == SourceHouse) {
		return false;
	}

	// find an emp weapon.
	TechnoTypeClass *TT = Unit->GetTechnoType();
	if (TT->TypeImmune) {
		for (int i = 0; i < 18; ++i) {
			WeaponTypeClass *WeaponType = (!Unit->Veterancy.IsElite())
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

bool WarheadTypeExt::isEMPImmune(TechnoClass * Unit, HouseClass * SourceHouse) {
	// iron curtained objects can not be emp'd
	if (Unit->IsIronCurtained()) {
		return true;
	}

	// this can be overridden by a flag on the techno.
	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(Unit->GetTechnoType());
	if (pData->ImmuneToEMP) {
		if (verbose)
			Debug::Log("[isEMPImmune] \"%s\" is ImmuneToEMP.\n", Unit->get_ID());
		return true;
	}

	// ignore if type immune. don't even try.
	if (isEMPTypeImmune(Unit, SourceHouse)) {
		// This unit can fire emps and type immunity
		// grants it to never be affected.
		if (verbose)
			Debug::Log("[isEMPImmune] \"%s\" is TypeImmune.\n", Unit->get_ID());
		return true;
	}

	return false;
}

bool WarheadTypeExt::canAffectTarget(TechnoClass * Target, HouseClass * SourceHouse, WarheadTypeClass *WH) {
	if (SourceHouse && Target && WH) {
		// owner and target house are allied and this warhead
		// is set to not hurt any allies.
		bool alliedWithTarget = SourceHouse->IsAlliedWith(Target->Owner);
		if (alliedWithTarget && !WarheadTypeExt::EMP_WH->AffectsAllies) {
			if (verbose)
				Debug::Log("[isEMPImmune] \"%s\" does not AffectAllies.\n", WH->ID);
			return false;
		}

		// this warhead's pulse is designed to fly around
		// enemy units. useful for healing.
		WarheadTypeExt::ExtData *pWHdata = WarheadTypeExt::ExtMap.Find(WH);
		if (!alliedWithTarget && !pWHdata->AffectsEnemies) {
			if (verbose)
				Debug::Log("[isEMPImmune] \"%s\" does not AffectEnemies.\n", WH->ID);
			return false;
		}
	}

	return true;
}

bool WarheadTypeExt::canBeEMPAffected(TechnoClass * Unit, HouseClass * SourceHouse) {
	// check whether this techno can be affected at all
	bool isEMPProne = !isEMPImmune(Unit, SourceHouse);
					  //&& canAffectTarget(Unit, SourceHouse, WarheadTypeExt::EMP_WH);

	// unit can be affected.
	if (isEMPProne) {
		// buildings are emp prone if they consume power and need it to function
		if (BuildingClass * VictimBuilding = specific_cast<BuildingClass *> (Unit)) {
			BuildingTypeClass * VictimBuildingType = VictimBuilding->Type;
			isEMPProne = ((VictimBuildingType->Powered && (VictimBuildingType->PowerDrain > 0))
				          || VictimBuildingType->TogglePower);

			// may have a special function.
			if (VictimBuildingType->Radar ||
				(VictimBuildingType->SuperWeapon> -1) || (VictimBuildingType->SuperWeapon2 > -1)
				|| VictimBuildingType->UndeploysInto
				|| VictimBuildingType->PowersUnit
				|| VictimBuildingType->Sensors
				|| VictimBuildingType->LaserFencePost) {
					isEMPProne = true;
			}
		} else {
			// affected only if this is a cyborg.
			if (InfantryClass * VictimInfantry = specific_cast<InfantryClass *> (Unit)) {
				isEMPProne = VictimInfantry->Type->Cyborg_;
			} else {
				// this is a vessel or vehicle that is not organic.
				isEMPProne = !Unit->GetTechnoType()->Organic;
			}
		}
	}

	return isEMPProne;
}

int WarheadTypeExt::getCappedDuration(int CurrentValue, int Duration, int Cap) {
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

void WarheadTypeExt::updateRadarBlackout(TechnoClass * Techno) {
	if (BuildingClass * Building = specific_cast<BuildingClass *>(Techno)) {
		if (!Building->Type->InvisibleInGame) {
			if (Building->Type->Radar) {
				if (verbose)
					Debug::Log("[updateRadarBlackout] Before: %d, %d\n",
							Building->EMPLockRemaining,
							Building->Owner->RadarBlackoutTimer.TimeLeft);
				if (Building->EMPLockRemaining > 0) {
					Building->Owner->CreateRadarOutage(Building->EMPLockRemaining);
				} else {
					Building->Owner->RadarBlackoutTimer.TimeLeft = 0; // trigger the radar outage check
					Building->Owner->RadarBlackout = true;
				}
				if (verbose)
					Debug::Log("[updateRadarBlackout] After: %d\n",
							Building->Owner->RadarBlackoutTimer.TimeLeft);
			}
		}
	}
}

bool WarheadTypeExt::enableEMPEffect(TechnoClass * Victim, ObjectClass * Souce) {
	if (BuildingClass * Building = specific_cast<BuildingClass *>(Victim)) {
		Building->DisableStuff();
		if (Building->Type->Radar) {
			if (WarheadTypeExt::verbose)
				Debug::Log("[enableEMPEffect] Radar down for: %d\n", Building->EMPLockRemaining);
			Building->Owner->CreateRadarOutage(Building->EMPLockRemaining);
		}
	} else {
		if (AircraftClass * Aircraft = specific_cast<AircraftClass *>(Victim)) {
			// crash flying aircraft
			if (Aircraft->InAir) {
				if (WarheadTypeExt::verbose)
					Debug::Log("[enableEMPEffect] Plane crash: %s\n", Aircraft->get_ID());
				Aircraft->Crash(Souce);
				return true;
			}
		}
		if (FootClass * Foot = generic_cast<FootClass *>(Victim)) {
			if (Foot->Locomotor->Is_Moving()) {
				Foot->Locomotor->Stop_Moving();
				if (!Foot->Deactivated)
					Foot->Deactivate();
			}
			Foot->Locomotor->Power_Off();
		}
	}

	// the unit still lives.
	return false;
}

void WarheadTypeExt::disableEMPEffect(TechnoClass * Victim) {
	if (BuildingClass * Building = specific_cast<BuildingClass *>(Victim)) {
		if (!Building->Type->InvisibleInGame) {
			Building->EnableStuff();
			if (Building->Type->Radar) {
				updateRadarBlackout(Building);
			}
		}
	} else {
		if (FootClass * Foot = generic_cast<FootClass *>(Victim)) {
			Foot->Locomotor->Power_On();
			if (Foot->Deactivated)
				Foot->Reactivate();
		}
		TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(Victim);
		if (pData && pData->EMPSparkleAnim) {
			pData->EMPSparkleAnim->RemainingIterations = 0; // basically "you don't need to show up anymore"
			pData->EMPSparkleAnim = NULL;
		}
	}
}

void WarheadTypeExt::createEMPulse(EMPulseClass *pThis, TechnoClass *pGenerator) {
	// fill the gaps
	HouseClass *pHouse = (pGenerator ? pGenerator->Owner : NULL);

	// get cap and other extended properties.
	WarheadTypeExt::ExtData *pWH = WarheadTypeExt::ExtMap.Find(WarheadTypeExt::EMP_WH);
	int Cap = (pWH ? pWH->EMP_Cap : -1);

	if (WarheadTypeExt::verbose)
		Debug::Log("[createEMPulse] Duration: %d\n", pThis->Duration);

	CellStruct cellCoords = MapClass::Instance->GetCellAt(&pThis->BaseCoords)->MapCoords;
	int countCells = CellSpread::NumCells(pThis->Spread);
	for (int i = 0; i < countCells; ++i) {
		CellStruct tmpCell = CellSpread::GetCell(i);
		tmpCell += cellCoords;
		CellClass *c = MapClass::Instance->GetCellAt(&tmpCell);
		for (ObjectClass *curObj = c->GetContent(); curObj; curObj = curObj->NextObject) {
			if (TechnoClass * curTechno = generic_cast<TechnoClass *> (curObj)) {
				if (WarheadTypeExt::verbose)
					Debug::Log("[createEMPulse] Step 1: %s => %s\n",
							(pGenerator ? pGenerator->get_ID() : NULL),
							curTechno->get_ID());

				if (WarheadTypeExt::canBeEMPAffected(curTechno, pHouse)) {
					if (WarheadTypeExt::verbose)
						Debug::Log("[createEMPulse] Step 2: %s\n",
								curTechno->get_ID());
					// get the new capped value
					int oldValue = curTechno->EMPLockRemaining;
					int newValue = WarheadTypeExt::getCappedDuration(oldValue, pThis->Duration, Cap);

					if (WarheadTypeExt::verbose)
						Debug::Log("[createEMPulse] Step 3: %d\n",
								newValue);

					// can not be less than zero
					curTechno->EMPLockRemaining = max(0, newValue);
					if (WarheadTypeExt::verbose)
						Debug::Log("[createEMPulse] Step 4: %d\n",
								newValue);

					// newly de-paralyzed
					if ((oldValue > 0) && (curTechno->EMPLockRemaining <= 0)) {
						if (WarheadTypeExt::verbose)
							Debug::Log("[createEMPulse] Step 5a\n");
						WarheadTypeExt::disableEMPEffect(curTechno);
					} else if ((oldValue <= 0) && (curTechno->EMPLockRemaining > 0)) {
						// newly paralyzed unit
						if (WarheadTypeExt::verbose)
							Debug::Log("[createEMPulse] Step 5b\n");
						if (WarheadTypeExt::enableEMPEffect(curTechno, pGenerator)) {
							continue;
						}
						// Set the animation.
						TechnoExt::ExtData *pData = TechnoExt::ExtMap.Find(curTechno);
						if (!pData->EMPSparkleAnim) {
							GAME_ALLOC(AnimClass, pData->EMPSparkleAnim, RulesClass::Instance->EMPulseSparkles, &curTechno->Location);
							//pData->EMPSparkleAnim->RemainingIterations = -1;
							pData->EMPSparkleAnim->SetOwnerObject(curTechno);
						}
					} else if (oldValue != newValue) {
						// At least update the radar, if this is one.
						WarheadTypeExt::updateRadarBlackout(curTechno);
						if (WarheadTypeExt::verbose)
							Debug::Log("[createEMPulse] Step 5c\n");
					}
				}
			}
		}
	}
}

DEFINE_HOOK(5240BD, CyborgParsingMyArse, 7) {
	return 0x5240C4;
}

// completely new implementation of the EMPulse.
DEFINE_HOOK(4C54E0, EMPulseClass_Initialize, 6) {
	GET(EMPulseClass *, pThis, ECX);
	GET_STACK(TechnoClass *, pGenerator, 0x4);

	WarheadTypeExt::createEMPulse(pThis, pGenerator);

	// skip old function entirely.
	return 0x4C58B6;
}

// this does not do anything different now, but
// we will call our own sparkle animation retrival
// function that is quicker.
DEFINE_HOOK(6FAF0D, TechnoClass_Update_EMPDuration, 6) {
	GET(TechnoClass *, pThis, ESI);

	if (pThis->EMPLockRemaining) {
		--pThis->EMPLockRemaining;
		if (!pThis->EMPLockRemaining) {
			// the forced vacation just ended.
			WarheadTypeExt::disableEMPEffect(pThis);
		}
	}

	return 0x6FAFFD;
}
