#include "Body.h"
#include "../TechnoType/Body.h"
#include "../../Misc/SWTypes.h"
#include "../../Misc/PoweredUnitClass.h"

#include <HouseClass.h>
#include <BuildingClass.h>
#include <GeneralStructures.h>
#include <Helpers/Template.h>

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
//	RETZ_UNLESS(pData->Survivors_PilotChance || pData->Survivors_PassengerChance);
	RETZ_UNLESS(!pSelfData->Survivors_Done);

	CoordStruct loc = pThis->Location;

	int chance = pData->Survivors_PilotChance.BindTo(pThis)->Get();
	// remove check for Crewed if it is accounted for outside of this function
	// checks if Crewed=yes is set and there is a chance pilots survive, and, if yes...
	// ...attempts to spawn one Survivors_PilotCount times

	if(Type->Crewed && chance && !IgnoreDefenses) {
		for(int i = 0; i < pData->Survivors_PilotCount; ++i) {
			if(ScenarioClass::Instance->Random.RandomRanged(1, 100) <= chance) {
				signed int idx = pOwner->SideIndex;
				auto Pilots = &pData->Survivors_Pilots;
				if(Pilots->ValidIndex(idx)) {
					if(InfantryTypeClass *PilotType = Pilots->GetItem(idx)) {
						InfantryClass *Pilot = reinterpret_cast<InfantryClass *>(PilotType->CreateObject(pOwner));
						// HouseClass::CreateParadrop does this when building passengers for a paradrop... it might be a wise thing to mimic!
						Pilot->Remove();

						Pilot->Health = (PilotType->Strength / 2);
						Pilot->Veterancy.Veterancy = pThis->Veterancy.Veterancy;
						CoordStruct destLoc, tmpLoc = loc;
						CellStruct tmpCoords = CellSpread::GetCell(ScenarioClass::Instance->Random.RandomRanged(0, 7));

						tmpLoc.X += tmpCoords.X * 144;
						tmpLoc.Y += tmpCoords.Y * 144;

						CellClass * tmpCell = MapClass::Instance->GetCellAt(&tmpLoc);

						tmpCell->FindInfantrySubposition(&destLoc, &tmpLoc, 0, 0, 0);

						destLoc.Z = loc.Z;

						if(!TechnoExt::EjectSurvivor(Pilot, &destLoc, Select)) {
							Pilot->RegisterDestruction(pKiller); //(TechnoClass *)R->get_StackVar32(0x54));
							GAME_DEALLOC(Pilot);
						}
					}
				}
			}
		}
	}

	chance = pData->Survivors_PassengerChance.BindTo(pThis)->Get();

	while(pThis->Passengers.FirstPassenger) {
		bool toDelete = 1;
		FootClass *passenger = pThis->RemoveFirstPassenger();
		bool toSpawn = false;
		if(chance > 0) {
			toSpawn = ScenarioClass::Instance->Random.RandomRanged(1, 100) <= chance;
		} else if(chance == -1 && pThis->WhatAmI() == abs_Unit) {
			int occupation = passenger->IsCellOccupied(pThis->GetCell(), -1, -1, 0, 1);
			toSpawn = (occupation == 0 || occupation == 2);
		}
		if(toSpawn && !IgnoreDefenses) {
			CoordStruct destLoc, tmpLoc = loc;
			CellStruct tmpCoords = CellSpread::GetCell(ScenarioClass::Instance->Random.RandomRanged(0, 7));

			tmpLoc.X += tmpCoords.X * 128;
			tmpLoc.Y += tmpCoords.Y * 128;

			CellClass * tmpCell = MapClass::Instance->GetCellAt(&tmpLoc);

			tmpCell->FindInfantrySubposition(&destLoc, &tmpLoc, 0, 0, 0);

			destLoc.Z = loc.Z;

			toDelete = !TechnoExt::EjectSurvivor(passenger, &destLoc, Select);
		}
		if(toDelete) {
			passenger->RegisterDestruction(pKiller); //(TechnoClass *)R->get_StackVar32(0x54));
			passenger->UnInit();
		}
	}

	pSelfData->Survivors_Done = 1;
}
/**
	\param Survivor Passenger to eject
	\param loc Where to put the passenger
	\param Select Whether to select the Passenger afterwards
*/
bool TechnoExt::EjectSurvivor(FootClass *Survivor, CoordStruct *loc, bool Select)
{
	bool success;
	CoordStruct tmpCoords;
	CellClass * pCell = MapClass::Instance->GetCellAt(loc);
	pCell->GetCoordsWithBridge(&tmpCoords);
	Survivor->OnBridge = pCell->ContainsBridge();

	int floorZ = tmpCoords.Z;
	bool Chuted(false);
	if(loc->Z - floorZ > 208) {
		success = Survivor->SpawnParachuted(loc);
		Chuted = true;
	} else {
		loc->Z = floorZ;
		success = Survivor->Put(loc, ScenarioClass::Instance->Random.RandomRanged(0, 7));
	}
	RET_UNLESS(success);
	Survivor->Transporter = NULL;


	Survivor->LastMapCoords = pCell->MapCoords;
	// don't ask, don't tell (ripped from 0x51A84F)
	if(Chuted) {
		bool scat(Survivor->OnBridge);
		DWORD *flagsOfSomeKind = reinterpret_cast<DWORD *>(pCell->unknown_121 + 3);
		if((flagsOfSomeKind[scat] & 0x1C) == 0x1C) {
			pCell->ScatterContent(0xA8F200, 1, 1, scat);
		}
	} else {
		Survivor->Scatter(0xB1CFE8, 1, 0);
		Survivor->QueueMission(Survivor->Owner->ControlledByHuman() ? mission_Guard : mission_Hunt, 0);
	}
	Survivor->unknown_bool_690 = Survivor->unknown_bool_691 = false;


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

	short limit = (howMany < 0 || howMany > pThis->Passengers.NumPassengers) ? pThis->Passengers.NumPassengers : howMany;

	for(short i = 0; (i < limit) && pThis->Passengers.FirstPassenger; ++i) {
		CoordStruct destination;
		TechnoExt::GetPutLocation(pThis->Location, destination);

		FootClass *passenger = pThis->RemoveFirstPassenger();
		if(!passenger->Put(&destination, ScenarioClass::Instance->Random.RandomRanged(0, 7))) {
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
	\author Renegade
	\date 27.05.2010
*/
void TechnoExt::GetPutLocation(CoordStruct const &current, CoordStruct &target) {
	// this whole thing does not at all account for cells which are completely occupied.
	CoordStruct tmpLoc = current;
	CellStruct tmpCoords = CellSpread::GetCell(ScenarioClass::Instance->Random.RandomRanged(0, 7));

	tmpLoc.X += tmpCoords.X * 128;
	tmpLoc.Y += tmpCoords.Y * 128;

	CellClass * tmpCell = MapClass::Instance->GetCellAt(&tmpLoc);

	tmpCell->FindInfantrySubposition(&target, &tmpLoc, 0, 0, 0);

	target.Z = current.Z;
	return;
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
	DWORD Facing;
	this->AttachedToObject->Facing.GetFacing(&Facing);
	WORD F = (WORD)Facing;
	return (F >> (16 - countFrames));
}

bool TechnoExt::ExtData::DrawVisualFX() {
	TechnoClass * Object = this->AttachedToObject;
	if(Object->VisualCharacter(true, Object->Owner) == VisualType::Normal) {
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

void Container<TechnoExt>::InvalidatePointer(void *ptr) {
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
	if(Object->IsCellOccupied(MyCell, -1, -1, 0, 0)) {
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
		FacingStruct::Facet Facing = {0, 0, 0};
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

void TechnoExt::TransferMindControl(TechnoClass *From, TechnoClass *To) {
	if(auto Controller = From->MindControlledBy) {
		++Unsorted::IKnowWhatImDoing;
		if(auto Manager = Controller->CaptureManager) { // shouldn't be necessary, but WW uses it...
			bool FoundNode(false);
			for(int i = 0; i < Manager->ControlNodes.Count; ++i) {
				auto Node = Manager->ControlNodes[i];
				if(Node->Unit == From) {
					Node->Unit = To;
					To->SetOwningHouse(From->GetOwningHouse(), 0);
					To->MindControlledBy = Controller;
					From->MindControlledBy = NULL;
					FoundNode = true;
					break;
				}
			}
			if(!FoundNode) { // say the link was broken beforehand, by inconsistently ordered code...
				Debug::Log("Transferring %s's MC link from %s to %s manually, problems might occur...\n"
					, Controller->get_ID(), From->get_ID(), To->get_ID());
				Manager->CaptureUnit(To);
			}
		}
		--Unsorted::IKnowWhatImDoing;
	} else if(auto MCHouse = From->MindControlledByHouse) {
		To->MindControlledByHouse = MCHouse;
		From->MindControlledByHouse = NULL;
	}
	if(auto Anim = From->MindControlRingAnim) {
		auto ToAnim = &To->MindControlRingAnim;
		if(*ToAnim) {
			(*ToAnim)->TimeToDie = 1;
		}
		*ToAnim = Anim;
		Anim->SetOwnerObject(To);
	}
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
		auto FromExt = TechnoExt::ExtMap.Find(From);
		auto ToExt = TechnoExt::ExtMap.Find(To);
		ToExt->AttachedEffects.Clear();
		if (FromExt->AttachedEffects.Count != 0) {
			
			// while recreation itself isn't the best idea, less hassle and more reliable
			// list gets intact in the end
			for (int i=0; i<FromExt->AttachedEffects.Count; i++){
				FromExt->AttachedEffects.GetItem(i)->Type->Attach(To, FromExt->AttachedEffects.GetItem(i)->ActualDuration);
				FromExt->AttachedEffects.GetItem(i)->Destroy();
				delete FromExt->AttachedEffects.GetItem(i);
			
			}
			FromExt->AttachedEffects.Clear();
			FromExt->AttachedTechnoEffect_isset=false;
			TechnoExt::RecalculateStats(To);
		}
	
}

/*! This function recalculates the stats modifiable by crates and update them (aimed for request #255)
	\todo code that crate effects not get ignored
	\author Graion Dilach
	\date 2011-10-12
*/

void TechnoExt::RecalculateStats(TechnoClass *pTechno){
	auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
	double Firepower = 1, Armor = 1, Speed = 1; //if there's hooks for crate-stuff, they could be the base for this
	bool Cloak = TechnoExt::CanICloakByDefault(pTechno);

	Debug::Log("[AttachEffect]Recalculating stats of %s...\n", pTechno->get_ID());

	if (pTechnoExt->AttachedEffects.Count > 0){		//QDB #666... -_- 
		for (int i=0; i < pTechnoExt->AttachedEffects.Count; i++){

			Firepower *= pTechnoExt->AttachedEffects.GetItem(i)->Type->FirepowerMultiplier;
			Speed *= pTechnoExt->AttachedEffects.GetItem(i)->Type->SpeedMultiplier;
			Armor *= pTechnoExt->AttachedEffects.GetItem(i)->Type->ArmorMultiplier;
			Cloak = Cloak || pTechnoExt->AttachedEffects.GetItem(i)->Type->Cloakable;
		
		}
	}

	pTechno->FirepowerMultiplier = Firepower;
	pTechno->ArmorMultiplier = Armor;

	/*if (pTechno->Cloakable && !Cloak){
		pTechno->Uncloak(false);
		pTechno->Cloakable = Cloak;
		
	} else {*/
		pTechno->Cloakable = Cloak;
	//}

	if(FootClass *Foot = generic_cast<FootClass *>(pTechno)) {
		Foot->SpeedMultiplier = Speed;
	}

	Debug::Log("[AttachEffect]Calculation was successful.\n", pTechno->get_ID());
}

/*! This function calculates that the unit wold be cloaked by default
	\author Graion Dilach
	\date 2011-10-16
*/


bool TechnoExt::CanICloakByDefault(TechnoClass *pTechno){
	Debug::Log("[AttachEffect]Can %s cloak by default?\n", pTechno->get_ID());
	if (pTechno->GetTechnoType()->Cloakable){
		return true;
	}

	if (pTechno->Veterancy.IsVeteran() && pTechno->GetTechnoType()->VeteranAbilities.CLOAK){
		return true;
	}

	if (pTechno->Veterancy.IsElite() && (pTechno->GetTechnoType()->VeteranAbilities.CLOAK || pTechno->GetTechnoType()->EliteAbilities.CLOAK)){
		return true;
	}

	return false;
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
	TechnoExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(70BF50, TechnoClass_SaveLoad_Prefix, 5)
DEFINE_HOOK_AGAIN(70C250, TechnoClass_SaveLoad_Prefix, 8)
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
