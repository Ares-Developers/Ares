#include "Body.h"
#include <WeaponTypeClass.h>
#include "../../Enum/ArmorTypes.h"
#include "../Techno/Body.h"
#include "Misc/EMPulse.h"

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

template<> WarheadTypeExt::TT *Container<WarheadTypeExt>::SavingObject = NULL;
template<> IStream *Container<WarheadTypeExt>::SavingStream = NULL;

hash_ionExt WarheadTypeExt::IonExt;

WarheadTypeClass * WarheadTypeExt::Temporal_WH = NULL;

WarheadTypeClass * WarheadTypeExt::EMP_WH = NULL;

void WarheadTypeExt::ExtData::LoadFromINIFile(WarheadTypeClass *pThis, CCINIClass *pINI)
{
	const char * section = pThis->ID;

	INI_EX exINI(pINI);

	if(!pINI->GetSection(section)) {
		return;
	}

	// writing custom verses parser just because
	if(pINI->ReadString(section, "Verses", "", Ares::readBuffer, Ares::readLength)) {
		int idx = 0;
		for(char *cur = strtok(Ares::readBuffer, ","); cur; cur = strtok(NULL, ",")) {
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
		this->Temporal_WarpAway.Parse(&exINI, section, "Temporal.WarpAway");
	}

	this->DeployedDamage = pINI->ReadDouble(section, "Damage.Deployed", this->DeployedDamage);

	this->Ripple_Radius = pINI->ReadInteger(section, "Ripple.Radius", this->Ripple_Radius);

	this->AffectsEnemies = pINI->ReadBool(section, "AffectsEnemies", this->AffectsEnemies);

	this->InfDeathAnim.Parse(&exINI, section, "InfDeathAnim");

	this->KillDriver = pINI->ReadBool(section, "KillDriver", this->KillDriver);

	this->Malicious.Read(&exINI, section, "Malicious");
};

void Container<WarheadTypeExt>::InvalidatePointer(void *ptr) {
	AnnounceInvalidPointerMap(WarheadTypeExt::IonExt, ptr);
	AnnounceInvalidPointer(WarheadTypeExt::Temporal_WH, ptr);
}

// =============================
// load/save
void Container<WarheadTypeExt>::Save(WarheadTypeClass *pThis, IStream *pStm) {
	WarheadTypeExt::ExtData* pData = this->SaveKey(pThis, pStm);

	if(pData) {
		ULONG out;
		pData->Verses.Save(pStm);
	}
}
/*
		pStm->Write(&IonBlastClass::Array->Count, 4, &out);
		for(int ii = 0; ii < IonBlastClass::Array->Count; ++ii) {
			IonBlastClass *ptr = IonBlastClass::Array->Items[ii];
			pStm->Write(ptr, 4, &out);
			pStm->Write(WarheadTypeExt::IonExt[ptr], 4, &out);
		}
*/
void Container<WarheadTypeExt>::Load(WarheadTypeClass *pThis, IStream *pStm) {
	WarheadTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	pData->Verses.Load(pStm, 0);

	SWIZZLE(pData->Temporal_WarpAway);
}

/*!
	This function checks if the passed warhead has Ripple.Radius set, and, if so, applies the effect.
	\note Moved here from hook BulletClass_Fire.
	\param coords The coordinates of the warhead impact, the center of the Ripple area.
*/
void WarheadTypeExt::ExtData::applyRipples(CoordStruct *coords) {
	if (this->Ripple_Radius) {
		IonBlastClass *IB;
		GAME_ALLOC(IonBlastClass, IB, *coords);
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
void WarheadTypeExt::ExtData::applyIronCurtain(CoordStruct *coords, HouseClass* Owner, int damage) {
	CellStruct cellCoords = MapClass::Instance->GetCellAt(coords)->MapCoords;

	if(this->IC_Duration != 0) {
		// set of affected objects. every object can be here only once.
		DynamicVectorClass<TechnoClass*> *items = Helpers::Alex::getCellSpreadItems(coords,
			this->AttachedToObject->CellSpread, true);

		// affect each object
		for(int i=0; i<items->Count; ++i) {
			if(TechnoClass *curTechno = items->GetItem(i)) {

				// affects enemies or allies respectively?
				if(WarheadTypeExt::canWarheadAffectTarget(curTechno, Owner, this->AttachedToObject)) {

					// respect verses the boolean way
					if(abs(this->Verses[curTechno->GetTechnoType()->Armor].Verses) < 0.001) {
						break;
					}

					// get the values
					int oldValue = (curTechno->IronCurtainTimer.Ignorable() ? 0 : curTechno->IronCurtainTimer.TimeLeft);
					int newValue = Helpers::Alex::getCappedDuration(oldValue, this->IC_Duration, this->IC_Cap);

					// update iron curtain
					if(oldValue <= 0) {
						// start iron curtain effect?
						if(newValue > 0) {
							// damage the victim before ICing it
							if(damage) {
								curTechno->ReceiveDamage(&damage, 0, this->AttachedToObject, NULL, true, false, Owner);
							}

							// unit may be destroyed already.
							if(curTechno->IsAlive) {
								curTechno->IronCurtain(newValue, Owner, 0);
							}
						}
					} else {
						// iron curtain effect is already on.
						if(newValue > 0) {
							// set new length
							curTechno->IronCurtainTimer.TimeLeft = newValue;
						} else {
							// turn iron curtain off
							curTechno->IronCurtainTimer.TimeLeft = 1;
						}
					}
				}
			}
		}

		items->Clear();
		delete items;
	}
}

/*!
	This function checks if the passed warhead has EMP.Duration set, and, if so, applies the effect.
	\note Moved here from hook BulletClass_Fire.
	\param coords The coordinates of the warhead impact, the center of the EMP area.
*/
void WarheadTypeExt::ExtData::applyEMP(CoordStruct *coords, TechnoClass *source) {
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
bool WarheadTypeExt::ExtData::applyPermaMC(CoordStruct *coords, HouseClass* Owner, ObjectClass* Target) {
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
			pTarget->QueueMission(mission_Guard, 0);

			CoordStruct XYZ = *coords;
			XYZ.Z += pType->MindControlRingOffset;

			AnimClass *MCAnim;
			GAME_ALLOC(AnimClass, MCAnim, RulesClass::Instance->PermaControlledAnimationType, &XYZ);
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
	} else if(TechnoClass *pTarget = generic_cast<TechnoClass *>(Bullet->Target)) {
		// don't penetrate the Iron Curtain // typedef IronCurtain ChastityBelt
		if(pTarget->IsIronCurtained()) {
			return false;
		}
		TechnoTypeClass *pTargetType = pTarget->GetTechnoType();
		TechnoTypeExt::ExtData* TargetTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType);

		// conditions: Warhead is KillDriver, target is Vehicle or Aircraft, but not protected and not a living being
		if(((pTarget->WhatAmI() == abs_Unit) || (pTarget->WhatAmI() == abs_Aircraft))
			&& !(pTarget->BeingWarpedOut || TargetTypeExt->ProtectedDriver || pTargetType->Organic || pTargetType->Natural)) {

			// if this aircraft is expected to dock to anything, don't allow killing its pilot
			// (reason being: the game thinks you lost the aircraft that just turned, and assumes you have free aircraft space,
			// allowing you to build more aircraft, for the docking spot that is still occupied by the previous plane.)
			if(AircraftClass * pTargetAircraft = specific_cast<AircraftClass *>(pTarget)) { // relying on short-circuit evaluation here - nest this if necessary
				if(pTargetAircraft->Type->AirportBound || pTargetAircraft->Type->Dock.Count) {
					return false;
				}
			}

			// If this vehicle uses Operator=, we have to take care of actual "physical" drivers, rather than theoretical ones
			FootClass *passenger = NULL;
			if(TargetTypeExt->IsAPromiscuousWhoreAndLetsAnyoneRideIt && (passenger = pTarget->Passengers.RemoveFirstPassenger())) {
				// kill first passenger
				passenger->RegisterDestruction(Bullet->Owner);
				passenger->UnInit();

			} else if(TargetTypeExt->Operator) {
				// kill first passenger of Operator= kind

				// temp holder for the preceeding non-operators
				PassengersClass worthlessOnes;

				// copy out worthless passengers until we find the driver cowardly hiding among them, then kill him
				while(pTarget->Passengers.FirstPassenger) {
					if(pTarget->Passengers.FirstPassenger->GetTechnoType() == TargetTypeExt->Operator) {
						FootClass *passenger = pTarget->Passengers.RemoveFirstPassenger();
						passenger->RegisterDestruction(Bullet->Owner);
						passenger->UnInit();
						break;
					}
					worthlessOnes.AddPassenger(pTarget->Passengers.RemoveFirstPassenger());
				}

				// copy the worthless scum back in
				while(worthlessOnes.FirstPassenger) {
					pTarget->Passengers.AddPassenger(worthlessOnes.RemoveFirstPassenger());
				}
			}

			// if passengers remain in the vehicle, operator-using or not, they should leave
			if(pTarget->Passengers.NumPassengers) {
				TechnoExt::EjectPassengers(pTarget, -1);
			}

			// If this unit is driving under influence, we have to free it first
			if(TechnoClass *Controller = pTarget->MindControlledBy) {
				if(CaptureManagerClass *MC = Controller->CaptureManager) {
					MC->FreeUnit(pTarget);
				}
			}

			// If this unit mind controls stuff, we should free the controllees, since they still belong to the previous owner
			if(pTarget->CaptureManager) {
				pTarget->CaptureManager->FreeAll();
			}

			// BelongsToATeam()
			// If this unit spawns stuff, we should kill the spawns, since they still belong to the previous owner
			if(pTarget->SpawnManager) {
				pTarget->SpawnManager->KillNodes();
				pTarget->SpawnManager->Target = NULL;
				pTarget->SpawnManager->Destination = NULL;
			}

			// If this unit enslaves stuff, we should free the slaves, since they still belong to the previous owner
			// <DCoder> SlaveManagerClass::Killed() sets the manager's Owner to NULL
			// <Renegade> okay, does Killed() also destroy the slave manager, or just unlink it from the unit?
			// <DCoder> unlink
			// <Renegade> so on principle, I could just re-link it?
			// <DCoder> yes you can
			if(SlaveManagerClass * pSlaveManager = pTarget->SlaveManager) {
				pSlaveManager->Killed(Bullet->Owner);
				pSlaveManager->ZeroOutSlaves();
				pTarget->SlaveManager->Owner = pTarget;
			}

			// Hand over to Civilian/Special house
			pTarget->SetOwningHouse(HouseClass::FindByCountryIndex(HouseTypeClass::FindIndexOfName("Special")));
			pTarget->QueueMission(mission_Sticky, true);
			return true;
		} else {
			return false;
		}
	}
	return false;
}

// =============================
// container hooks

DEFINE_HOOK(75D1A9, WarheadTypeClass_CTOR, 7)
{
	GET(WarheadTypeClass*, pItem, EBP);

	WarheadTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(75E510, WarheadTypeClass_DTOR, 6)
{
	GET(WarheadTypeClass*, pItem, ECX);

	WarheadTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(75E0C0, WarheadTypeClass_SaveLoad_Prefix, 8)
DEFINE_HOOK_AGAIN(75E2C0, WarheadTypeClass_SaveLoad_Prefix, 5)
{
	GET_STACK(WarheadTypeExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<WarheadTypeExt>::SavingObject = pItem;
	Container<WarheadTypeExt>::SavingStream = pStm;

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

DEFINE_HOOK(75DEA0, WarheadTypeClass_LoadFromINI, 5)
DEFINE_HOOK_AGAIN(75DEAF, WarheadTypeClass_LoadFromINI, 5)
{
	GET(WarheadTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x150);

	WarheadTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
