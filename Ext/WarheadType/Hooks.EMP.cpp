#include "Body.h"
#include "../TechnoType//Body.h"
bool Verbose = true;

bool IsTypeImmune(TechnoClass * Unit, HouseClass * SourceHouse) {
	// If houses differ, TypeImmune does not count.
	if (Unit->Owner == SourceHouse) {
		return false;
	}

	// Find an EMP weapon.
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
				// This unit can fire emps and type immunity
				// grants it to never be affected.
				return true;
			}
		}
	}

	return false;
}

bool IsEMPImmune(TechnoClass * Unit, HouseClass * SourceHouse) {
	// iron curtained objects can not be emp'd
	if (Unit->IsIronCurtained()) {
		return true;
	}

	// this can be overridden by a flag on the techno.
	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(Unit->GetTechnoType());
	if (pData->ImmuneToEMP) {
		if (Verbose)
			Debug::Log("[IsEMPImmune] \"%s\" is ImmuneToEMP.\n", Unit->get_ID());
		return true;
	}

	// ignore if type immune. don't even try.
	if (IsTypeImmune(Unit, SourceHouse)) {
		// This unit can fire emps and type immunity
		// grants it to never be affected.
		if (Verbose)
			Debug::Log("[IsEMPImmune] \"%s\" is TypeImmune.\n", Unit->get_ID());
		return true;
	}

	return false;
}

