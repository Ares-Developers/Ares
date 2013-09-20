#include "Body.h"
#include "../Rules/Body.h"
#include "../TechnoType/Body.h"
#include "../../Misc/SWTypes.h"
#include "../../Misc/PoweredUnitClass.h"

#include <HouseClass.h>
#include <BuildingClass.h>
#include <GeneralStructures.h>
#include <Helpers/Template.h>
#include <SlaveManagerClass.h>
#include <SpawnManagerClass.h>

template<> const DWORD Extension<TechnoClass>::Canary = 0x55555555;
Container<TechnoExt> TechnoExt::ExtMap;

template<> TechnoExt::TT *Container<TechnoExt>::SavingObject = NULL;
template<> IStream *Container<TechnoExt>::SavingStream = NULL;

FireError::Value TechnoExt::FiringStateCache = FireError::NotAValue;

bool TechnoExt::NeedsRegap = false;

void TechnoExt::SpawnSurvivors(FootClass *pThis, TechnoClass *pKiller, bool Select, bool IgnoreDefenses)
{
	TechnoTypeClass *Type = pThis->GetTechnoType();

	HouseClass *pOwner = pThis->Owner;
	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(Type);
	TechnoExt::ExtData *pSelfData = TechnoExt::ExtMap.Find(pThis);

	CoordStruct loc = pThis->Location;
	int chance = pData->Survivors_PilotChance.Get(pThis);
	if(chance < 0) {
		chance = int(RulesClass::Instance->CrewEscape * 100);
	}

	// always eject passengers, but crew only if not already processed.
	if(!pSelfData->Survivors_Done && !pSelfData->DriverKilled && !IgnoreDefenses) {
		// save this, because the hijacker can kill people
		int PilotCount = pData->Survivors_PilotCount;
		if(PilotCount < 0) {
			// default pilot count, depending on crew
			PilotCount = (Type->Crewed ? 1 : 0);
		}

		// process the hijacker
		if(InfantryClass *Hijacker = RecoverHijacker(pThis)) {
			TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(Hijacker->Type);

			if(!EjectRandomly(Hijacker, loc, 144, Select)) {
				Hijacker->RegisterDestruction(pKiller);
				GAME_DEALLOC(Hijacker);
			} else {
				// the hijacker will now be controlled instead of the unit
				if(TechnoClass* pController = pThis->MindControlledBy) {
					++Unsorted::IKnowWhatImDoing; // disables sound effects
					pController->CaptureManager->FreeUnit(pThis);
					pController->CaptureManager->CaptureUnit(Hijacker); // does the immunetopsionics check for us
					--Unsorted::IKnowWhatImDoing;
					Hijacker->QueueMission(mission_Guard, true); // override the fate the AI decided upon
					
				}
				VocClass::PlayAt(pExt->HijackerLeaveSound, &pThis->Location, 0);

				// lower than 0: kill all, otherwise, kill n pilots
				PilotCount = ((pExt->HijackerKillPilots < 0) ? 0 : (PilotCount - pExt->HijackerKillPilots));
			}
		}

		// possibly eject up to PilotCount crew members
		if(Type->Crewed && chance > 0) {
			signed int idx = pOwner->SideIndex;
			auto Pilots = &pData->Survivors_Pilots;
			if(Pilots->ValidIndex(idx)) {
				if(InfantryTypeClass *PilotType = Pilots->GetItem(idx)) {
					for(int i = 0; i < PilotCount; ++i) {
						if(ScenarioClass::Instance->Random.RandomRanged(1, 100) <= chance) {
							InfantryClass *Pilot = reinterpret_cast<InfantryClass *>(PilotType->CreateObject(pOwner));
							Pilot->Health = (PilotType->Strength / 2);
							Pilot->Veterancy.Veterancy = pThis->Veterancy.Veterancy;

							if(!EjectRandomly(Pilot, loc, 144, Select)) {
								Pilot->RegisterDestruction(pKiller); //(TechnoClass *)R->get_StackVar32(0x54));
								GAME_DEALLOC(Pilot);
							} else {
								if(pThis->AttachedTag && pThis->AttachedTag->IsTriggerRepeating()) {
									Pilot->ReplaceTag(pThis->AttachedTag);
								}
							}
						}
					}
				}
			}
		}
	}

	// passenger escape chances
	chance = pData->Survivors_PassengerChance.Get(pThis);

	// eject or kill all passengers. if defenses are to be ignored, passengers
	// killed no matter what the odds are.
	while(pThis->Passengers.FirstPassenger) {
		bool toDelete = 1;
		FootClass *passenger = pThis->RemoveFirstPassenger();
		bool toSpawn = false;
		if(chance > 0) {
			toSpawn = ScenarioClass::Instance->Random.RandomRanged(1, 100) <= chance;
		} else if(chance == -1 && pThis->WhatAmI() == abs_Unit) {
			Move::Value occupation = passenger->IsCellOccupied(pThis->GetCell(), -1, -1, nullptr, true);
			toSpawn = (occupation == Move::OK || occupation == Move::MovingBlock);
		}
		if(toSpawn && !IgnoreDefenses) {
			toDelete = !EjectRandomly(passenger, loc, 128, Select);
		}
		if(toDelete) {
			passenger->RegisterDestruction(pKiller); //(TechnoClass *)R->get_StackVar32(0x54));
			passenger->UnInit();
		}
	}

	// do not ever do this again for this unit
	pSelfData->Survivors_Done = 1;
}
/**
	\param Survivor Passenger to eject
	\param loc Where to put the passenger
	\param Select Whether to select the Passenger afterwards
*/
bool TechnoExt::EjectSurvivor(FootClass *Survivor, CoordStruct *loc, bool Select)
{
	bool success = false;
	bool chuted = false;
	CoordStruct tmpCoords;
	CellClass * pCell = MapClass::Instance->GetCellAt(loc);
	if(pCell == MapClass::InvalidCell()) {
		return false;
	}
	pCell->GetCoordsWithBridge(&tmpCoords);
	Survivor->OnBridge = pCell->ContainsBridge();

	int floorZ = tmpCoords.Z;
	if(loc->Z - floorZ > 208) {
		// HouseClass::CreateParadrop does this when building passengers for a paradrop... it might be a wise thing to mimic!
		Survivor->Remove();

		success = Survivor->SpawnParachuted(loc);
		chuted = true;
	} else {
		loc->Z = floorZ;
		success = Survivor->Put(loc, ScenarioClass::Instance->Random.RandomRanged(0, 7));
	}
	RET_UNLESS(success);
	Survivor->Transporter = NULL;
	Survivor->LastMapCoords = pCell->MapCoords;

	// don't ask, don't tell
	if(chuted) {
		bool scat = Survivor->OnBridge;
		auto occupation = scat ? pCell->AltOccupationFlags : pCell->OccupationFlags;
		if((occupation & 0x1C) == 0x1C) {
			pCell->ScatterContent(0xA8F200, 1, 1, scat);
		}
	} else {
		Survivor->Scatter(0xB1CFE8, 1, 0);
		Survivor->QueueMission(Survivor->Owner->ControlledByHuman() ? mission_Guard : mission_Hunt, 0);
	}
	Survivor->ShouldEnterOccupiable = Survivor->ShouldGarrisonStructure = false;

	if(Select) {
		Survivor->Select();
	}
	return 1;
	//! \todo Tag
}

