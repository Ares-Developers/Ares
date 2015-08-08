#include "Body.h"
#include "../Bullet/Body.h"
#include "../SWType/Body.h"
#include "../Building/Body.h"
#include "../../Misc/SWTypes.h"
#include "../WarheadType/Body.h"
#include "../BuildingType/Body.h"
#include "../../Ext/Techno/Body.h"
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

		HouseClass* pFirer = PsyDom::Owner;
		CellStruct cell = PsyDom::Coords;

		CellClass *pTarget = MapClass::Instance->GetCellAt(cell);
		CoordStruct coords = pTarget->GetCoords();
		
		// blast!
		if(pData->Dominator_Ripple) {
			if(auto pBlast = GameCreate<IonBlastClass>(coords)) {
				pBlast->DisableIonBeam = TRUE;
			}
		}

		// tell!
		if(pData->SW_RadarEvent) {
			RadarEventClass::Create(RadarEventType::SuperweaponActivated, cell);
		}

		// anim
		PsyDom::Anim = nullptr;
		if(AnimTypeClass* pAnimType = pData->Dominator_SecondAnim.Get(RulesClass::Instance->DominatorSecondAnim)) {
			CoordStruct animCoords = coords;
			animCoords.Z += pData->Dominator_SecondAnimHeight;
			PsyDom::Anim = GameCreate<AnimClass>(pAnimType, animCoords);
		}

		// kill
		auto damage = pData->GetDamage();
		if(damage > 0) {
			if(auto pWarhead = pData->GetWarhead()) {
				MapClass::Instance->DamageArea(coords, damage, nullptr, pWarhead, true, pFirer);
			}
		}

		// capture
		if(pData->Dominator_Capture) {
			DynamicVectorClass<FootClass*> Minions;

			auto Dominate = [pData, pFirer, &Minions](TechnoClass* pTechno) -> bool {
				TechnoTypeClass* pType = pTechno->GetTechnoType();

				// don't even try.
				if(pTechno->IsIronCurtained()) {
					return true;
				}

				// ignore BalloonHover and inair units.
				if(pType->BalloonHover || pTechno->IsInAir()) {
					return true;
				}

				// ignore units with no drivers
				TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pTechno);
				if(pExt->DriverKilled) {
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

				// ignore permanently mind-controlled
				if(pTechno->MindControlledByAUnit && !pTechno->MindControlledBy
					&& !pData->Dominator_CapturePermaMindControlled) {
						return true;
				}

				// ignore ImmuneToPsionics, if wished
				if(pType->ImmuneToPsionics && !pData->Dominator_CaptureImmuneToPsionics) {
					return true;
				}

				// free this unit
				if(pTechno->MindControlledBy) {
					pTechno->MindControlledBy->CaptureManager->FreeUnit(pTechno);
				}

				// capture this unit, maybe permanently
				pTechno->SetOwningHouse(pFirer);
				pTechno->MindControlledByAUnit = pData->Dominator_PermanentCapture;

				// remove old permanent mind control anim
				if(pTechno->MindControlRingAnim) {
					pTechno->MindControlRingAnim->UnInit();
					pTechno->MindControlRingAnim = nullptr;
				}

				// create a permanent capture anim
				if(AnimTypeClass* pAnimType = pData->Dominator_ControlAnim.Get(RulesClass::Instance->PermaControlledAnimationType)) {
					CoordStruct animCoords = pTechno->GetCoords();
					animCoords.Z += pType->MindControlRingOffset;
					pTechno->MindControlRingAnim = GameCreate<AnimClass>(pAnimType, animCoords);
					if(pTechno->MindControlRingAnim) {
						pTechno->MindControlRingAnim->SetOwnerObject(pTechno);
					}
				}

				// add to the other newly captured minions.
				if(FootClass* pFoot = generic_cast<FootClass*>(pTechno)) {
					Minions.AddItem(pFoot);
				}

				return true;
			};

			// every techno in this area shall be one with Yuri.
			auto range = pData->GetRange();
			Helpers::Alex::DistinctCollector<TechnoClass*> items;
			Helpers::Alex::for_each_in_rect_or_spread<TechnoClass>(cell, range.WidthOrRange, range.Height, items);
			items.for_each(Dominate);

			// the AI sends all new minions to hunt
			if(!PsyDom::Owner->ControlledByHuman()) {
				for(int i=0; i<Minions.Count; ++i) {
					FootClass* pFoot = Minions.GetItem(i);
					pFoot->QueueMission(Mission::Hunt, false);
				}
			}
		}
		
		// skip everything
		return 0x53B3EC;
	}
	return 0;
}