bool CanBeAffected(TechnoClass * Unit, HouseClass * SourceHouse) {
	// check whether this techno can be affected at all
	bool isEMPProne = !IsEMPImmune(Unit, SourceHouse);

	// unit can be affected.
	if (isEMPProne) {
		// buildings are emp prone if they consume power and need it to function
		if (BuildingClass * VictimBuilding = specific_cast<BuildingClass *> (Unit)) {
			BuildingTypeClass * VictimBuildingType = VictimBuilding->Type;
			isEMPProne = (VictimBuildingType->Powered && (VictimBuildingType->PowerDrain > 0));

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

int GetCappedDuration(int CurrentValue, int Duration, int Cap) {
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

void UpdateRadarBlackout(TechnoClass * Techno) {
	if (BuildingClass * Building = specific_cast<BuildingClass *>(Techno)) {
		if (!Building->Type->InvisibleInGame) {
			if (Building->Type->Radar) {
				if (Verbose)
					Debug::Log("[UpdateRadarBlackout] Before: %d, %d\n",
							Building->EMPLockRemaining,
							Building->Owner->RadarBlackoutTimer.TimeLeft);
				if (Building->EMPLockRemaining > 0) {
					Building->Owner->CreateRadarOutage(Building->EMPLockRemaining);
				} else {
					Building->Owner->RadarBlackout = 1; // trigger the radar outage check
				}
				if (Verbose)
					Debug::Log("[UpdateRadarBlackout] After: =%d\n",
							Building->Owner->RadarBlackoutTimer.TimeLeft);
			}
		}
	}
}

bool EnableEMPEffect(TechnoClass * Victim, ObjectClass * Souce) {
	if (BuildingClass * Building = specific_cast<BuildingClass *>(Victim)) {
		Building->DisableStuff();
		if (Building->Type->Radar) {
			Building->Owner->CreateRadarOutage(Building->EMPLockRemaining);
		}
	} else {
		if (AircraftClass * Aircraft = specific_cast<AircraftClass *>(Victim)) {
			// crash flying aircraft
			if (Aircraft->InAir) {
				Aircraft->Crash(Souce);
				return true;
			}
		}
		if (FootClass * Foot = generic_cast<FootClass *>(Victim)) {
			if (Foot->Locomotor->Is_Moving()) {
				Foot->Locomotor->Stop_Moving();
			}
			Foot->Locomotor->Power_Off();
		}
	}

	// the unit still lives.
	return false;
}

void DisableEMPEffect(TechnoClass * Victim) {
	if (BuildingClass * Building = specific_cast<BuildingClass *>(Victim)) {
		if (!Building->Type->InvisibleInGame) {
			Building->EnableStuff();
			if (Building->Type->Radar) {
				UpdateRadarBlackout(Building);
			}
		}
	} else {
		if (FootClass * Foot = generic_cast<FootClass *>(Victim)) {
			Foot->Locomotor->Power_On();
		}
		for (int i = 0; i < AnimClass::Array->Count; ++i) {
			if (AnimClass *Anim = AnimClass::Array->GetItem(i)) {
				if (Anim->OwnerObject == Victim) {
					if (Anim->Type == RulesClass::Instance->EMPulseSparkles) {
						Anim->RemainingIterations = 0; // basically "you don't need to show up anymore"
					}
				}
			}
		}
	}
}

bool EMPSparkleExists(TechnoClass * Victim) {
	for (int i = 0; i < AnimClass::Array->Count; ++i) {
		if (AnimClass *Anim = AnimClass::Array->GetItem(i)) {
			if (Anim->OwnerObject == Victim) {
				if (Anim->Type == RulesClass::Instance->EMPulseSparkles) {
					return true;
				}
			}
		}
	}

	return false;
}

DEFINE_HOOK(4C575E, EMPulseClass_CyborgCheck, 7) {
	GET(TechnoClass *, curVictim, ESI);
	return curVictim->GetTechnoType()->Cyborg_ ? 0x4C577A : 0;
}

DEFINE_HOOK(5240BD, CyborgParsingMyArse, 7) {
	return 0x5240C4;
}

// completely new implementation of the EMPulse.
DEFINE_HOOK(4C54E0, EMPulseClass_Initialize, 6) {
	GET(EMPulseClass *, pThis, ECX);
	GET_STACK(TechnoClass *, pGenerator, 0x4);

	HouseClass *pHouse = (pGenerator ? pGenerator->Owner : NULL);

	// Get cap.
	WarheadTypeExt::ExtData *pData = WarheadTypeExt::TechnoExt[pGenerator];
	int Cap = (pData ? pData->EMP_Cap : -1);

	if (Verbose)
		Debug::Log("[EMPulseClass_Initialize] Duration: %d\n", pThis->Duration);

	CellStruct cellCoords = MapClass::Instance->GetCellAt(&pThis->BaseCoords)->MapCoords;
	int countCells = CellSpread::NumCells(pThis->Spread);
	for (int i = 0; i < countCells; ++i) {
		CellStruct tmpCell = CellSpread::GetCell(i);
		tmpCell += cellCoords;
		CellClass *c = MapClass::Instance->GetCellAt(&tmpCell);
		for (ObjectClass *curObj = c->GetContent(); curObj; curObj = curObj->NextObject) {
			if (TechnoClass * curTechno = generic_cast<TechnoClass *> (curObj)) {
				if (Verbose)
					Debug::Log("[EMPulseClass_Initialize] Step 1: %s => %s\n",
							(pGenerator ? pGenerator->get_ID() : NULL),
							curTechno->get_ID());

				if (CanBeAffected(curTechno, pHouse)) {
					if (Verbose)
						Debug::Log("[EMPulseClass_Initialize] Step 2: %s\n",
								curTechno->get_ID());
					// get the new capped value
					int oldValue = curTechno->EMPLockRemaining;
					int newValue = GetCappedDuration(oldValue, pThis->Duration, Cap);

					if (Verbose)
						Debug::Log("[EMPulseClass_Initialize] Step 3: %d\n",
								newValue);

					// can not be less than zero
					curTechno->EMPLockRemaining = max(0, newValue);
					if (Verbose)
						Debug::Log("[EMPulseClass_Initialize] Step 4: %d\n",
								newValue);

					// newly de-paralyzed
					if ((oldValue > 0) && (curTechno->EMPLockRemaining <= 0)) {
						if (Verbose)
							Debug::Log("[EMPulseClass_Initialize] Step 5a\n");
						DisableEMPEffect( curTechno);
					} else if ((oldValue <= 0) && (curTechno->EMPLockRemaining > 0)) {
						// newly paralyzed unit
						if (Verbose)
							Debug::Log("[EMPulseClass_Initialize] Step 5b\n");
						if (EnableEMPEffect(curTechno, pGenerator)) {
							continue;
						}
						// Set the animation.
						if (!EMPSparkleExists(curTechno)) {
							AnimClass *EMPSparkles;
							GAME_ALLOC(AnimClass, EMPSparkles, RulesClass::Instance->EMPulseSparkles, &curTechno->Location);
							EMPSparkles->RemainingIterations = -1;
							EMPSparkles->SetOwnerObject(curTechno);
						}
					} else {
						// At least update the radar, if this is one.
						UpdateRadarBlackout( curTechno);
					}
				}
			}
		}
	}

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
			DisableEMPEffect(pThis);
		}
	}

	return 0x6FAFFD;
}