/**
	This function ejects a given number of passengers from the passed transporter.

	\param pThis Pointer to the transporter
	\param howMany How many passengers to eject - pass negative number for "all"
	\author Renegade
	\date 27.05.2010
*/
void TechnoExt::EjectPassengers(FootClass *pThis, signed short howMany) {
	if(howMany == 0 || !pThis->Passengers.NumPassengers) {
		return;
	}

	short limit = (howMany < 0 || howMany > pThis->Passengers.NumPassengers) ? (short)pThis->Passengers.NumPassengers : howMany;

	for(short i = 0; (i < limit) && pThis->Passengers.FirstPassenger; ++i) {
		FootClass *passenger = pThis->RemoveFirstPassenger();
		if(!EjectRandomly(passenger, pThis->Location, 128, false)) {
			passenger->UnInit();
		}
	}
	return;
}


/**
	This function drops the coordinates of an infantry subposition into the target parameter.
	Could probably work for vehicles as well, though they'd be off-center.

	\param current The current position of the transporter, the starting point to look from
	\param target A CoordStruct to save the finally computed position to
	\param distance The distance in leptons from the current position
	\author Renegade
	\date 27.05.2010
*/
void TechnoExt::GetPutLocation(CoordStruct const &current, CoordStruct &target, int distance) {
	// this whole thing does not at all account for cells which are completely occupied.
	CoordStruct tmpLoc = current;
	CellStruct tmpCoords = CellSpread::GetCell(ScenarioClass::Instance->Random.RandomRanged(0, 7));

	tmpLoc.X += tmpCoords.X * distance;
	tmpLoc.Y += tmpCoords.Y * distance;

	CellClass * tmpCell = MapClass::Instance->GetCellAt(&tmpLoc);

	tmpCell->FindInfantrySubposition(&target, &tmpLoc, 0, 0, 0);

	target.Z = current.Z;
	return;
}

//! Places a unit next to a given location on the battlefield.
/**
	
	\param pEjectee The FootClass to be ejected.
	\param location The current position of the transporter, the starting point to look from
	\param distance The distance in leptons from the current position
	\param select Whether the placed FootClass should be selected
	\author AlexB
	\date 12.04.2011
*/
bool TechnoExt::EjectRandomly(FootClass* pEjectee, CoordStruct const &location, int distance, bool select) {
	CoordStruct destLoc;
	GetPutLocation(location, destLoc, distance);
	return TechnoExt::EjectSurvivor(pEjectee, &destLoc, select);
}

