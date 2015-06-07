#include "UnitDelivery.h"
#include "../../Ext/House/Body.h"
#include "../../Ext/Techno/Body.h"
#include "../../Utilities/TemplateDef.h"

#include <AircraftClass.h>
#include <BuildingClass.h>
#include <CellSpread.h>
#include <HouseClass.h>
#include <OverlayTypeClass.h>

void SW_UnitDelivery::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	pData->SW_AITargetingType = SuperWeaponAITargetingMode::ParaDrop;
}

void SW_UnitDelivery::LoadFromINI(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW, CCINIClass *pINI)
{
	const char * section = pSW->ID;

	if(!pINI->GetSection(section)) {
		return;
	}

	INI_EX exINI(pINI);
	pData->SW_Deliverables.Read(exINI, section, "Deliver.Types");
	pData->SW_DeliverBuildups.Read(exINI, section, "Deliver.Buildups");
	pData->SW_OwnerHouse.Read(exINI, section, "Deliver.Owner");
}

bool SW_UnitDelivery::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	SuperWeaponTypeClass *pSW = pThis->Type;
	SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSW);

	int deferment = pData->SW_Deferment.Get(-1);
	if(deferment < 0) {
		deferment = 20;
	}

	this->newStateMachine(deferment, Coords, pThis);

	return true;
}

void UnitDeliveryStateMachine::Update()
{
	if(this->Finished()) {
		CoordStruct coords = CellClass::Cell2Coord(this->Coords);

		auto pData = this->FindExtData();

		pData->PrintMessage(pData->Message_Activate, this->Super->Owner);

		if(pData->SW_ActivationSound != -1) {
			VocClass::PlayAt(pData->SW_ActivationSound, coords, nullptr);
		}

		this->PlaceUnits();
	}
}

//This function doesn't skip any placeable items, no matter how
//they are ordered. Infantry is grouped. Units are moved to the
//center as close as they can.

void UnitDeliveryStateMachine::PlaceUnits()
{
	auto pData = this->FindExtData();

	if(!pData) {
		return;
	}

	// get the house the units will belong to
	auto pOwner = HouseExt::GetHouseKind(pData->SW_OwnerHouse, false, this->Super->Owner);

	// create an instance of each type and place it
	for(auto pType : pData->SW_Deliverables) {
		auto Item = static_cast<TechnoClass*>(pType->CreateObject(pOwner));
		auto ItemBuilding = abstract_cast<BuildingClass*>(Item);

		// get the best options to search for a place
		short extentX = 1;
		short extentY = 1;
		SpeedType SpeedType = SpeedType::Track;
		MovementZone MovementZone = MovementZone::Normal;
		bool buildable = false;
		bool anywhere = false;

		if(ItemBuilding) {
			auto BuildingType = ItemBuilding->Type;
			extentX = BuildingType->GetFoundationWidth();
			extentY = BuildingType->GetFoundationHeight(true);
			anywhere = BuildingType->PlaceAnywhere;
			if(pType->SpeedType == SpeedType::Float) {
				SpeedType = SpeedType::Float;
			} else {
				buildable = true;
			}
		} else {
			// place aircraft types on ground explicitly
			if(pType->WhatAmI() != AbstractType::AircraftType) {
				SpeedType = pType->SpeedType;
				MovementZone = pType->MovementZone;
			}
		}

		// move the target cell so this object is centered on the actual location
		auto PlaceCoords = this->Coords - CellStruct{extentX / 2, extentY / 2};

		// find a place to put this
		if(!anywhere) {
			int a5 = -1; // usually MapClass::CanLocationBeReached call. see how far we can get without it
			PlaceCoords = MapClass::Instance->Pathfinding_Find(PlaceCoords,
				SpeedType, a5, MovementZone, false, extentX, extentY, true,
				false, false, false, CellStruct::Empty, false, buildable);
		}

		if(auto pCell = MapClass::Instance->TryGetCellAt(PlaceCoords)) {
			Item->OnBridge = pCell->ContainsBridge();

			// set the appropriate mission
			if(ItemBuilding && pData->SW_DeliverBuildups) {
				ItemBuilding->QueueMission(Mission::Construction, false);
			} else {
				// only computer units can hunt
				auto Guard = ItemBuilding || pOwner->ControlledByHuman();
				auto Mission = Guard ? Mission::Guard : Mission::Hunt;
				Item->QueueMission(Mission, false);
			}

			// place and set up
			auto XYZ = pCell->GetCoordsWithBridge();
			if(Item->Put(XYZ, (MapClass::GetCellIndex(pCell->MapCoords) & 7u))) {
				if(ItemBuilding) {
					if(pData->SW_DeliverBuildups) {
						ItemBuilding->DiscoveredBy(this->Super->Owner);
						ItemBuilding->unknown_bool_6DD = 1;
					}
				} else if(pType->BalloonHover || pType->JumpJet) {
					Item->Scatter(CoordStruct::Empty, true, false);
				}

				if(auto pItemData = TechnoExt::ExtMap.Find(Item)) {
					if(!pItemData->IsPowered() || !pItemData->IsOperated()) {
						Item->Deactivate();
						if(ItemBuilding) {
							Item->Owner->RecheckTechTree = true;
						}
					}
				}
			} else {
				Item->UnInit();
			}
		}
	}
}
