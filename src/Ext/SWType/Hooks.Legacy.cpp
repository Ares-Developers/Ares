#include "Body.h"
#include "../Bullet/Body.h"
#include "../SWType/Body.h"
#include "../../Misc/SWTypes.h"
#include "../WarheadType/Body.h"
#include "../BuildingType/Body.h"
#include "../../Misc/SWTypes/Nuke.h"
#include "../../Utilities/Helpers.Alex.h"
#include "../../Misc/SWTypes/Dominator.h"
#include "../../Misc/SWTypes/LightningStorm.h"

#include <IonBlastClass.h>
#include <ScenarioClass.h>
#include <WarheadTypeClass.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>

// completely replace the PsyDom::Fire() method.
DEFINE_HOOK(53B080, PsyDom_Fire, 5) {
	if(SuperClass * pSuper = SW_PsychicDominator::CurrentPsyDom) {
		SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pSuper->Type);

		HouseClass* pFirer = PsyDom::Owner();
		CellStruct cell = PsyDom::Coords();

		CoordStruct coords;
		CellClass *pTarget = MapClass::Instance->GetCellAt(&cell);
		pTarget->GetCoords(&coords);
		
		// blast!
		if(pData->Dominator_Ripple.Get()) {
			IonBlastClass* pBlast = NULL;
			GAME_ALLOC(IonBlastClass, pBlast, coords);
			if(pBlast) {
				pBlast->DisableIonBeam = 1;
			}
		}

		// anim
		AnimClass* pAnim = NULL;
		if(pData->Dominator_SecondAnim.Get()) {
			CoordStruct animCoords = coords;
			animCoords.Z += pData->Dominator_SecondAnimHeight;
			GAME_ALLOC(AnimClass, pAnim, pData->Dominator_SecondAnim, &animCoords);
			PsyDom::Anim(pAnim);
		}

		// kill
		if(pData->SW_Damage > 0 && pData->SW_Warhead.Get()) {
			MapClass::Instance->DamageArea(&coords, pData->SW_Damage, 0, pData->SW_Warhead, 1, pFirer);
		}

		// capture
		if(pData->Dominator_Capture.Get()) {
			DynamicVectorClass<FootClass*> Minions;

			auto Dominate = [&](ObjectClass* pObj) -> bool {
				if(TechnoClass* pTechno = generic_cast<TechnoClass*>(pObj)) {
					TechnoTypeClass* pType = pTechno->GetTechnoType();

					// don't even try.
					if(pTechno->IsIronCurtained()) {
						return true;
					}

					// ignore BalloonHover and inair units.
					if(pType->BalloonHover || pTechno->IsInAir()) {
						return true;
					}

					// SW dependent stuff
					if(!pData->IsHouseAffected(pFirer, pTechno->Owner)) {
						return true;
					}

					if(!pData->IsTechnoAffected(pTechno)) {
						return true;
					}

					// ignore mind-controlled
					if(pTechno->MindControlledBy && !pData->Dominator_CaptureMindControlled) {
						return true;
					}

					// ignore ImmuneToPsionics, if wished.
					if(pType->ImmuneToPsionics && !pData->Dominator_CaptureImmuneToPsionics) {
						return true;
					}

					// free this unit
					if(pTechno->MindControlledBy) {
						pTechno->MindControlledBy->CaptureManager->FreeUnit(pTechno);
					}

					// capture this unit permanently
					pTechno->SetOwningHouse(pFirer);

					// create a permanent capture anim
					if(pData->Dominator_ControlAnim.Get()) {
						CoordStruct animCoords;
						pTechno->GetCoords(&animCoords);
						animCoords.Z += pType->MindControlRingOffset;
						GAME_ALLOC(AnimClass, pTechno->MindControlRingAnim, pData->Dominator_ControlAnim, &animCoords);
						if(pTechno->MindControlRingAnim) {
							pTechno->MindControlRingAnim->SetOwnerObject(pTechno);
						}
					}

					// add to the other newly captured minions.
					if(FootClass* pFoot = generic_cast<FootClass*>(pObj)) {
						Minions.AddItem(pFoot);
					}
				}

				return true;
			};

			// every techno in this area shall be one with Yuri.
			if(Helpers::Alex::DistinctCollector<ObjectClass*> *items = new Helpers::Alex::DistinctCollector<ObjectClass*>()) {
				Helpers::Alex::forEachObjectInRange(&cell, pData->SW_WidthOrRange, pData->SW_Height, items->getCollector());
				items->forEach(Dominate);
			}

			// the AI sends all new minions to hunt
			if(!PsyDom::Owner()->ControlledByHuman()) {
				for(int i=0; i<Minions.Count; ++i) {
					FootClass* pFoot = Minions.GetItem(i);
					pFoot->QueueMission(mission_Hunt, false);
				}
			}
		}
		
		// skip everything
		return 0x53B3EC;
	}
	return 0;
}