//! Breaks the link between DrainTarget and DrainingMe.
/*!
	The links between the drainer and its victim are removed and the draining
	animation is no longer played.

	\param Drainer The Techno that drains power or credits.
	\param Drainee The Techno that power or credits get gets drained from.

	\author AlexB
	\date 2010-04-27
*/
void TechnoExt::StopDraining(TechnoClass *Drainer, TechnoClass *Drainee) {
	// fill the gaps
	if(Drainer && !Drainee)
		Drainee = Drainer->DrainTarget;

	if(!Drainer && Drainee)
		Drainer = Drainee->DrainingMe;

	// sanity check, then end draining.
	if(Drainer && Drainee) {
		// stop the animation.
		if (Drainer->DrainAnim) {
			Drainer->DrainAnim->UnInit();
			Drainer->DrainAnim = NULL;
		}

		// remove links.
		Drainee->DrainingMe = NULL;
		Drainer->DrainTarget = NULL;

		// tell the game to recheck the drained
		// player's tech level.
		if (Drainee->Owner) {
			Drainee->Owner->ShouldRecheckTechTree = true;
		}
	}
}

unsigned int TechnoExt::ExtData::AlphaFrame(SHPStruct *Image) {
	int countFrames = Conversions::Int2Highest(Image->Frames);
	DirStruct Facing;
	this->AttachedToObject->Facing.GetFacing(&Facing);
	return (Facing.Value >> (16 - countFrames));
}

bool TechnoExt::ExtData::DrawVisualFX() {
	TechnoClass * Object = this->AttachedToObject;
	if(Object->VisualCharacter(VARIANT_TRUE, Object->Owner) == VisualType::Normal) {
		if(!Object->Disguised) {
			return true;
		}
	}
	return false;
}

UnitTypeClass * TechnoExt::ExtData::GetUnitType() {
	if(UnitClass * U = specific_cast<UnitClass *>(this->AttachedToObject)) {
		TechnoTypeExt::ExtData * pData = TechnoTypeExt::ExtMap.Find(U->Type);
		if(pData->WaterImage && !U->OnBridge && U->GetCell()->LandType == lt_Water) {
			return pData->WaterImage;
		}
	}
	return NULL;
}

void Container<TechnoExt>::InvalidatePointer(void *ptr, bool bRemoved) {
	AnnounceInvalidPointerMap(TechnoExt::AlphaExt, ptr);
	AnnounceInvalidPointerMap(TechnoExt::SpotlightExt, ptr);
	AnnounceInvalidPointer(TechnoExt::ActiveBuildingLight, ptr);
}

/*! This function checks if this object can currently be used, in terms of having or needing an operator.
	\return true if it needs an operator and has one, <b>or if it doesn't need an operator in the first place</b>. false if it needs an operator and doesn't have one.
	\author Renegade
	\date 27.04.10
*/
bool TechnoExt::ExtData::IsOperated() {
	TechnoTypeExt::ExtData* TypeExt = TechnoTypeExt::ExtMap.Find(this->AttachedToObject->GetTechnoType());

	if(TypeExt->Operator) {
		if(this->AttachedToObject->Passengers.NumPassengers) {
			// loop & condition come from D
			for(ObjectClass* O = this->AttachedToObject->Passengers.GetFirstPassenger(); O; O = O->NextObject) {
				if(FootClass *F = generic_cast<FootClass *>(O)) {
					if(F->GetType() == TypeExt->Operator) {
						// takes a specific operator and someone is present AND that someone is the operator, therefore it is operated
						return true;
					}
				}
			}
			// takes a specific operator and someone is present, but it's not the operator, therefore it's not operated
			return false;
		} else {
			// takes a specific operator but no one is present, therefore it's not operated
			return false;
		}
	} else if(TypeExt->IsAPromiscuousWhoreAndLetsAnyoneRideIt) {
		// takes anyone, therefore it's operated if anyone is there
		return (this->AttachedToObject->Passengers.NumPassengers > 0);
	} else {
		/* Isn't even set as an Operator-using object, therefore we are returning TRUE,
		 since, logically, if it doesn't need operators, it can be/is operated, no matter if there are passengers or not.
		 (Also, if we didn't do this, Reactivate() would fail for for non-Operator-units, for example.) */
		return true;
	}
}

