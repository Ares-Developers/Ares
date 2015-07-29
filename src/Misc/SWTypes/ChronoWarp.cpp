#include "ChronoWarp.h"
#include "../../Ares.h"
#include "../../Ext/House/Body.h"
#include "../../Ext/Building/Body.h"
#include "../../Ext/TechnoType/Body.h"
#include "../../Utilities/Helpers.Alex.h"
#include "../../Utilities/TemplateDef.h"

#include <AnimClass.h>
#include <LocomotionClass.h>
#include <BulletClass.h>
#include <LightSourceClass.h>
#include <RadarEventClass.h>

bool SW_ChronoWarp::HandlesType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::ChronoWarp);
}

SuperWeaponFlags SW_ChronoWarp::Flags() const
{
	return SuperWeaponFlags::NoAnim | SuperWeaponFlags::NoEvent | SuperWeaponFlags::PostClick;
}

void SW_ChronoWarp::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// Every other thing will be read in the ChronoSphere.
	
	pData->SW_Cursor = MouseCursor::GetCursor(MouseCursorType::Chronosphere);
}

bool SW_ChronoWarp::Activate(SuperClass* pThis, const CellStruct &Coords, bool IsPlayer)
{
	// get the previous super weapon
	SuperClass* pSource = nullptr;
	if(auto const pExt = HouseExt::ExtMap.Find(pThis->Owner)) {
		pSource = pThis->Owner->Supers.GetItemOrDefault(pExt->SWLastIndex);
	}

	// use source super weapon properties
	if(!pSource || pSource->Type->Type != SuperWeaponType::ChronoSphere) {
		// idiots at work.
		Debug::Log("ChronoWarp typed super weapon triggered as standalone. Use ChronoSphere instead.\n");
		return false;
	}

	Debug::Log("[ChronoWarp::Activate] Launching %s with %s as source.\n", pThis->Type->ID, pSource->Type->ID);
	auto const pData = SWTypeExt::ExtMap.Find(pSource->Type);

	// add radar events for source and target
	if(pData->SW_RadarEvent) {
		RadarEventClass::Create(RadarEventType::SuperweaponActivated, pSource->ChronoMapCoords);
		RadarEventClass::Create(RadarEventType::SuperweaponActivated, Coords);
	}

	// update animations
	SWTypeExt::ClearChronoAnim(pThis);

	if(auto const pAnimType = pData->Chronosphere_BlastSrc.Get(RulesClass::Instance->ChronoBlast)) {
		auto const pCellSource = MapClass::Instance->GetCellAt(pSource->ChronoMapCoords);
		auto coordsSource = pCellSource->GetCoordsWithBridge();
		coordsSource.Z += pData->SW_AnimHeight;
		GameCreate<AnimClass>(pAnimType, coordsSource);
	}

	if(auto const pAnimType = pData->Chronosphere_BlastDest.Get(RulesClass::Instance->ChronoBlastDest)) {
		auto const pCellTarget = MapClass::Instance->GetCellAt(Coords);
		auto coordsTarget = pCellTarget->GetCoordsWithBridge();
		coordsTarget.Z += pData->SW_AnimHeight;
		GameCreate<AnimClass>(pAnimType, coordsTarget);
	}

	DynamicVectorClass<ChronoWarpStateMachine::ChronoWarpContainer> RegisteredBuildings;

	auto Chronoport = [pThis, pData, pSource, Coords, &RegisteredBuildings](TechnoClass* const pTechno) -> bool {
		// is this thing affected at all?
		if(!pData->IsHouseAffected(pThis->Owner, pTechno->Owner)) {
			return true;
		}

		if(!pData->IsTechnoAffected(pTechno)) {
			return true;
		}

		auto const pType = pTechno->GetTechnoType();
		auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

		// can this techno be chronoshifted?
		if(!pExt->Chronoshift_Allow) {
			return true;
		}

		// differentiate between buildings and vehicle-type buildings
		bool IsVehicle = false;
		if(auto const pBld = abstract_cast<BuildingClass*>(pTechno)) {
			// always ignore bridge repair huts
			if(pBld->Type->BridgeRepairHut) {
				return true;
			}

			// use "smart" detection of vehicular building types?
			if(pData->Chronosphere_ReconsiderBuildings) {
				IsVehicle = pExt->Chronoshift_IsVehicle;
			}

			// always let undeployers pass if all undeployers are affected
			if(!pData->Chronosphere_AffectUndeployable || !pBld->Type->UndeploysInto) {
				// we don't handle buildings and this is a real one
				if(!IsVehicle && !pData->Chronosphere_AffectBuildings) {
					return true;
				}

				// this is a vehicle in disguise and we don't handle them
				if(IsVehicle && !(pData->SW_AffectsTarget & SuperWeaponTarget::Unit)) {
					return true;
				}
			} else {
				// force vehicle placement rules
				IsVehicle = true;
			}
		}

		// some quick exclusion criteria
		if(pTechno->IsImmobilized
			|| pTechno->IsInAir()
			|| pTechno->IsBeingWarpedOut()
			|| pTechno->IsWarpingIn())
		{
				return true;
		}

		// unwarpable unit
		if(!pType->Warpable && !pData->Chronosphere_AffectUnwarpable) {
			return true;
		}

		// iron curtained units
		if(pTechno->IsIronCurtained() && !pData->Chronosphere_AffectIronCurtain) {
			return true;
		}

		// if this is a newly produced unit that still is in its
		// weapons factory, this skips it.
		if(pTechno->WhatAmI() == AbstractType::Unit) {
			if(auto const pLinkBld = abstract_cast<BuildingClass*>(pTechno->GetNthLink(0))) {
				if(pLinkBld->Type->WeaponsFactory) {
					if(MapClass::Instance->GetCellAt(pTechno->Location)->GetBuilding() == pLinkBld) {
						return true;
					}
				}
			}
		}

		// behind this point, the units are affected.

		// organics are destroyed as long as they aren't teleporters
		if(pType->Organic && pData->Chronosphere_KillOrganic) {
			if(!pType->Teleporter || pData->Chronosphere_KillTeleporters) {
				int strength = pType->Strength;
				pTechno->ReceiveDamage(&strength, 0, RulesClass::Instance->C4Warhead,
					nullptr, true, false, pSource->Owner);
				return true;
			}
		}

		// remove squids. terror drones stay inside.
		if(auto const pFoot = abstract_cast<FootClass*>(pTechno)) {
			if(auto const pSquid = pFoot->ParasiteEatingMe) {
				if(pType->Naval) {
					if(auto const pSquidParasite = pSquid->ParasiteImUsing) {
						pSquidParasite->SuppressionTimer.Start(500);
						pSquidParasite->ExitUnit();
					}
				}
			}
		}

		// disconnect bunker and contents
		if(pTechno->BunkerLinkedItem) {
			if(auto const pBunkerLink = abstract_cast<BuildingClass*>(pTechno->BunkerLinkedItem)) {
				// unit will be destroyed or chronoported. in every case the bunker will be empty.
				pBunkerLink->ClearBunker();
			} else if(auto const pBunker = abstract_cast<BuildingClass*>(pTechno)) {
				// the bunker leaves...
				pBunker->UnloadBunker();
				pBunker->EmptyBunker();
			}
		}

		// building specific preparations
		if(auto const pBld = abstract_cast<BuildingClass*>(pTechno)) {
			// tell all linked units to get off
			pBld->SendToEachLink(RadioCommand::RequestRedraw);
			pBld->SendToEachLink(RadioCommand::NotifyUnlink);

			// destroy the building light source
			if(pBld->LightSource) {
				pBld->LightSource->Deactivate();
				GameDelete(pBld->LightSource);
				pBld->LightSource = nullptr;
			}

			// shut down cloak generation
			if(pBld->Type->CloakGenerator && pBld->CloakRadius) {
				pBld->HasCloakingData = -1;
				pBld->NeedsRedraw = true;
				pBld->CloakRadius = 1;
				pBld->UpdateCloak();
			}
		}

		// get the cells and coordinates
		auto const coordsUnitSource = pTechno->GetCoords();
		auto const cellUnitTarget = pTechno->GetCell()->MapCoords - pSource->ChronoMapCoords + Coords;

		if(auto const pFoot = abstract_cast<FootClass*>(pTechno)) {
			// move the unit to the new position
			auto const pCellUnitTarget = MapClass::Instance->GetCellAt(cellUnitTarget);
			auto const offset = Coords - pSource->ChronoMapCoords;
			auto const coordsUnitTarget = pCellUnitTarget->FixHeight(
				coordsUnitSource + CoordStruct{offset.X * 256, offset.Y * 256, 0});

			// clean up the unit's current cell
			pFoot->Locomotor->Mark_All_Occupation_Bits(0);
			pFoot->Locomotor->Force_Track(-1, coordsUnitSource);
			pFoot->MarkAllOccupationBits(coordsUnitSource);
			pFoot->FrozenStill = true;

			// piggyback the original locomotor onto a new teleport locomotor and
			// use that for the next move order.
			LocomotionClass::ChangeLocomotorTo(pFoot, LocomotionClass::CLSIDs::Teleport);

			// order unit to move to target location
			pFoot->IsImmobilized = true;
			pFoot->ChronoDestCoords = coordsUnitTarget;
			pFoot->SendToEachLink(RadioCommand::NotifyUnlink);
			pFoot->ChronoWarpedByHouse = pThis->Owner;
			pFoot->SetDestination(pCellUnitTarget, true);
		} else if(auto const pBld = abstract_cast<BuildingClass*>(pTechno)) {
			// begin the building chronoshift
			pBld->BecomeUntargetable();
			for(auto const& pBullet : *BulletClass::Array) {
				if(pBullet->Target == pBld) {
					pBullet->LoseTarget();
				}
			}

			// the buidling counts as warped until it reappears
			pBld->BeingWarpedOut = true;
			pBld->Owner->RecheckTechTree = true;
			pBld->Owner->RecheckPower = true;
			pBld->DisableTemporal();
			pBld->UpdatePlacement(PlacementType::Redraw);

			auto const pBldExt = BuildingExt::ExtMap.Find(pBld);
			pBldExt->AboutToChronoshift = true;

			// register for chronoshift
			ChronoWarpStateMachine::ChronoWarpContainer Container(pBld, cellUnitTarget, pBld->Location, IsVehicle);
			RegisteredBuildings.AddItem(Container);
		}

		return true;
	};

	// collect every techno in this range only once. apply the Chronosphere.
	auto range = pData->GetRange();
	Helpers::Alex::DistinctCollector<TechnoClass*> items;
	Helpers::Alex::for_each_in_rect_or_range<TechnoClass>(pSource->ChronoMapCoords, range.WidthOrRange, range.Height, items);
	items.for_each(Chronoport);

	if(RegisteredBuildings.Count) {
		this->newStateMachine(RulesClass::Instance->ChronoDelay + 1, Coords, pSource, this, std::move(RegisteredBuildings));
	}

	return true;
}