// replace entire function
DEFINE_HOOK(53C280, ScenarioClass_UpdateLighting, 5)
{
	auto lighting = SWTypeExt::GetLightingColor();

	auto scen = ScenarioClass::Instance;
	if(lighting.HasValue) {
		// something changed the lighting
		scen->AmbientTarget = lighting.Ambient;
		scen->RecalcLighting(lighting.Red, lighting.Green, lighting.Blue, 1);
	} else {
		// default lighting
		scen->AmbientTarget = scen->AmbientOriginal;
		scen->RecalcLighting(-1, -1, -1, 0);
	}

	return 0x53C441;
}

DEFINE_HOOK(555E50, LightConvertClass_CTOR_Lighting, 5)
{
	GET(LightConvertClass*, pThis, ESI);

	auto lighting = SWTypeExt::GetLightingColor();

	if(lighting.HasValue) {
		if(pThis->Color1.Red == -1) {
			pThis->Color1.Red = 1000;
			pThis->Color1.Green = 1000;
			pThis->Color1.Blue = 1000;
		}
	} else {
		lighting.Red = pThis->Color1.Red;
		lighting.Green = pThis->Color1.Green;
		lighting.Blue = pThis->Color1.Blue;
	}

	pThis->UpdateColors(lighting.Red, lighting.Green, lighting.Blue, lighting.HasValue);

	return 0x55606C;
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
		if(Coords == CellStruct::Empty) {
			auto const& Bounds = MapClass::Instance->MapCoordBounds;
			auto& Random = ScenarioClass::Instance->Random;
			while(!MapClass::Instance->CellExists(Coords)) {
				Coords.X = static_cast<short>(Random.RandomRanged(0, Bounds.Right));
				Coords.Y = static_cast<short>(Random.RandomRanged(0, Bounds.Bottom));
			}
		}

		// yes. set them even if the Lightning Storm
		// is active.
		LightningStorm::Coords = Coords;
		LightningStorm::Owner = pOwner;
		
		if(!LightningStorm::Active) {
			if(deferment) {
				// register this storm to start soon
				if(!LightningStorm::Deferment || LightningStorm::Deferment >= deferment) {
					LightningStorm::Deferment = deferment;
				}
				LightningStorm::Duration = duration;
				ret = true;
			} else {
				// start the mayhem. not setting this will create an
				// infinite loop. not tested what happens after that.
				LightningStorm::Duration = duration;
				LightningStorm::StartTime = Unsorted::CurrentFrame;
				LightningStorm::Active = true;

				// blackout
				auto outage = pData->Weather_RadarOutage.Get(RulesClass::Instance->LightningStormDuration);
				if(outage > 0) {
					for(int i=0; i<HouseClass::Array->Count; ++i) {
						HouseClass* pHouse = HouseClass::Array->GetItem(i);
						if(pData->IsHouseAffected(pOwner, pHouse, pData->Weather_RadarOutageAffects)) {
							if(!pHouse->Defeated) {
								pHouse->CreateRadarOutage(outage);
							}
						}
					}
				}
				if(HouseClass::Player) {
					HouseClass::Player->RecheckRadar = true;
				}

				// let there be light
				ScenarioClass::Instance->UpdateLighting();

				// activation stuff
				if(pData->Weather_PrintText.Get(RulesClass::Instance->LightningPrintText)) {
					pData->PrintMessage(pData->Message_Activate, pSuper->Owner);
				}

				int sound = pData->SW_ActivationSound.Get(RulesClass::Instance->StormSound);
				if(sound != -1) {
					VocClass::PlayGlobal(sound, 8192, 1.0);
				}

				if(pData->SW_RadarEvent) {
					RadarEventClass::Create(RadarEventType::SuperweaponActivated, Coords);
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

// this is a complete rewrite of LightningStorm::Update.
DEFINE_HOOK(53A6CF, LightningStorm_Update, 7) {
	// switch lightning for nuke
	if(NukeFlash::Duration != -1) {
		if(NukeFlash::StartTime + NukeFlash::Duration < Unsorted::CurrentFrame) {
			if(NukeFlash::IsFadingIn()) {
				NukeFlash::Status = NukeFlashStatus::FadeOut;
				NukeFlash::StartTime = Unsorted::CurrentFrame;
				NukeFlash::Duration = 15;
				ScenarioClass::Instance->UpdateLighting();
				MapClass::Instance->RedrawSidebar(1);
			} else if(NukeFlash::IsFadingOut()) {
				SW_NuclearMissile::CurrentNukeType = nullptr;
				NukeFlash::Status = NukeFlashStatus::Inactive;
			}
		}
	}

	// update other screen effects
	PsyDom::Update();
	ChronoScreenEffect::Update();

	// remove all bolts from the list that are halfway done
	if(LightningStorm::BoltsPresent->Count > 0) {
		for(int i=LightningStorm::BoltsPresent->Count-1; i>=0; --i) {
			if(AnimClass *pAnim = LightningStorm::BoltsPresent->GetItem(i)) {
				if(pAnim->Animation.Value >= pAnim->Type->GetImage()->Frames / 2) {
					LightningStorm::BoltsPresent->RemoveItem(i);
				}
			}
		}
	}

	// find the clouds that should strike right now
	for(int i=LightningStorm::CloudsManifesting->Count-1; i>=0; --i) {
		if(AnimClass *pAnim = LightningStorm::CloudsManifesting->GetItem(i)) {
			if(pAnim->Animation.Value >= pAnim->Type->GetImage()->Frames / 2) {
				CoordStruct crdStrike = pAnim->GetCoords();
				LightningStorm::Strike2(crdStrike);
				LightningStorm::CloudsManifesting->RemoveItem(i);
			}
		}
	}

	// all currently present clouds have to disappear first
	if(LightningStorm::CloudsPresent->Count <= 0) {
		// end the lightning storm
		if(LightningStorm::TimeToEnd) {
			if(LightningStorm::Active) {
				LightningStorm::Active = false;
				LightningStorm::Owner = nullptr;
				LightningStorm::Coords = CellStruct::Empty;
				SW_LightningStorm::CurrentLightningStorm = nullptr;
				ScenarioClass::Instance->UpdateLighting();
			}
			LightningStorm::TimeToEnd = false;
		}
	} else {
		for(int i=LightningStorm::CloudsPresent->Count-1; i>=0; --i) {
			if(AnimClass *pAnim = LightningStorm::CloudsPresent->GetItem(i)) {
				if(pAnim->Animation.Value >= pAnim->Type->GetImage()->Frames - 1) {
					LightningStorm::CloudsPresent->RemoveItem(i);
				}
			}
		}
	}

	// check for presence of Ares SW
	if(SuperClass* pSuper = SW_LightningStorm::CurrentLightningStorm) {
		CellStruct LSCell = LightningStorm::Coords;

		SuperWeaponTypeClass *pType = pSuper->Type;
		SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pType);

		if(!LightningStorm::Active || LightningStorm::TimeToEnd) {
			int deferment = LightningStorm::Deferment;

			// still counting down?
			if(deferment > 0) {
				--deferment;
				LightningStorm::Deferment = deferment;

				// still waiting
				if(deferment) {
					if(!(deferment % 225)) {
						if(pData->Weather_PrintText.Get(RulesClass::Instance->LightningPrintText)) {
							pData->PrintMessage(pData->Message_Launch, pSuper->Owner);
						}
					}
				} else {
					// launch the storm
					LightningStorm::Start(LightningStorm::Duration, 0, LSCell, LightningStorm::Owner);
				}
			}
		} else {
			// does this Lightning Storm go on?
			int duration = LightningStorm::Duration;
			if(duration == -1 || duration + LightningStorm::StartTime >= Unsorted::CurrentFrame) {

				// deterministic damage. the very target cell.
				auto hitDelay = pData->Weather_HitDelay.Get(RulesClass::Instance->LightningHitDelay);
				if(hitDelay > 0 && !(Unsorted::CurrentFrame % hitDelay)) {
					LightningStorm::Strike(LightningStorm::Coords);
				}

				// random damage. somewhere in range.
				auto scatterDelay = pData->Weather_ScatterDelay.Get(RulesClass::Instance->LightningScatterDelay);
				if(scatterDelay > 0 && !(Unsorted::CurrentFrame % scatterDelay)) {
					auto range = pData->GetRange();
					int width = range.width();
					int height = range.height();
					bool isRectangle = true;

					// is circular range?
					if(height <= 0) {
						height = width;
						isRectangle = false;
					}

					// generate a new place to strike
					CellStruct cell;
					if(height > 0 && width > 0 && MapClass::Instance->CellExists(LSCell)) {
						for(int k=pData->Weather_ScatterCount; k>0; --k) {
							bool found;
							for(int i=0; i<3; ++i) {
								cell = LSCell;
								cell.X += static_cast<short>(ScenarioClass::Instance->Random.RandomRanged(-width / 2, width / 2));
								cell.Y += static_cast<short>(ScenarioClass::Instance->Random.RandomRanged(-height / 2, height / 2));
	
								// don't even try if this is invalid
								found = false;
								if(MapClass::Instance->CellExists(cell)) {
									// out of range?
									if(!isRectangle) {
										if(cell.DistanceFrom(LSCell) > range.WidthOrRange) {
											continue;
										}
									}

									// assume valid
									found = true;

									// if we respect lightning rods, start looking for one.
									if(!pData->Weather_IgnoreLightningRod) {
										// if, by coincidence, this is a rod, hit it.
										CellClass *pImpactCell = MapClass::Instance->GetCellAt(cell);
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
													cell = MapClass::Instance->GetCellAt(pBld->Location)->MapCoords;
													break;
												}
											}
										}
									}

									// is this spot far away from another cloud?
									auto separation = pData->Weather_Separation.Get(RulesClass::Instance->LightningSeparation);
									if(separation > 0) {
										for(int j=0; j<LightningStorm::CloudsPresent->Count; ++j) {
											// assume success and disprove.
											CellStruct *pCell2 = &LightningStorm::CloudsPresent->GetItem(j)->GetCell()->MapCoords;
											int dist = std::abs(pCell2->X - cell.X) + std::abs(pCell2->Y - cell.Y);
											if(dist < separation) {
												found = false;
												break;
											}
										}
									}

									// valid cell.
									if(found) {
										break;
									}
								}
							}

							// found a valid position. strike there.
							if(found) {
								LightningStorm::Strike(cell);
							}
						}
					}
				}
			} else {
				// it's over already
				LightningStorm::TimeToEnd = true;
			}
		}

		// jump over everything
		return 0x53AB45;
	}

	// still support old logic for triggers
	return 0x53A8FF;
}