/*! This function checks if this object can currently be used, in terms of having or needing a powering structure and that structure's status.
	\return true if it needs a structure and has an active one, <b>or if it doesn't need a structure in the first place</b>. false if it needs a structure and doesn't have an active one.
	\author Renegade
	\date 27.04.10
*/
bool TechnoExt::ExtData::IsPowered() {
	TechnoTypeClass *TT = this->AttachedToObject->GetTechnoType();
	if(TT && TT->PoweredUnit) {
		HouseClass* Owner = this->AttachedToObject->Owner;
		for(int i = 0; i < Owner->Buildings.Count; ++i) {
			BuildingClass* Building = Owner->Buildings.GetItem(i);
			if(Building->Type->PowersUnit) {
				if(Building->Type->PowersUnit == TT) {
					return Building->RegisteredAsPoweredUnitSource && !Building->IsUnderEMP(); // alternatively, HasPower, IsPowerOnline()
				}
			}
		}
		// if we reach this, we found no building that currently powers this object
		return false;
	// #617
	} else if(this->PoweredUnit) {
		return this->PoweredUnit->IsPowered();
	} else {
		// object doesn't need a particular powering structure, therefore, for the purposes of the game, it IS powered
		return true;
	}
}

/*
 * Object should NOT be placed on the map (->Remove() it or don't Put in the first place)
 * otherwise Bad Things (TM) will happen. Again.
 */
bool TechnoExt::CreateWithDroppod(FootClass *Object, CoordStruct *XYZ) {
	auto MyCell = MapClass::Instance->GetCellAt(XYZ);
	if(Object->IsCellOccupied(MyCell, -1, -1, nullptr, false) != Move::OK) {
//		Debug::Log("Cell occupied... poof!\n");
		return false;
	} else {
//		Debug::Log("Destinating %s @ {%d, %d, %d}\n", Object->GetType()->ID, XYZ->X, XYZ->Y, XYZ->Z);
		LocomotionClass::ChangeLocomotorTo(Object, &LocomotionClass::CLSIDs::Droppod);
		CoordStruct xyz = *XYZ;
		xyz.Z = 0;
		Object->SetLocation(&xyz);
		Object->SetDestination(MyCell, 1);
		Object->Locomotor->Move_To(*XYZ);
		DirStruct Facing;
		Object->Facing.SetFacing(&Facing);
		if(!Object->InLimbo) {
			Object->See(0, 0);
			Object->QueueMission(mission_Guard, 0);
			Object->NextMission();
			return true;
		}
		//Debug::Log("InLimbo... failed?\n");
		return false;
	}
}

// destroy a given techno by dealing absolute damage
void TechnoExt::Destroy(TechnoClass* pTechno, TechnoClass* pKiller, HouseClass* pKillerHouse, WarheadTypeClass* pWarhead) {
	if(!pKillerHouse && pKiller) {
		pKillerHouse = pKiller->Owner;
	}

	if(!pWarhead) {
		pWarhead = RulesClass::Instance->C4Warhead;
	}

	int health = pTechno->Health;
	pTechno->ReceiveDamage(&health, 0, pWarhead, pKiller, TRUE, FALSE, pKillerHouse);
}

void TechnoExt::TransferIvanBomb(TechnoClass *From, TechnoClass *To) {
	if(auto Bomb = From->AttachedBomb) {
		From->AttachedBomb = NULL;
		Bomb->TargetUnit = To;
		To->AttachedBomb = Bomb;
		To->BombVisible = From->BombVisible;
		// if there already was a bomb attached to target unit, it's gone now...
		// it shouldn't happen though, this is used for (un)deploying objects only
	}
}

void TechnoExt::TransferAttachedEffects(TechnoClass *From, TechnoClass *To) {
	TechnoExt::ExtData *FromExt = TechnoExt::ExtMap.Find(From);
	TechnoExt::ExtData *ToExt = TechnoExt::ExtMap.Find(To);

	for (int i=0; i < ToExt->AttachedEffects.Count; ++i) {
		auto ToItem = ToExt->AttachedEffects.GetItem(i);
		ToItem->Destroy();
		delete ToItem;
	}
	ToExt->AttachedEffects.Clear();

	// while recreation itself isn't the best idea, less hassle and more reliable
	// list gets intact in the end
	for (int i=0; i < FromExt->AttachedEffects.Count; i++) {
		auto FromItem = FromExt->AttachedEffects.GetItem(i);
		FromItem->Type->Attach(To, FromItem->ActualDuration, FromItem->Invoker);
		//FromItem->Type->Attach(To, FromItem->ActualDuration, FromItem->Invoker, FromItem->ActualDamageDelay);
		FromItem->Destroy();
		delete FromItem;
	}

	FromExt->AttachedEffects.Clear();
	FromExt->AttachedTechnoEffect_isset = false;
	TechnoExt::RecalculateStats(To);
}

/*! This function recalculates the stats modifiable by crates and update them (aimed for request #255)
	\todo code that crate effects not get ignored
	\author Graion Dilach
	\date 2011-10-12
*/