void ChronoWarpStateMachine::Update()
{
	int passed = this->TimePassed();

	if(passed == 1) {
		// redraw all buildings
		for(auto const& item : this->Buildings) {
			if(item.building) {
				item.building->UpdatePlacement(PlacementType::Redraw);
			}
		}
	} else if(passed == this->Duration - 1) {
		// move the array so items can't get invalidated
		auto const buildings = std::move(this->Buildings);

		// remove all buildings from the map at once
		for(auto const& item : buildings) {
			item.building->Remove();
			item.building->ActuallyPlacedOnMap = false;
		}

		// bring back all buildings
		for(auto const& item : buildings) {
			auto const pBld = item.building;

			if(!pBld || pBld->TemporalTargetingMe) {
				continue;
			}

			// use some logic to place this unit on some other
			// cell if the target cell is occupied. this emulates
			// the behavior of other units.
			auto success = false;
			for(CellSpreadEnumerator it(item.isVehicle ? 10u : 0u); it; ++it) {
				auto const cellNew = item.target + *it;

				if(pBld->Type->CanCreateHere(cellNew, nullptr)) {
					auto const pNewCell = MapClass::Instance->GetCellAt(cellNew);
					auto const coordsNew = pNewCell->GetCoordsWithBridge();

					if(pBld->Put(coordsNew, Direction::North)) {
						success = true;
						break;
					}
				}
			}

			if(!success) {
				// put it back where it was
				++Unsorted::IKnowWhatImDoing;
				pBld->Put(item.origin, Direction::North);
				pBld->Place(false);
				--Unsorted::IKnowWhatImDoing;
			}

			// chronoshift ends
			pBld->BeingWarpedOut = false;
			pBld->Owner->RecheckPower = true;
			pBld->Owner->RecheckTechTree = true;
			pBld->EnableTemporal();
			pBld->UpdatePlacement(PlacementType::Redraw);

			auto const pBldExt = BuildingExt::ExtMap.Find(pBld);
			pBldExt->AboutToChronoshift = false;

			if(!success) {
				// destroy (buildings only if they are supposed to)
				auto const pExt = this->FindExtData();

				if(item.isVehicle || pExt->Chronosphere_BlowUnplaceable) {
					int damage = pBld->Type->Strength;
					pBld->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead,
						nullptr, true, true, this->Super->Owner);
				}
			}
		}
	} else if(passed == this->Duration) {
		auto const pOwner = this->Super->Owner;
		pOwner->RecheckPower = true;
		pOwner->RecheckTechTree = true;
		pOwner->RecheckRadar = true;
	}
}

void ChronoWarpStateMachine::InvalidatePointer(void *ptr, bool remove)
{
	if(remove) {
		for(int i = 0; i < this->Buildings.Count; ++i) {
			if(this->Buildings[i].building == ptr) {
				this->Buildings.RemoveItem(i);
				break;
			}
		}
	}
}

bool ChronoWarpStateMachine::Load(AresStreamReader &Stm, bool RegisterForChange) {
	return SWStateMachine::Load(Stm, RegisterForChange)
		&& Stm
		.Process(this->Buildings, RegisterForChange)
		.Process(this->Duration, RegisterForChange)
		.Success();
}

bool ChronoWarpStateMachine::Save(AresStreamWriter &Stm) const {
	return SWStateMachine::Save(Stm)
		&& Stm
		.Process(this->Buildings)
		.Process(this->Duration)
		.Success();
}