DEFINE_HOOK(53C321, ScenarioClass_UpdateLighting_PsyDom, 5) {
	if(SuperClass *pSuper = SW_PsychicDominator::CurrentPsyDom) {
		SWTypeExt::ChangeLighting(pSuper);
		R->EAX(1);
		return 0x53C43F;
	}

	return 0;
}

DEFINE_HOOK(53C2A6, ScenarioClass_UpdateLighting_LightningStorm, 5) {
	if(SuperClass *pSuper = SW_LightningStorm::CurrentLightningStorm) {
		SWTypeExt::ChangeLighting(pSuper);
		R->EAX(1);
		return 0x53C43F;
	}

	return 0;
}

DEFINE_HOOK(53C3B1, ScenarioClass_UpdateLighting_Nuke, 5) {
	if(SuperWeaponTypeClass *pType = SW_NuclearMissile::CurrentNukeType) {
		SWTypeExt::ChangeLighting(pType);
		R->EAX(1);
		return 0x53C43F;
	}

	return 0;
}

// skip the entire method, we handle it ourselves
DEFINE_HOOK(53AF40, PsyDom_Update, 6) {
	return 0x53B060;
}

// this is a complete rewrite of LightningStorm::Start.
DEFINE_HOOK(539EB0, LightningStorm_Start, 5) {
	if(SuperClass* pSuper = SW_LightningStorm::CurrentLightningStorm) {
		GET(int, duration, ECX);
		GET(int, deferment, EDX);
		GET_STACK(CellStruct, Coords, 0x4);
		GET_STACK(HouseClass*, pOwner, 0x8);

		SuperWeaponTypeClass *pType = pSuper->Type;
		SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pType);

		bool ret = false;

		// generate random coords if the passed ones are empty
		if(Coords.X == 0 && Coords.Y == 0) {
			for(; !MapClass::Instance->CellExists(&Coords);) {
				Coords.X = (short)ScenarioClass::Instance->Random.RandomRanged(0, MapClass::Instance->unknown_12C);
				Coords.Y = (short)ScenarioClass::Instance->Random.RandomRanged(0, MapClass::Instance->unknown_130);
			}
		}

		// yes. set them even if the Lightning Storm
		// is active.
		LightningStorm::Coords(Coords);
		LightningStorm::Owner(pOwner);
		
		if(!LightningStorm::Active()) {
			if(deferment) {
				// register this storm to start soon
				if(!LightningStorm::Deferment() || LightningStorm::Deferment() >= deferment) {
					LightningStorm::Deferment(deferment);
				}
				LightningStorm::Duration(duration);
				ret = true;
			} else {
				// start the mayhem. not setting this will create an
				// infinite loop. not tested what happens after that.
				LightningStorm::Duration(duration);
				LightningStorm::StartTime(Unsorted::CurrentFrame);
				LightningStorm::Active(true);

				// blackout
				if(pData->Weather_RadarOutage > 0) {
					for(int i=0; i<HouseClass::Array->Count; ++i) {
						HouseClass* pHouse = HouseClass::Array->GetItem(i);
						if(pData->IsHouseAffected(pOwner, pHouse)) {
							if(!pHouse->Defeated) {
								pHouse->CreateRadarOutage(pData->Weather_RadarOutage);
							}
						}
					}
				}
				if(HouseClass::Player) {
					HouseClass::Player->RadarBlackout = true;
				}

				// let there be light
				ScenarioClass::Instance->UpdateLighting();

				// activation stuff
				if(pData->Weather_PrintText.Get()) {
					pData->PrintMessage(pData->Message_Activate, pSuper->Owner);
				}
				if(pData->SW_ActivationSound != -1) {
					VocClass::PlayGlobal(pData->SW_ActivationSound, 1.0, 0);
				}

				MapClass::Instance->RedrawSidebar(1);
			}
		}

		R->EAX(ret);
		return 0x539F80;
	}

	// legacy way still needed for triggers.
	return 0;
}