void TechnoExt::RecalculateStats(TechnoClass *pTechno) {
	auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
	double Firepower = pTechnoExt->Crate_FirepowerMultiplier,
		Armor = pTechnoExt->Crate_ArmorMultiplier,
		Speed = pTechnoExt->Crate_SpeedMultiplier; //if there's hooks for crate-stuff, they could be the base for this
	bool Cloak = TechnoExt::CanICloakByDefault(pTechno) || pTechnoExt->Crate_Cloakable;

	//Debug::Log("[AttachEffect]Recalculating stats of %s...\n", pTechno->get_ID());

	for (int i = 0; i < pTechnoExt->AttachedEffects.Count; i++) {
		auto Item = pTechnoExt->AttachedEffects.GetItem(i);
		auto iType = Item->Type;
		//do not use *= here... Valuable sends GetEx and repositions the double
		Firepower = iType->FirepowerMultiplier * Firepower;
		Speed = iType->SpeedMultiplier * Speed;
		Armor = iType->ArmorMultiplier * Armor;
		Cloak = Cloak || !!iType->Cloakable;
	}

	pTechno->FirepowerMultiplier = Firepower;
	pTechno->ArmorMultiplier = Armor;

	pTechno->Cloakable = Cloak;

	if(FootClass *Foot = generic_cast<FootClass *>(pTechno)) {
		Foot->SpeedMultiplier = Speed;
	}

	//Debug::Log("[AttachEffect]Calculation was successful.\n", pTechno->get_ID());
}

/*! This function calculates whether the unit would be cloaked by default
	\author Graion Dilach
	\date 2011-10-16
*/
bool TechnoExt::CanICloakByDefault(TechnoClass *pTechno) {
	//Debug::Log("[AttachEffect]Can %s cloak by default?\n", pTechno->get_ID());
	auto tType = pTechno->GetTechnoType();
	return tType->Cloakable || pTechno->HasAbility(Abilities::CLOAK);
}

bool TechnoExt::ExtData::IsDeactivated() const {
	return this->AttachedToObject->Deactivated;
}

eAction TechnoExt::ExtData::GetDeactivatedAction(ObjectClass *Hovered) const {
	if(!Hovered) {
		return act_None;
	}
	if(auto tHovered = generic_cast<TechnoClass *>(Hovered)) {
		if(this->AttachedToObject->Owner->IsAlliedWith(tHovered)) {
			if(tHovered->IsSelectable()) {
				return act_Select;
			}
		}
	}
	return act_None;
}

void TechnoExt::ExtData::InvalidateAttachEffectPointer(void *ptr) {
	for(auto i = 0; i < this->AttachedEffects.Count; ++i) {
		this->AttachedEffects.GetItem(i)->InvalidatePointer(ptr);
	}
}

/*! This function detaches a specific spawned object from it's spawner.
	The check if it's a spawned object at all should be done before this function is called.
	Check for SpawnOwner, specifically.

	\param Spawnee The spawned object
	\param NewSpawneeOwner The house which should control the spawnee after the function
	\author Graion Dilach
	\date 2011-06-09
	\todo Get an assembly-reader to document Status in YR++ and update Status accordingly
*/
void TechnoExt::DetachSpecificSpawnee(TechnoClass *Spawnee, HouseClass *NewSpawneeOwner){

	// setting up the nodes. Funnily, nothing else from the manager is needed
	auto *SpawnNode = &(Spawnee->SpawnOwner->SpawnManager->SpawnedNodes);

	//find the specific spawnee in the node
	for (int i=0; i<SpawnNode->Count; ++i){

		if(Spawnee == SpawnNode->GetItem(i)->Unit) {

			SpawnNode->GetItem(i)->Unit = NULL;
			Spawnee->SpawnOwner = NULL;

			SpawnNode->GetItem(i)->Status = SpawnNode::state_Dead;

			Spawnee->SetOwningHouse(NewSpawneeOwner);
		}
	}
}

/*! This function frees a specific slave from it's manager.
	The check if it's a slave at all should be done before this function is called.
	Check for SlaveOwner, specifically.

	\param Slave The slave which should be freed
	\param Affector The house which causes this slave to be freed (where it should be freed to)
	\author Graion Dilach
	\date 2011-06-09
*/
void TechnoExt::FreeSpecificSlave(TechnoClass *Slave, HouseClass *Affector){

	//If you're a slave, you're an InfantryClass. But since most functions use TechnoClasses and the check can be done in that level as well
	//it's easier to set up the recasting in this function
	//Anybody who writes 357, take note that SlaveManager uses InfantryClasses everywhere, SpawnManager uses TechnoClasses derived from AircraftTypeClasses
	//as I wrote it in http://bugs.renegadeprojects.com/view.php?id=357#c10331
	//So, expand that one instead, kthx.

	if(InfantryClass * pSlave = specific_cast<InfantryClass *>(Slave)) {
		auto Manager= pSlave->SlaveOwner->SlaveManager;

		//LostSlave can free the unit from the miner, so we're awesome. 
		Manager->LostSlave(pSlave);
		pSlave->SlaveOwner = NULL;

		//OK, delinked, Now relink it to the side which separated the slave from the miner
		pSlave->SetOwningHouse(Affector);
	}
}

