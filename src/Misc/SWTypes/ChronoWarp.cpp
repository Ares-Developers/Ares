#include "ChronoWarp.h"
#include "../../Ares.h"
#include "../../Ext/House/Body.h"
#include "../../Ext/TechnoType/Body.h"
#include "../../Utilities/Helpers.Alex.h"

#include <LocomotionClass.h>
#include <BulletClass.h>

bool SW_ChronoWarp::HandlesType(int type)
{
	return (type == SuperWeaponType::ChronoWarp);
}

SuperWeaponFlags::Value SW_ChronoWarp::Flags()
{
	return SuperWeaponFlags::NoAnim | SuperWeaponFlags::NoEvent | SuperWeaponFlags::PostClick;
}

void SW_ChronoWarp::Initialize(SWTypeExt::ExtData *pData, SuperWeaponTypeClass *pSW)
{
	// Every other thing will be read in the ChronoSphere.
	
	pData->SW_Cursor = MouseCursor::First[MouseCursorType::Chronosphere];
}

bool SW_ChronoWarp::Launch(SuperClass* pThis, CellStruct* pCoords, byte IsPlayer)
{
	// get the previous super weapon
	SuperClass* pSource = NULL;
	if(HouseExt::ExtData *pExt = HouseExt::ExtMap.Find(pThis->Owner)) {
		if(pThis->Owner->Supers.ValidIndex(pExt->SWLastIndex)) {
			pSource = pThis->Owner->Supers.GetItem(pExt->SWLastIndex);
		}
	}

	// use source super weapon properties
	if(pSource && (pSource->Type->Type == SuperWeaponType::ChronoSphere)) {
		if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSource->Type)) {
			Debug::Log("[ChronoWarp::Launch] Launching %s with %s as source.\n", pThis->Type->ID, pSource->Type->ID);

			// add radar events for source and target
			if(pData->SW_RadarEvent.Get()) {
				RadarEventClass::Create(RADAREVENT_SUPERWEAPONLAUNCHED, pSource->ChronoMapCoords);
				RadarEventClass::Create(RADAREVENT_SUPERWEAPONLAUNCHED, *pCoords);
			}

			// cell and coords calculations
			CellClass *pCellSource = MapClass::Instance->GetCellAt(&pSource->ChronoMapCoords);
			CellClass *pCellTarget = MapClass::Instance->GetCellAt(pCoords);

			CoordStruct coordsSource;
			pCellSource->GetCoordsWithBridge(&coordsSource);
			coordsSource.Z += pData->SW_AnimHeight;

			CoordStruct coordsTarget;
			pCellTarget->GetCoordsWithBridge(&coordsTarget);
			coordsTarget.Z += pData->SW_AnimHeight;

			// Update animations
			SWTypeExt::ClearChronoAnim(pThis);

			AnimClass *pAnim = NULL;
			GAME_ALLOC(AnimClass, pAnim, pData->Chronosphere_BlastSrc.Get(), &coordsSource);
			GAME_ALLOC(AnimClass, pAnim, pData->Chronosphere_BlastDest.Get(), &coordsTarget);

			DynamicVectorClass<ChronoWarpStateMachine::ChronoWarpContainer> RegisteredBuildings;

			auto Chronoport = [&](ObjectClass* pObj) -> bool {
				// sanity checks
				TechnoClass *pTechno = generic_cast<TechnoClass*>(pObj);
				if(!pTechno) {
					return true;
				}

				// is this thing affected at all?
				if(!pData->IsHouseAffected(pThis->Owner, pTechno->Owner)) {
					return true;
				}

				if(!pData->IsTechnoAffected(pTechno)) {
					return true;
				}

				TechnoTypeClass *pType = pTechno->GetTechnoType();
				TechnoTypeExt::ExtData *pExt = TechnoTypeExt::ExtMap.Find(pType);

				// can this techno be chronoshifted?
				if(!pExt->Chronoshift_Allow) {
					return true;
				}

				// short way out for buildings
				bool IsVehicle = pExt->Chronoshift_IsVehicle.Get();
				if(BuildingClass* pBld = specific_cast<BuildingClass*>(pObj)) {
					bool Handle = false;
					if(pBld->Type->UndeploysInto) {
						IsVehicle = true;

						if(pData->Chronosphere_AffectUndeployable.Get()) {
							// this thing undeploys and we handle all undeployers
							Handle = true;
						}
					}

					// if we don't handle buildings, get out now.
					if(!Handle && !pData->Chronosphere_AffectBuildings.Get()) {
						return true;
					}

					// if this is a vehicle and we don't handle them, get out
					if(IsVehicle && !(pData->SW_AffectsTarget & SuperWeaponTarget::Unit)) {
						return true;
					}
				}

				// some quick exclusion criteria
				if(pTechno->IsImmobilized || pTechno->IsInAir()
					|| pTechno->IsBeingWarpedOut() || pTechno->IsWarpingIn()) {
						return true;
				}

				// unwarpable unit
				if(!pType->Warpable && !pData->Chronosphere_AffectUnwarpable.Get()) {
					return true;
				}

				// iron curtained units
				if(pTechno->IsIronCurtained() && !pData->Chronosphere_AffectIronCurtain.Get()) {
					return true;
				}

				// if this is a newly produced unit that still is in its
				// weapons factory, this skips it.
				if(pTechno->WhatAmI() == abs_Unit) {
					TechnoClass* pLink = pTechno->GetNthLink(0);
					if(pLink) {
						if(BuildingClass* pLinkBld = specific_cast<BuildingClass*>(pLink)) {
							if(pLinkBld->Type->WeaponsFactory) {
								if(MapClass::Instance->GetCellAt(&pTechno->Location)->GetBuilding() == pLinkBld) {
									return true;
								}
							}
						}
					}
				}

				// behind this point, the units are affected.

				// organics are destroyed as long as they aren't teleporters
				if(pType->Organic && pData->Chronosphere_KillOrganic.Get()) {
					if(!pType->Teleporter || pData->Chronosphere_KillTeleporters.Get()) {
						pTechno->ReceiveDamage(&pType->Strength, 0,
							RulesClass::Instance->C4Warhead, NULL, true, false, pSource->Owner);
						return true;
					}
				}

				// remove squids. terror drones stay inside.
				if(FootClass *pFoot = generic_cast<FootClass*>(pObj)) {
					if(FootClass *pSquid = pFoot->ParasiteEatingMe) {
						if(pType->Naval) {
							if(ParasiteClass *pSquidParasite = pSquid->ParasiteImUsing) {
								pSquidParasite->SuppressionTimer.Start(500);
								pSquidParasite->ExitUnit();
							}
						}
					}
				}

				// unit will be destroyed or chronoported. in every case the bunker will be empty.
				if(pTechno->BunkerLinkedItem) {
					if(BuildingClass *pBunkerLink = specific_cast<BuildingClass*>(pTechno->BunkerLinkedItem)) {
						pBunkerLink->ClearBunker();
					}
				}

				// get the cells and coordinates
				CoordStruct coordsUnitSource;
				pTechno->GetCoords(&coordsUnitSource);
				CoordStruct coordsUnitTarget = coordsUnitSource;
				CellStruct cellUnitTarget = pTechno->GetCell()->MapCoords - pSource->ChronoMapCoords + *pCoords;
				CellClass* pCellUnitTarget = MapClass::Instance->GetCellAt(&cellUnitTarget);
				
				// move the unit to the new position
				coordsUnitTarget.X = coordsUnitSource.X + (pCoords->X - pSource->ChronoMapCoords.X) * 256;
				coordsUnitTarget.Y = coordsUnitSource.Y + (pCoords->Y - pSource->ChronoMapCoords.Y) * 256;
				pCellUnitTarget->FixHeight(&coordsUnitTarget);

				if(FootClass *pFoot = generic_cast<FootClass*>(pObj)) {
					// clean up the unit's current cell
					pFoot->Locomotor->Mark_All_Occupation_Bits(0);
					pFoot->Locomotor->Force_Track(-1, coordsUnitSource);
					pFoot->MarkAllOccupationBits(&coordsUnitSource);
					pFoot->FrozenStill = true;
				
					// piggyback the original locomotor onto a new teleport locomotor and
					// use that for the next move order.
					LocomotionClass::ChangeLocomotorTo(pFoot, &LocomotionClass::CLSIDs::Teleport);

					// order unit to move to target location
					pFoot->IsImmobilized = true;
					pFoot->ChronoDestCoords = coordsUnitTarget;
					pFoot->SendToEachLink(rc_Exit);
					pFoot->ChronoWarpedByHouse = pThis->Owner;
					pFoot->SetDestination(pCellUnitTarget, true);
				} else if (BuildingClass *pBld = generic_cast<BuildingClass*>(pObj)) {
					// begin the building chronoshift
					pBld->BecomeUntargetable();
					for(int i = 0; i<BulletClass::Array->Count; ++i) {
						BulletClass* pBullet = BulletClass::Array->GetItem(i);
						if(pBullet->Target == pBld) {
							pBullet->LoseTarget();
						}
					}

					// the buidling counts as warped until it reappears
					pBld->BeingWarpedOut = true;
					pBld->Owner->ShouldRecheckTechTree = true;
					pBld->Owner->PowerBlackout = true;
					pBld->DisableTemporal();
					pBld->SetLayer(Layer::Ground);

					if(pBld->Type->CloakGenerator && pBld->CloakRadius) {
						pBld->HasCloakingData = -1;
						pBld->IsSensed = true;
						pBld->CloakRadius = 1;
						pBld->UpdateTimers();
					}

					// register for chronoshift
					ChronoWarpStateMachine::ChronoWarpContainer Container(pBld, cellUnitTarget, pBld->Location, IsVehicle);
					RegisteredBuildings.AddItem(Container);
				}	

				return true;
			};

			// collect every techno in this range only once. apply the Chronosphere.
			if(Helpers::Alex::DistinctCollector<ObjectClass*> *items = new Helpers::Alex::DistinctCollector<ObjectClass*>()) {
				Helpers::Alex::forEachObjectInRange(&pSource->ChronoMapCoords, pData->SW_WidthOrRange, pData->SW_Height, items->getCollector());
				items->forEach(Chronoport);

				if(RegisteredBuildings.Count) {
					this->newStateMachine(RulesClass::Instance->ChronoDelay, *pCoords, pSource, this, &RegisteredBuildings);
				}
			}

			return true;
		}
	} else {
		// idiots at work.
		Debug::Log("ChronoWarp typed super weapon triggered as standalone. Use ChronoSphere instead.\n");
	}
	
	return false;
}