DEFINE_HOOK(53A8C6, LightningStorm_Update, 5) 
DEFINE_HOOK_AGAIN(53A8FF, LightningStorm_Update, 5) {
	if(SuperClass* pSuper = SW_LightningStorm::CurrentLightningStorm) {
		CellStruct LSCell = LightningStorm::Coords();

		SuperWeaponTypeClass *pType = pSuper->Type;
		SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pType);

		if(!LightningStorm::Active() || LightningStorm::TimeToEnd()) {
			int deferment = LightningStorm::Deferment();

			// still counting down?
			if(deferment > 0) {
				--deferment;
				LightningStorm::Deferment(deferment);

				// still waiting
				if(deferment) {
					if(!(deferment % 225)) {
						if(pData->Weather_PrintText.Get()) {
							pData->PrintMessage(pData->Message_Launch, pSuper->Owner);
						}
					}
				} else {
					// launch the storm
					LightningStorm::Start(LightningStorm::Duration(), 0, LSCell,
						LightningStorm::Owner());
				}
			}
		} else {
			// does this Lightning Storm go on?
			int duration = LightningStorm::Duration();
			if(duration == -1 || duration + LightningStorm::StartTime() >= Unsorted::CurrentFrame) {

				// deterministic damage. the very target cell.
				if(pData->Weather_HitDelay > 0 && !(Unsorted::CurrentFrame % pData->Weather_HitDelay)) {
					LightningStorm::Strike(LightningStorm::Coords());
				}

				// random damage. somewhere in range.
				if(pData->Weather_ScatterDelay > 0 && !(Unsorted::CurrentFrame % pData->Weather_ScatterDelay)) {
					int width = (int)pData->SW_WidthOrRange;
					int height = pData->SW_Height;
					bool isRectangle = true;

					// is circular range?
					if(height < 0) {
						height = width;
						isRectangle = false;
					}

					// generate a new place to strike
					CellStruct cell;
					if(height > 0 && width > 0 && MapClass::Instance->CellExists(&LSCell)) {
						bool found = true;
						for(int i=0; i<5; ++i) {
							cell = LSCell;
							cell.X += (short)ScenarioClass::Instance->Random.RandomRanged(-width / 2, width / 2);
							cell.Y += (short)ScenarioClass::Instance->Random.RandomRanged(-height / 2, height / 2);

							// don't even try if this is invalid
							if(MapClass::Instance->CellExists(&cell)) {
								// out of range?
								if(!isRectangle) {
									if(cell.DistanceFrom(LSCell) > pData->SW_WidthOrRange) {
										continue;
									}
								}

								// if we respect lightning rods, start looking for one.
								if(!pData->Weather_IgnoreLightningRod.Get()) {
									// if, by coincidence, this is a rod, hit it.
									CellClass *pImpactCell = MapClass::Instance->GetCellAt(&cell);
									if(BuildingClass *pBld = pImpactCell->GetBuilding()) {
										if(pBld->Type->LightningRod) {
											break;
										}
									}

									// if a lightning rod is next to this, hit that instead. naive.
									CoordStruct nullCoords;
									if(ObjectClass* pObj = pImpactCell->FindObjectNearestTo(&nullCoords, false, pImpactCell->GetBuilding())) {
										if(BuildingClass *pBld = specific_cast<BuildingClass*>(pObj)) {
											if(pBld->Type->LightningRod) {
												cell = MapClass::Instance->GetCellAt(&pBld->Location)->MapCoords;
												break;
											}
										}
									}
								}

								// is this spot far away from another cloud?
								if(pData->Weather_Separation > 0) {
									DynamicVectorClass<AnimClass*>* pAnims = (DynamicVectorClass<AnimClass*>*)(0xA9F9D0);
									for(int k=0; k<pAnims->Count; ++k) {
										// assume success and disprove.
										if(cell.DistanceFrom(pAnims->GetItem(k)->GetCell()->MapCoords) < pData->Weather_Separation) {
											found = false;
											break;
										}
									}
								}
							}
						}

						// found a valid position. strike there.
						if(found) {
							LightningStorm::Strike(cell);
						}
					}
				}
			} else {
				// it's over already
				LightningStorm::TimeToEnd(1);
			}
		}

		// jump over everything
		return 0x53AB45;
	}

	// still support old logic for triggers
	return 0;
}