// If available, creates an InfantryClass instance and removes the hijacker from the victim.
InfantryClass* TechnoExt::RecoverHijacker(FootClass* pThis) {
	InfantryClass* Hijacker = NULL;
	if(pThis && pThis->HijackerInfantryType != -1) {
		if(InfantryTypeClass* HijackerType = InfantryTypeClass::Array->GetItem(pThis->HijackerInfantryType)) {
			TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
			TechnoTypeExt::ExtData* pTypeExt = TechnoTypeExt::ExtMap.Find(HijackerType);
			HouseClass* HijackerOwner = pExt->HijackerHouse ? pExt->HijackerHouse : pThis->Owner;
			if(!pTypeExt->HijackerOneTime && HijackerOwner && !HijackerOwner->Defeated) {
				Hijacker = reinterpret_cast<InfantryClass *>(HijackerType->CreateObject(HijackerOwner));
				Hijacker->Health = std::max(pExt->HijackerHealth / 2, 5);
			}
			pThis->HijackerInfantryType = -1;
			pExt->HijackerHealth = -1;
		}
	}
	return Hijacker;
}

// this isn't called VehicleThief action, because it also includes other logic
// related to infantry getting into an vehicle like CanDrive.
AresAction::Value TechnoExt::ExtData::GetActionHijack(TechnoClass* pTarget) {
	InfantryClass* pThis = specific_cast<InfantryClass*>(this->AttachedToObject);
	if(!pThis || !pTarget || !pThis->IsAlive || !pTarget->IsAlive) {
		return AresAction::None;
	}

	InfantryTypeClass* pType = pThis->Type;
	TechnoTypeClass* pTargetType = pTarget->GetTechnoType();
	TechnoTypeExt::ExtData* pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	// this can't steal vehicles
	if(!pType->VehicleThief && !pTypeExt->CanDrive) {
		return AresAction::None;
	}

	// i'm in a state that forbids capturing
	if(!this->IsOperated()) {
		return AresAction::None;
	}
	if(pType->Deployer) {
		Sequence::Value sequence = pThis->SequenceAnim;
		if(sequence == Sequence::Deploy || sequence == Sequence::Deployed
			|| sequence == Sequence::DeployedFire || sequence == Sequence::DeployedIdle) {
				return AresAction::None;
		}
	}

	// target type is not eligible (hijackers can also enter strange buildings)
	eAbstractType absTarget = pTarget->WhatAmI();
	if(absTarget != abs_Aircraft && absTarget != abs_Unit
		&& (!pType->VehicleThief || absTarget != abs_Building)) {
			return AresAction::None;
	}

	// target is bad
	if(pTarget->CurrentMission == mission_Selling || pTarget->IsBeingWarpedOut()
		|| pTargetType->IsTrain || pTargetType->BalloonHover
		|| (absTarget != abs_Unit && !pTarget->IsStrange())
		//|| (absTarget == abs_Unit && ((UnitTypeClass*)pTargetType)->NonVehicle) replaced by Hijacker.Allowed
		|| !pTarget->IsOnFloor()) {
			return AresAction::None;
	}

	// bunkered units can't be hijacked.
	if(pTarget->BunkerLinkedItem) {
		return AresAction::None;
	}

	// a thief that can't break mind control loses without trying further
	if(pType->VehicleThief) { 
		if(pTarget->IsMindControlled() && !pTypeExt->HijackerBreakMindControl) {
			return AresAction::None;
		}
	}

	 //drivers can drive, but only stuff owned by Special. if a driver is a vehicle thief
	 //also, it can reclaim units even if they are immune to hijacking (see below)
	bool specialOwned = !_strcmpi(pTarget->Owner->Type->ID, "Special");
	if(specialOwned && pTypeExt->CanDrive) {
		return AresAction::Drive;
	}

	// hijacking only affects enemies
	if(pType->VehicleThief) {
		// can't steal allied unit (CanDrive and special already handled)
		if(pThis->Owner->IsAlliedWith(pTarget->Owner) || specialOwned) {
			return AresAction::None;
		}

		TechnoTypeExt::ExtData* pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType);
		if(!pTargetTypeExt->HijackerAllowed) {
			return AresAction::None;
		}

		// allowed to steal from enemy
		return AresAction::Hijack;
	}

	// no hijacking ability
	return AresAction::None;
}