// create a cloud.
DEFINE_HOOK(53A140, LightningStorm_Strike, 7) {
	if(SuperClass* pSuper = SW_LightningStorm::CurrentLightningStorm) {
		GET_STACK(CellStruct, Cell, 0x4);

		SuperWeaponTypeClass *pType = pSuper->Type;
		SWTypeExt::ExtData *pData = SWTypeExt::ExtMap.Find(pType);

		// get center of cell coords
		CellClass* pCell = MapClass::Instance->GetCellAt(Cell);
		CoordStruct Coords = pCell->GetCoordsWithBridge();

		// create a cloud animation
		if(Coords != CoordStruct::Empty) {
			// select the anim
			auto itClouds = pData->Weather_Clouds.GetElements(RulesClass::Instance->WeatherConClouds);
			AnimTypeClass* pAnimType = itClouds.at(ScenarioClass::Instance->Random.Random() % itClouds.size());

			// infer the height this thing will be drawn at.
			if(pData->Weather_CloudHeight < 0) {
				if(auto itBolts = pData->Weather_Bolts.GetElements(RulesClass::Instance->WeatherConBolts)) {
					AnimTypeClass* pBoltAnim = itBolts.at(0);
					pData->Weather_CloudHeight = Game::F2I(((pBoltAnim->GetImage()->Height / 2) - 0.5) * LightningStorm::CloudHeightFactor);
				}
			}
			Coords.Z += pData->Weather_CloudHeight;

			// create the cloud and do some book keeping.
			if(auto pAnim = GameCreate<AnimClass>(pAnimType, Coords)) {
				LightningStorm::CloudsManifesting->AddItem(pAnim);
				LightningStorm::CloudsPresent->AddItem(pAnim);
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
		CellClass* pCell = MapClass::Instance->GetCellAt(Coords);
		Coords = pCell->GetCoordsWithBridge();

		if(Coords != CoordStruct::Empty) {

			// create a bolt animation
			if(auto it = pData->Weather_Bolts.GetElements(RulesClass::Instance->WeatherConBolts)) {
				DWORD rnd = ScenarioClass::Instance->Random.Random();
				AnimTypeClass* pAnimType = it.at(rnd % it.size());

				if(auto pAnim = GameCreate<AnimClass>(pAnimType, Coords)) {
					LightningStorm::BoltsPresent->AddItem(pAnim);
				}
			}

			// play lightning sound
			if(auto it = pData->Weather_Sounds.GetElements(RulesClass::Instance->LightningSounds)) {
				DWORD rnd = ScenarioClass::Instance->Random.Random();
				VocClass::PlayAt(it.at(rnd % it.size()), Coords, nullptr);
			}

			bool debris = false;
			BuildingClass* pBld = pCell->GetBuilding();

			CoordStruct empty;
			ObjectClass* pObj = pCell->FindObjectNearestTo(&empty, false, nullptr);
			bool isInfantry = (pObj && pObj->WhatAmI() == AbstractType::Infantry);

			// empty cell action
			if(!pBld && !pObj) {
				switch(pCell->LandType)
				{
				case LandType::Road:
				case LandType::Rock:
				case LandType::Wall:
				case LandType::Weeds:
					debris = true;
					break;
				default:
				break;
				}
			}

			// account for lightning rods
			int damage = pData->GetDamage();
			if(!pData->Weather_IgnoreLightningRod) {
				if(BuildingClass* pBldObj = specific_cast<BuildingClass*>(pObj)) {
					if(pBldObj->Type->LightningRod) {
						if(BuildingTypeExt::ExtData *pExt = BuildingTypeExt::ExtMap.Find(pBldObj->Type)) {
							// multiply the damage, but never go below zero.
							damage = static_cast<int>(std::max(damage * pExt->LightningRod_Modifier, 0.0));
						}
					}
				}
			}

			// cause mayhem
			if(damage) {
				auto pWarhead = pData->GetWarhead();
				MapClass::FlashbangWarheadAt(damage, pWarhead, Coords, false, SpotlightFlags::None);
				MapClass::DamageArea(Coords, damage, nullptr, pWarhead, true, pSuper->Owner);

				// fancy stuff if damage is dealt
				AnimTypeClass* pAnimType = MapClass::SelectDamageAnimation(damage, pWarhead, pCell->LandType, Coords);
				GameCreate<AnimClass>(pAnimType, Coords);
			}

			// has the last target been destroyed?
			if(pObj != pCell->FindObjectNearestTo(&empty, false, nullptr)) {
				debris = true;
			}

			// create some debris
			if(auto it = pData->Weather_Debris.GetElements(RulesClass::Instance->MetallicDebris)) {

				// dead infantry never generates debris.
				if(!isInfantry && debris) {
					int count = ScenarioClass::Instance->Random.RandomRanged(pData->Weather_DebrisMin, pData->Weather_DebrisMax);
					for(int i=0; i<count; ++i) {
						DWORD rnd = ScenarioClass::Instance->Random.Random();
						AnimTypeClass *pAnimType = it.at(rnd % it.size());

						GameCreate<AnimClass>(pAnimType, Coords);
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
	GET(WarheadTypeClass* const, pWarhead, ESI);
	if(auto const pSuper = SW_LightningStorm::CurrentLightningStorm) {
		auto const pData = SWTypeExt::ExtMap.Find(pSuper->Type);

		if(pData->GetWarhead() == pWarhead) {
			auto const pAnimType = pData->Weather_BoltExplosion.Get(
				RulesClass::Instance->WeatherConBoltExplosion);

			if(pAnimType) {
				R->EAX(pAnimType);
				return 0x48A5AD;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(44C9FF, BuildingClass_Missile_PsiWarn, 6) {
	GET(BuildingClass* const, pThis, ESI);

	auto const type = pThis->FiringSWType;

	if(auto const pSW = SuperWeaponTypeClass::Array->GetItemOrDefault(type)) {
		auto const pExt = SWTypeExt::ExtMap.Find(pSW);
		if(auto const& Anim = pExt->Nuke_PsiWarning) {
			R->EAX(Anim->ArrayIndex);
			Debug::Log("PsiWarn set\n");
			return 0;
		}

		// skip psi warning.
		Debug::Log("PsiWarn skipped\n");
		return 0x44CA7A;
	}

	return 0;
}

// upward pointing missile, launched from missile silo.
DEFINE_HOOK(44CABA, BuildingClass_Missile_CreateBullet, 6) {
	GET(CellClass* const, pCell, EAX);
	GET(BuildingClass* const, pThis, ESI);

	auto const type = pThis->FiringSWType;

	if(auto const pSW = SuperWeaponTypeClass::Array->GetItemOrDefault(type)) {
		if(auto const pWeapon = pSW->WeaponType) {
			auto const pBullet = pWeapon->Projectile->CreateBullet(
				pCell, pThis, pWeapon->Damage, pWeapon->Warhead, 255, true);

			if(pBullet) {
				auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
				pBulletExt->NukeSW = pSW;

				R->EBX(pSW->WeaponType);
				R->EAX(pBullet);

				return 0x44CAF2;
			}
		}
	}

	return 0;
}

// special takeoff anim.
DEFINE_HOOK(44CC8B, BuildingClass_Missile_NukeTakeOff, 6) {
	GET(BuildingClass* const, pThis, ESI);

	auto const type = pThis->FiringSWType;

	if(auto const pSW = SuperWeaponTypeClass::Array->GetItemOrDefault(type)) {
		auto const pExt = SWTypeExt::ExtMap.Find(pSW);

		auto const pAnimType = pExt->Nuke_TakeOff.Get(
			RulesClass::Instance->NukeTakeOff);

		if(pAnimType) {
			R->ECX(pAnimType);
			return 0x44CC91;
		}
	}

	return 0;
}

// remove ZAdjust hardcoding
DEFINE_HOOK(44CC9D, BuildingClass_Missile_NukeTakeOffB, A) {
	GET(AnimClass* const, pAnim, EAX);

	if(!pAnim->ZAdjust) {
		pAnim->ZAdjust = -100;
	}
	return 0x44CCA7;
}

// create a downward pointing missile if the launched one leaves the map.
DEFINE_HOOK(46B371, BulletClass_NukeMaker, 5) {
	GET(BulletClass* const, pThis, EBP);
	auto const pExt = BulletExt::ExtMap.Find(pThis);

	if(auto const pSW = pExt->NukeSW) {
		auto const pSWExt = SWTypeExt::ExtMap.Find(pSW);

		// get the per-SW nuke payload weapon
		if(WeaponTypeClass const* const pPayload = pSWExt->Nuke_Payload) {

			// these are not available during initialisation, so we gotta
			// fall back now if they are invalid.
			auto const damage = pSWExt->GetDamage();
			auto const pWarhead = pSWExt->GetWarhead();

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
			Debug::Log(
				"[%s] has no payload weapon type, or it is invalid.\n",
				pSW->ID);
		}
	}

	return 0;
}

// just puts the launched SW pointer on the downward aiming missile.
DEFINE_HOOK(46B423, BulletClass_NukeMaker_PropagateSW, 6) {
	GET(BulletClass* const, pThis, EBP);
	GET(BulletClass* const, pNuke, EDI);

	auto const pThisExt = BulletExt::ExtMap.Find(pThis);
	auto const pNukeExt = BulletExt::ExtMap.Find(pNuke);
	pNukeExt->NukeSW = pThisExt->NukeSW;

	return 0;
}

// deferred explosion. create a nuke ball anim and, when that is over, go boom.
DEFINE_HOOK(467E59, BulletClass_Update_NukeBall, 5) {
	// changed the hardcoded way to just do this if the warhead is called NUKE
	// to a more universal approach. every warhead can get this behavior.
	GET(BulletClass* const, pThis, EBP);

	auto const pExt = BulletExt::ExtMap.Find(pThis);
	auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pThis->WH);

	enum { Default = 0u, FireNow = 0x467F9Bu, PreImpact = 0x467EB6 };

	auto allowFlash = true;

	// this is a bullet launched by a super weapon
	if(pExt->NukeSW) {
		SW_NuclearMissile::CurrentNukeType = pExt->NukeSW;

		if(pThis->GetHeight() < 0) {
			pThis->SetHeight(0);
		}

		// cause yet another radar event
		auto const pSWTypeExt = SWTypeExt::ExtMap.Find(pExt->NukeSW);
		if(pSWTypeExt->SW_RadarEvent) {
			auto const coords = pThis->GetMapCoords();
			RadarEventClass::Create(
				RadarEventType::SuperweaponActivated, coords);
		}

		allowFlash = pSWTypeExt->Lighting_Enabled;
	}

	// does this create a flash?
	if(allowFlash && pWarheadExt->NukeFlashDuration > 0) {
		// replaces call to NukeFlash::FadeIn

		// manual light stuff
		NukeFlash::Status = NukeFlashStatus::FadeIn;
		ScenarioClass::Instance->AmbientTimer.Start(1);

		// enable the nuke flash
		NukeFlash::StartTime = Unsorted::CurrentFrame;
		NukeFlash::Duration = pWarheadExt->NukeFlashDuration;

		SWTypeExt::ChangeLighting(pExt->NukeSW);
		MapClass::Instance->RedrawSidebar(1);
	}

	if(pWarheadExt->PreImpactAnim != -1) {
		int idxPreImpact = pWarheadExt->PreImpactAnim;
		R->EAX(idxPreImpact);
		return PreImpact;
	}

	return FireNow;
}

// iron curtained units would crush themselves
DEFINE_HOOK(7187DA, TeleportLocomotionClass_Unwarp_PreventSelfCrush, 6) {
	GET(TechnoClass*, pTeleporter, EDI);
	GET(TechnoClass*, pContent, ECX);
	return (pTeleporter == pContent) ? 0x71880A : 0;
}

// sink stuff that simply cannot exist on water
DEFINE_HOOK(7188F2, TeleportLocomotionClass_Unwarp_SinkJumpJets, 7) {
	GET(CellClass*, pCell, EAX);
	GET(TechnoClass**, pTechno, ESI);

	if(pCell->Tile_Is_Wet()) {
		if(UnitClass* pUnit = specific_cast<UnitClass*>(pTechno[3])) {
			if(pUnit->Deactivated) {
				// this thing does not float
				R->BL(0);
			}

			// manually sink it
			if(pUnit->Type->SpeedType == SpeedType::Hover && pUnit->Type->JumpJet) {
				return 0x718A66;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(446AAF, BuildingClass_Place_SkipFreeUnits, 6)
{
	// allow free units and non-separate aircraft to be created
	// only once.
	GET(BuildingClass*, pBld, EBP);
	BuildingExt::ExtData* pExt = BuildingExt::ExtMap.Find(pBld);
	if(!pExt->FreeUnits_Done) {
		pExt->FreeUnits_Done = true;
		return 0;
	}

	// skip handling free units
	return 0x446FB6;
}

DEFINE_HOOK(71AE85, TemporalClass_CanWarpTarget_PreventChronoBuilding, A)
{
	// prevent warping buildings that are about to be chronoshifted.
	// if such building is attacked, it will be removed by the chronosphere
	// and it won't come back and the affected player can't be defeated.
	GET(BuildingClass*, pBld, ESI);
	if(BuildingExt::ExtData* pExt = BuildingExt::ExtMap.Find(pBld)) {
		if(pExt->AboutToChronoshift) {
			return 0x71AE93;
		}
	}

	return 0;
}

DEFINE_HOOK(44CE46, BuildingClass_Mi_Missile_Pulsball, 5)
{
	GET(BuildingClass*, pThis, ESI);
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	auto pPulseBall = AnimTypeClass::Find("PULSBALL");
	auto delay = 32;

	if(auto pSuper = pExt->SuperWeapon) {
		auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);
		pPulseBall = pSWExt->EMPulse_PulseBall.Get(pPulseBall);
		delay = pSWExt->EMPulse_PulseDelay;
	}

	// also support no pulse ball
	if(pPulseBall) {
		auto flh = pThis->GetFLH(0, CoordStruct::Empty);
		GameCreate<AnimClass>(pPulseBall, flh);
	}

	pThis->MissionStatus = 2;
	R->EAX(delay);
	return 0x44CEC2;
}

DEFINE_HOOK(44CCE7, BuildingClass_Mi_Missile_GenericSW, 6)
{
	GET(BuildingClass*, pThis, ESI);

	// added this check so this is mutually exclusive
	if(pThis->Type->EMPulseCannon) {
		return 0x44CD18;
	}

	// originally, this part was related to chem missiles
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	auto pTarget = pExt->SuperTarget ? pExt->SuperTarget
		: MapClass::Instance->GetCellAt(pThis->Owner->NukeTarget);

	pThis->Fire(pTarget, 0);
	pThis->QueueMission(Mission::Guard, false);

	R->EAX(1);
	return 0x44D599;
}