// create a cloud.
DEFINE_HOOK(53A140, LightningStorm_Strike, 7) {
	if(SuperClass* pSuper = SW_LightningStorm::CurrentLightningStorm) {
		GET_STACK(CellStruct, Cell, 0x4);

		SuperWeaponTypeClass *pType = pSuper->Type;
		SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pType);

		// get center of cell coords
		CoordStruct Coords;
		CellClass* pCell = MapClass::Instance->GetCellAt(&Cell);
		pCell->GetCoordsWithBridge(&Coords);
		
		// create a cloud animation
		if(Coords != *(CoordStruct*)(0xA9FA30)) {
			// select the anim
			AnimTypeClass* pAnimType = pData->Weather_Clouds.GetItem(ScenarioClass::Instance->Random.Random() % pData->Weather_Clouds.Count);

			// infer the height this thing will be drawn at.
			if(pData->Weather_CloudHeight < 0) {
				if(pData->Weather_Bolts.Count) {
					AnimTypeClass* pBoltAnim = pData->Weather_Bolts.GetItem(0);
					pData->Weather_CloudHeight = pBoltAnim->GetImage()->Height;
				}
			}
			Coords.Z += (int)Game::F2I(((pData->Weather_CloudHeight / 2) - 0.5) * *(double*)(0xB0CDD8));

			// create it and do hacky book keeping.
			AnimClass* pAnim = NULL;
			GAME_ALLOC(AnimClass, pAnim, pAnimType, &Coords);

			if(pAnim) {
				DynamicVectorClass<AnimClass*>* pAnims1 = (DynamicVectorClass<AnimClass*>*)(0xA9FA60);
				DynamicVectorClass<AnimClass*>* pAnims2 = (DynamicVectorClass<AnimClass*>*)(0xA9F9D0);

				pAnims1->AddItem(pAnim);
				pAnims2->AddItem(pAnim);
			}
		}

		R->EAX(true);
		return 0x53A2F1;
	}

	// legacy way for triggers.
	return 0;
}

// create bolt and damage area.
DEFINE_HOOK(53A300, LightningStorm_Strike2, 5) {
	if(SuperClass* pSuper = SW_LightningStorm::CurrentLightningStorm) {
		GET_STACK(CoordStruct, Coords, 0x4);

		SuperWeaponTypeClass *pType = pSuper->Type;
		SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pType);

		// get center of cell coords
		CellClass* pCell = MapClass::Instance->GetCellAt(&Coords);
		pCell->GetCoordsWithBridge(&Coords);

		if(Coords != *(CoordStruct*)(0xA9FA30)) {

			// create a bolt animation
			if(pData->Weather_Bolts.Count) {
				DWORD rnd = ScenarioClass::Instance->Random.Random();
				AnimTypeClass* pAnimType = pData->Weather_Bolts.GetItem(rnd % pData->Weather_Bolts.Count);

				AnimClass* pAnim = NULL;
				GAME_ALLOC(AnimClass, pAnim, pAnimType, &Coords);
				
				if(pAnim) {
					DynamicVectorClass<AnimClass*>* pAnims = (DynamicVectorClass<AnimClass*>*)(0xA9FA18);
					pAnims->AddItem(pAnim);
				}
			}
			
			// play lightning sound
			if(pData->Weather_Sounds.Count) {
				int rnd = ScenarioClass::Instance->Random.Random();
				VocClass::PlayAt(pData->Weather_Sounds.GetItem(rnd % pData->Weather_Sounds.Count), &Coords, NULL);
			}
						
			bool debris = false;
			BuildingClass* pBld = pCell->GetBuilding();

			CoordStruct empty;
			ObjectClass* pObj = pCell->FindObjectNearestTo(&empty, false, NULL);
			bool isInfantry = (pObj && pObj->WhatAmI() == abs_Infantry);

			// empty cell action
			if(!pBld && !pObj) {
				switch(pCell->LandType)
				{
				case lt_Road:
				case lt_Rock:
				case lt_Wall:
				case lt_Weeds:
					debris = true;
					break;
				default:
				break;
		        }
			}

			// account for lightning rods
			int damage = pData->SW_Damage;
			if(!pData->Weather_IgnoreLightningRod.Get()) {
				if(BuildingClass* pBld = specific_cast<BuildingClass*>(pObj)) {
					if(pBld->Type->LightningRod) {
						if(BuildingTypeExt::ExtData *pExt = BuildingTypeExt::ExtMap.Find(pBld->Type)) {
							// multiply the damage, but never go below zero.
							damage = (int)std::max(damage * pExt->LightningRod_Modifier, 0.0);
						}
					}
				}
			}

			// cause mayhem
			if(damage) {
				MapClass::FlashbangWarheadAt(damage, pData->SW_Warhead, Coords, false, 0);
				MapClass::DamageArea(&Coords, damage, NULL, pData->SW_Warhead, true, pSuper->Owner);

				// fancy stuff if damage is dealt
				AnimClass* pAnim = NULL;
				AnimTypeClass* pAnimType = MapClass::SelectDamageAnimation(damage, pData->SW_Warhead, pCell->LandType, &Coords);
				GAME_ALLOC(AnimClass, pAnim, pAnimType, &Coords);
			}

			// has the last target been destroyed?
			if(pObj != pCell->FindObjectNearestTo(&empty, false, NULL)) {
				debris = true;
			}

			// create some debris
			if(pData->Weather_Debris.Count) {
				
				// dead infantry never generates debris.
				if(!isInfantry && debris) {
					int count = ScenarioClass::Instance->Random.RandomRanged(pData->Weather_DebrisMin, pData->Weather_DebrisMax);
					for(int i=0; i<count; ++i) {
						DWORD rnd = ScenarioClass::Instance->Random.Random();
						AnimTypeClass *pAnimType = pData->Weather_Debris.GetItem(rnd % pData->Weather_Debris.Count);

						AnimClass *pAnim = NULL;
						GAME_ALLOC(AnimClass, pAnim, pAnimType, &Coords);
					}
				}
			}
		}
		
		return 0x53A69A;
	}
	
	// legacy way for triggers.
	return 0;
}