// perform the most appropriate hijack action
bool TechnoExt::ExtData::PerformActionHijack(TechnoClass* pTarget) {
	// was the hijacker lost in the process?
	bool ret = false;

	if(InfantryClass* pThis = specific_cast<InfantryClass*>(this->AttachedToObject)) {
		InfantryTypeClass* pType = pThis->Type;
		TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
		TechnoTypeExt::ExtData* pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		AresAction::Value action = pExt->GetActionHijack(pTarget);

		// abort capturing this thing, it looked
		// better from over there...
		if(!action) {
			pThis->SetDestination(NULL, true);
			CoordStruct crd;
			pTarget->GetCoords(&crd);
			pThis->Scatter((DWORD)&crd, 1, 0);
			return false;
		}

		// prepare for a smooth transition. free the destination from
		// any mind control. #762
		if(pTarget->MindControlledBy) {
			pTarget->MindControlledBy->CaptureManager->FreeUnit(pTarget);
		}
		pTarget->MindControlledByAUnit = false;
		if(pTarget->MindControlRingAnim) {
			pTarget->MindControlRingAnim->UnInit();
			pTarget->MindControlRingAnim = NULL;
		}

		bool asPassenger = false;
		if(action == AresAction::Drive) {
			TechnoTypeExt::ExtData* pDestTypeExt = TechnoTypeExt::ExtMap.Find(pTarget->GetTechnoType());
			if(pDestTypeExt->Operator || pDestTypeExt->IsAPromiscuousWhoreAndLetsAnyoneRideIt) {
				asPassenger = true;
			}
		}

		if(!asPassenger) {
			// raise some events in case the hijacker/driver will be
			// swallowed by the vehicle.
			if(pTarget->AttachedTag) {
				pTarget->AttachedTag->RaiseEvent(TriggerEvent::DestroyedByAnything, pThis, *(CellStruct*)0xA8F1E0, 0, 0);
			}
			pTarget->Owner->HasBeenThieved = true;
			if(pThis->AttachedTag) {
				if(pThis->AttachedTag->IsTriggerRepeating()) {
					pTarget->ReplaceTag(pThis->AttachedTag);
				}
			}
		} else {
			// raise some events in case the driver enters
			// a vehicle that needs an Operator
			if(pTarget->AttachedTag) {
				pTarget->AttachedTag->RaiseEvent(TriggerEvent::EnteredBy, pThis, *(CellStruct*)0xA8F1E0, 0, 0);
			}
		}

		// if the hijacker is mind-controlled, free it,
		// too, and attach to the new target. #762
		TechnoClass* controller = pThis->MindControlledBy;
		if(controller) {
			++Unsorted::IKnowWhatImDoing;
			controller->CaptureManager->FreeUnit(pThis);
			--Unsorted::IKnowWhatImDoing;
		}

		// let's make a steal
		pTarget->SetOwningHouse(pThis->Owner, 1);
		pTarget->GotHijacked();
		VocClass::PlayAt(pTypeExt->HijackerEnterSound, &pTarget->Location, 0);

		// remove the driverless-marker
		TechnoExt::ExtData* pDestExt = TechnoExt::ExtMap.Find(pTarget);
		pDestExt->DriverKilled = false;

		// save the hijacker's properties
		if(action == AresAction::Hijack) {
			pTarget->HijackerInfantryType = pType->ArrayIndex;
			pDestExt->HijackerHouse = pThis->Owner;
			pDestExt->HijackerHealth = pThis->Health;
		}

		// hook up the original mind-controller with the target #762
		if(controller) {
			++Unsorted::IKnowWhatImDoing;
			controller->CaptureManager->CaptureUnit(pTarget);
			--Unsorted::IKnowWhatImDoing;
		}

		// reboot the slave manager
		if(pTarget->SlaveManager) {
			pTarget->SlaveManager->ResumeWork();
		}

		// the hijacker enters and closes the door.
		ret = true;

		// only for the drive action: if the target requires an operator,
		// we add the driver to the passengers list instead of deleting it.
		// this does not check passenger count or size limits.
		if(asPassenger) {
			pTarget->AddPassenger(pThis);
			pThis->AbortMotion();
			ret = false;
		}

		pTarget->QueueMission(mission_Guard, true);
	}

	return ret;
}

/*! Gets whether the techno has the ability to cloak itself or is cloaked by others.

	The techno may have the ability to cloak itself, gained the cloaking ability through
	veterancy or it may be under the influence of Cloak Generators.

	\param allowPassive Allow the techno to be cloaked by others.

	\return True, if the techno can cloak, false otherwise.

	\author AlexB
	\date 2012-09-28
*/
bool TechnoExt::ExtData::IsCloakable(bool allowPassive) const
{
	TechnoClass* pThis = this->AttachedToObject;
	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	// object disallowed from cloaking
	if(!pTypeExt->CloakAllowed) {
		return false;
	}

	// check for active cloak
	if(pThis->IsCloakable() || pThis->HasAbility(Abilities::CLOAK)) {
		if(this->CanSelfCloakNow()) {
			return true;
		}
	}

	// if not actively cloakable
	if(allowPassive) {
		// cloak generators ignore everything above ground. this
		// fixes hover units not being affected by cloak.
		if(pThis->GetHeight() > RulesExt::Global()->CloakHeight.Get(RulesClass::Instance->HoverHeight)) {
			return false;
		}

		// search for cloak generators
		CoordStruct crd;
		pThis->GetCoords(&crd);
		CellClass* pCell = MapClass::Instance->GetCellAt(&crd);
		return pCell->CloakGen_InclHouse(pThis->Owner->ArrayIndex);
	}

	return false;
}