void ChronoWarpStateMachine::Update() {
	int passed = this->TimePassed();

	if(passed == 1) {
		// redraw all buildings
		for(int i=0; i<this->Buildings.Count; ++i) {
			ChronoWarpContainer Container = this->Buildings.GetItem(i);
			if(Container.pBld) {
				Container.pBld->SetLayer(Layer::Ground);
			}
		}
	} else if(passed == this->Duration) {
		// copy the array so items can't get invalidated
		DynamicVectorClass<ChronoWarpContainer> buildings;
		for(int i=0; i<this->Buildings.Count; ++i) {
			buildings.AddItem(this->Buildings.GetItem(i));
		}
		this->Buildings.Clear();

		// remove all buildings from the map at once
		for(int i=0; i<buildings.Count; ++i) {
			ChronoWarpContainer pContainer = buildings.GetItem(i);
			pContainer.pBld->Remove();
		}

		// bring back all buildings
		for(int i=0; i<buildings.Count; ++i) {
			ChronoWarpContainer pContainer = buildings.GetItem(i);
			if(BuildingClass* pBld = pContainer.pBld) {

				if(!pBld->TemporalTargetingMe) {
					// use some logic to place this unit on some other
					// cell if the target cell is occupied. this emulates
					// the behavior of other units.
					bool success = false;
					int count = CellSpread::NumCells(10);
					int idx = 0;
					do {
						CellStruct cellNew = CellSpread::GetCell(idx) + pContainer.target;
						CellClass* pNewCell = MapClass::Instance->GetCellAt(&cellNew);
						CoordStruct coordsNew;
						pNewCell->GetCoordsWithBridge(&coordsNew);

						if(pBld->Type->CanCreateHere(&cellNew, 0)) {
							if(pBld->Put(&coordsNew, pBld->Facing)) {
								success = true;
								break;
							}
						}
						++idx;
					} while(pContainer.isVehicle && (idx<count));

					if(!success) {
						// put it back where it was
						++Unsorted::IKnowWhatImDoing;
						pBld->Put(&pContainer.origin, pBld->Facing);
						--Unsorted::IKnowWhatImDoing;
					}

					// chronoshift ends
					pBld->BeingWarpedOut = false;
					pBld->Owner->PowerBlackout = true;
					pBld->Owner->ShouldRecheckTechTree = true;
					pBld->EnableTemporal();
					pBld->SetLayer(Layer::Ground);

					if(!success) {
						if(SWTypeExt::ExtData *pExt = SWTypeExt::ExtMap.Find(this->Super->Type)) {
							// destroy (buildings only if they are supposed to)
							if(pContainer.isVehicle || pExt->Chronosphere_BlowUnplaceable.Get()) {
								int damage = pBld->Type->Strength;
								pBld->ReceiveDamage(&damage, 0,
									RulesClass::Instance->C4Warhead, NULL, true, true, this->Super->Owner);
							}
						}
					}
				}
			}
		}
	}
}

void ChronoWarpStateMachine::PointerGotInvalid(void *ptr) {
	for(int i=0; i<this->Buildings.Count; ++i) {
		if(this->Buildings.GetItem(i).pBld == ptr) {
			this->Buildings.RemoveItem(i);
			break;
		}
	}
}