DEFINE_HOOK(48A59A, MapClass_SelectDamageAnimation_LightningWarhead, 5) {
	// override the lightning bolt explosion
	GET(WarheadTypeClass*, pWarhead, ESI);
	if(SuperClass* pSuper = SW_LightningStorm::CurrentLightningStorm) {
		SuperWeaponTypeClass *pType = pSuper->Type;
		if(SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pType)) {
			if((pData->SW_Warhead == pWarhead) && pData->Weather_BoltExplosion.Get()) {
				R->EAX(pData->Weather_BoltExplosion.Get());
				return 0x48A5AD;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(44C9FF, BuildingClass_Missile_PsiWarn, 6) {
	GET(BuildingClass*, pThis, ESI);
	int type = pThis->FiringSWType;

	if(SuperWeaponTypeClass::Array->ValidIndex(type)) {
		if(SuperWeaponTypeClass* pSW = SuperWeaponTypeClass::Array->GetItem(type)) {
			if(SWTypeExt::ExtData* pExt = SWTypeExt::ExtMap.Find(pSW)) {
				if(AnimTypeClass *pAnim = pExt->Nuke_PsiWarning) {
					R->EAX(pAnim->ArrayIndex);
					Debug::Log("PsiWarn set\n");
					return 0;
				}

				// skip psi warning.
				Debug::Log("PsiWarn skipped\n");
				return 0x44CA7A;
			}
		}
	}

	return 0;
}

// upward pointing missile, launched from missile silo.
DEFINE_HOOK(44CABA, BuildingClass_Missile_CreateBullet, 6) {
	GET(CellClass*, pCell, EAX);
	GET(BuildingClass*, pThis, ESI);

	int type = pThis->FiringSWType;

	if(SuperWeaponTypeClass::Array->ValidIndex(type)) {
		if(SuperWeaponTypeClass* pSW = SuperWeaponTypeClass::Array->GetItem(type)) {
			if(SWTypeExt::ExtData* pExt = SWTypeExt::ExtMap.Find(pSW)) {
				if(WeaponTypeClass *pWeapon = pSW->WeaponType) {
					if(BulletClass* pBullet = pWeapon->Projectile->CreateBullet(pCell, pThis, pWeapon->Damage, pWeapon->Warhead, 255, true)) {
						if(BulletExt::ExtData *pData = BulletExt::ExtMap.Find(pBullet)) {
							pData->NukeSW = pSW;
						}

						R->EBX(pSW->WeaponType);
						R->EAX(pBullet);
						
						return 0x44CAF2;
					}
				}
			}
		}
	}

	return 0;
}

// special takeoff anim.
DEFINE_HOOK(44CC8B, BuildingClass_Missile_NukeTakeOff, 6) {
	GET(BuildingClass*, pThis, ESI);

	int type = pThis->FiringSWType;

	if(SuperWeaponTypeClass::Array->ValidIndex(type)) {
		if(SuperWeaponTypeClass* pSW = SuperWeaponTypeClass::Array->GetItem(type)) {
			if(SWTypeExt::ExtData* pExt = SWTypeExt::ExtMap.Find(pSW)) {
				if(pExt->Nuke_TakeOff.Get()) {
					
					R->ECX(pExt->Nuke_TakeOff.Get());
					return 0x44CC91;
				}
			}
		}
	}

	return 0;
}

// create a downward pointing missile if the launched one leaves the map.
DEFINE_HOOK(46B371, BulletClass_NukeMaker, 5) {
	GET(BulletClass*, pBullet, EBP);

	if(pBullet && pBullet->WeaponType) {
		if(BulletExt::ExtData *pData = BulletExt::ExtMap.Find(pBullet)) {
			if(SuperWeaponTypeClass *pSW = pData->NukeSW) {
				if(SWTypeExt::ExtData* pExt = SWTypeExt::ExtMap.Find(pSW)) {

					// get the per-SW nuke payload weapon
					if(WeaponTypeClass *pPayload = pExt->Nuke_Payload) {

						// get damage and warhead. they are not available during
						// initialisation, so we gotta fall back now if they are invalid.
						int damage = (pExt->SW_Damage < 0 ? pPayload->Damage : pExt->SW_Damage.Get());
						WarheadTypeClass *pWarhead = (!pExt->SW_Warhead ? pPayload->Warhead : pExt->SW_Warhead.Get());

						// put the new values into the registers
						R->Stack(0x30, R->EAX());
						R->ESI(pPayload);
						R->Stack(0x10, 0);
						R->Stack(0x18, pPayload->Speed);
						R->Stack(0x28, pPayload->Projectile);
						R->EAX(pWarhead);
						R->ECX(R->lea_Stack<CoordStruct*>(0x10));
						R->EDX(damage);
				
						return 0x46B3B7;
					} else {
						Debug::Log("[%s] has no payload weapon type, or it is invalid.\n", pSW->ID);
					}
				}
			}
		}
	}

	return 0;
}

// just puts the launched SW pointer on the downward aiming missile.
DEFINE_HOOK(46B423, BulletClass_NukeMaker_PropagateSW, 6) {
	GET(BulletClass*, pBullet, EBP);
	GET(BulletClass*, pNuke, EDI);

	if(BulletExt::ExtData *pData = BulletExt::ExtMap.Find(pBullet)) {
		if(BulletExt::ExtData *pExt = BulletExt::ExtMap.Find(pNuke)) {
			pExt->NukeSW = pData->NukeSW;
		}
	}

	return 0;
}

// deferred explosion. create a nuke ball anim and, when that is over, go boom.
DEFINE_HOOK(467E59, BulletClass_Update_NukeBall, 5) {
	// changed the hardcoded way to just do this if the
	// warhead is called NUKE to an more universal
	// approach. every warhead having a pre-impact-anim
	// will get this behavior.
	GET(BulletClass*, pBullet, EBP);

	if(WarheadTypeExt::ExtData* pExt = WarheadTypeExt::ExtMap.Find(pBullet->WH)) {
		if(pExt->PreImpactAnim != -1) {
			// copy what the original function does, but only do it if
			// this is a SW launched bullet.
			if(BulletExt::ExtData* pData = BulletExt::ExtMap.Find(pBullet)) {
				SW_NuclearMissile::CurrentNukeType = pData->NukeSW;

				if(pData->NukeSW) {
					if(pBullet->GetHeight() < 0) {
						pBullet->SetHeight(0);
					}

					// replaces this call:
					//(*(void (__stdcall *)())(0x53AB70))();

					// manual light stuff
					LightningStorm::Status(1);
					ScenarioClass::Instance->Timer4.StartTime = Unsorted::CurrentFrame;
					ScenarioClass::Instance->Timer4.unknown = R->Stack<int>(0x28);
					ScenarioClass::Instance->Timer4.TimeLeft = 1;

					// hacky stuff
					*(int*)0x827FC8 = Unsorted::CurrentFrame;
					*(int*)0x827FCC = 30;

					SWTypeExt::ChangeLighting(pData->NukeSW);
					MapClass::Instance->RedrawSidebar(1);

					// cause yet another radar event
					CellStruct coords;
					pBullet->GetMapCoords(&coords);
					RadarEventClass::Create(RADAREVENT_SUPERWEAPONLAUNCHED, coords);
				}
			}

			R->EAX(pExt->PreImpactAnim.Get());
			return 0x467EB6;
		}

		// no pre impact anim
		return 0x467F9B;
	}

	return 0;
}