/*! Gets whether the techno is allowed to cloak.

	Checks all circumstances that might conflict with the unit cloaking.

	\return True, if the techno is allowed to cloak, false otherwise.

	\author AlexB
	\date 2012-09-28
*/
bool TechnoExt::ExtData::CloakAllowed() const
{
	if(this->CloakDisallowed(true)) {
		return false;
	}

	TechnoClass* pThis = this->AttachedToObject;

	if(pThis->CloakState == CloakState::Cloaked) {
		return false;
	}

	if(!pThis->DiskLaserTimer.Ignorable()) {
		return false;
	}

	if(pThis->Target && pThis->IsCloseEnoughToAttack(pThis->Target)) {
		return false;
	}

	if(pThis->WhatAmI() != BuildingClass::AbsID && pThis->CloakProgress.Value) {
		return false;
	}

	if(!pThis->CloakDelayTimer.Ignorable()) {
		return false;
	}

	if(pThis->LocomotorSource && pThis->AbstractFlags & ABSFLAGS_ISFOOT && ((FootClass*)pThis)->IsAttackedByLocomotor) {
		return false;
	}

	return true;
}

/*! Gets whether the techno is disallowed to cloak.

	Certain features uncloak the techno. If a techno is cloaked and this returns true,
	it should be revealed, because something keeps it from maintaining the cloak.

	\param allowPassive Allow the techno to be cloaked by others.

	\return True, if the techno is disallowed to stay cloaked, false otherwise.

	\author AlexB
	\date 2012-09-28
*/
bool TechnoExt::ExtData::CloakDisallowed(bool allowPassive) const
{
	if(this->IsCloakable(allowPassive)) {
		TechnoClass* pThis = this->AttachedToObject;
		return pThis->IsUnderEMP() || pThis->IsParalyzed()
			|| pThis->IsBeingWarpedOut() || pThis->IsWarpingIn()
			|| !this->CloakSkipTimer.Ignorable();
	}

	return true;
}

/*! Gets whether the techno is allowed to cloak, only accounting for features Ares adds.

	Edit this function to add new features that may prevent units from cloaking.

	\return True, if the techno is allowed to cloak, false otherwise.

	\author AlexB
	\date 2012-09-28
*/
bool TechnoExt::ExtData::CanSelfCloakNow() const
{
	auto pThis = this->AttachedToObject;

	// cloaked and deactivated units are hard to find otherwise
	if(this->DriverKilled || pThis->Deactivated) {
		return false;
	}

	auto pType = pThis->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	if(specific_cast<BuildingTypeClass*>(pType)) {
		if(pExt->CloakPowered && !pThis->IsPowerOnline()) {
			return false;
		}
	}

	if(auto pInf = specific_cast<InfantryClass*>(pThis)) {
		if(pExt->CloakDeployed && !pInf->IsDeployed()) {
			return false;
		}
	}

	// allows cloak
	return true;
}

// =============================
// load/save

void Container<TechnoExt>::Load(TechnoClass *pThis, IStream *pStm) {
	TechnoExt::ExtData* pData = this->LoadKey(pThis, pStm);

	SWIZZLE(pData->Insignia_Image);
}

// =============================
// container hooks

DEFINE_HOOK(6F3260, TechnoClass_CTOR, 5)
{
	GET(TechnoClass*, pItem, ESI);

	TechnoExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(6F4500, TechnoClass_DTOR, 5)
{
	GET(TechnoClass*, pItem, ECX);

	SWStateMachine::InvalidatePointer(pItem);
	//TechnoExt::ExtData *pItemExt = TechnoExt::ExtMap.Find(pItem);
	TechnoExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(70C250, TechnoClass_SaveLoad_Prefix, 8)
DEFINE_HOOK(70BF50, TechnoClass_SaveLoad_Prefix, 5)
{
	GET_STACK(TechnoExt::TT*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	Container<TechnoExt>::SavingObject = pItem;
	Container<TechnoExt>::SavingStream = pStm;

	return 0;
}

DEFINE_HOOK(70C249, TechnoClass_Load_Suffix, 5)
{
	TechnoExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(70C264, TechnoClass_Save_Suffix, 5)
{
	TechnoExt::ExtMap.SaveStatic();
	return 0;
}
