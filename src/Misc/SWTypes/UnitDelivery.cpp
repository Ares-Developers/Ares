#include "UnitDelivery.h"
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

// Replaced my own implementation with AlexB's shown below

//This function doesn't skip any placeable items, no matter how
//they are ordered. Infantry is grouped. Units are moved to the
//center as close as they can. There is an additional fix for a
//glitch: previously, even if a unit could be placed, it would
//have been unloaded again, when it was at index 100.

void UnitDeliveryStateMachine::PlaceUnits()
{
	auto pData = this->FindExtData();

	if(!pData) {
		return;
	}

	// get the house the units will belong to
	auto pOwner = this->Super->Owner;
	if(pData->SW_OwnerHouse == OwnerHouseKind::Civilian) {
		pOwner = HouseClass::FindCivilianSide();
	} else if(pData->SW_OwnerHouse == OwnerHouseKind::Special) {
		pOwner = HouseClass::FindSpecial();
	} else if(pData->SW_OwnerHouse == OwnerHouseKind::Neutral) {
		pOwner = HouseClass::FindNeutral();
	}

	for(size_t i=0; i<pData->SW_Deliverables.size(); ++i) {
		auto Type = pData->SW_Deliverables[i];
		auto Item = abstract_cast<TechnoClass*>(Type->CreateObject(pOwner));
		auto ItemBuilding = abstract_cast<BuildingClass*>(Item);

		if(ItemBuilding && pData->SW_DeliverBuildups) {
			ItemBuilding->QueueMission(mission_Construction, false);
		}

		// 100 cells should be enough for any sane delivery
		bool Placed = false;
		for(auto cellIdx = 0u; cellIdx < 100; ++cellIdx) {
			auto tmpCell = CellSpread::GetCell(cellIdx) + this->Coords;
			if(auto cell = MapClass::Instance->TryGetCellAt(tmpCell)) {
				auto XYZ = cell->GetCoordsWithBridge();

				bool validCell = true;
				if(auto Overlay = OverlayTypeClass::Array->GetItemOrDefault(cell->OverlayTypeIndex)) {
					// disallow placing on rocks, rubble and walls
					validCell = !Overlay->Wall && !Overlay->IsARock && !Overlay->IsRubble;
				}
				if(auto ItemAircraft = abstract_cast<AircraftClass*>(Item)) {
					// for aircraft: cell must be empty: non-water, non-cliff, non-shore, non-anything
					validCell &= !cell->GetContent() && !cell->Tile_Is_Cliff()
						&& !cell->Tile_Is_DestroyableCliff() && !cell->Tile_Is_Shore()
						&& !cell->Tile_Is_Water() && !cell->ContainsBridge();
				}
				if(Type->Naval && validCell) {
					// naval types look stupid on bridges
					validCell = (!cell->ContainsBridge() && cell->LandType != LandType::Road)
						|| Type->SpeedType == SpeedType::Hover;
				}

				if(validCell) {
					Item->OnBridge = cell->ContainsBridge();

					if(Item->Put(XYZ, (cellIdx & 7))) {
						if(ItemBuilding) {
							if(pData->SW_DeliverBuildups) {
								ItemBuilding->DiscoveredBy(this->Super->Owner);
								ItemBuilding->unknown_bool_6DD = 1;
							}
						} else {
							if(Type->BalloonHover || Type->JumpJet) {
								Item->Scatter(CoordStruct::Empty, true, false);
							}
						}
						if(auto pItemData = TechnoExt::ExtMap.Find(Item)) {
							if(!pItemData->IsPowered() || !pItemData->IsOperated()) {
								Item->Deactivate();
								if(ItemBuilding) {
									Item->Owner->RecheckTechTree = true;
								}
							}
						}
						Placed = true;
						break;
					}
				}
			}
		}

		if(!Placed) {
			Item->UnInit();
		}
	}
}
