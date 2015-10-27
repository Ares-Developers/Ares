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
	const auto pSuper = SW_LightningStorm::CurrentLightningStorm;

	if(!pSuper) {
		// legacy way still needed for triggers.
		return 0;
	}

	GET(int const, duration, ECX);
	GET(int const, deferment, EDX);
	GET_STACK(CellStruct, cell, 0x4);
	GET_STACK(HouseClass* const, pOwner, 0x8);

	auto const pType = pSuper->Type;
	auto const pExt = SWTypeExt::ExtMap.Find(pType);

	auto ret = false;

	// generate random cell if the passed ones are empty
	if(cell == CellStruct::Empty) {
		auto const& Bounds = MapClass::Instance->MapCoordBounds;
		auto& Random = ScenarioClass::Instance->Random;
		while(!MapClass::Instance->CellExists(cell)) {
			cell.X = static_cast<short>(Random.RandomRanged(0, Bounds.Right));
			cell.Y = static_cast<short>(Random.RandomRanged(0, Bounds.Bottom));
		}
	}

	// yes. set them even if the Lightning Storm is active.
	LightningStorm::Coords = cell;
	LightningStorm::Owner = pOwner;
		
	if(!LightningStorm::Active) {
		if(deferment) {
			// register this storm to start soon
			if(!LightningStorm::Deferment
				|| LightningStorm::Deferment >= deferment)
			{
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
			auto const outage = pExt->Weather_RadarOutage.Get(
				RulesClass::Instance->LightningStormDuration);
			if(outage > 0) {
				for(auto const& pHouse : *HouseClass::Array) {
					if(pExt->IsHouseAffected(
						pOwner, pHouse, pExt->Weather_RadarOutageAffects))
					{
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
			if(pExt->Weather_PrintText.Get(
				RulesClass::Instance->LightningPrintText))
			{
				pExt->PrintMessage(pExt->Message_Activate, pSuper->Owner);
			}

			auto const sound = pExt->SW_ActivationSound.Get(
				RulesClass::Instance->StormSound);
			if(sound != -1) {
				VocClass::PlayGlobal(sound, 0x2000, 1.0);
			}

			if(pExt->SW_RadarEvent) {
				RadarEventClass::Create(
					RadarEventType::SuperweaponActivated, cell);
			}

			MapClass::Instance->RedrawSidebar(1);
		}
	}

	R->EAX(ret);
	return 0x539F80;
}

// this is a complete rewrite of LightningStorm::Update.
DEFINE_HOOK(53A6CF, LightningStorm_Update, 7) {
	enum { Legacy = 0x53A8FFu, Handled = 0x53AB45u };

	auto const currentFrame = Unsorted::CurrentFrame;

	// switch lightning for nuke
	if(NukeFlash::Duration != -1) {
		if(NukeFlash::StartTime + NukeFlash::Duration < currentFrame) {
			if(NukeFlash::IsFadingIn()) {
				NukeFlash::Status = NukeFlashStatus::FadeOut;
				NukeFlash::StartTime = currentFrame;
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
	for(auto i = LightningStorm::BoltsPresent->Count - 1; i >= 0; --i) {
		if(auto const pAnim = LightningStorm::BoltsPresent->Items[i]) {
			if(pAnim->Animation.Value >= pAnim->Type->GetImage()->Frames / 2) {
				LightningStorm::BoltsPresent->RemoveItem(i);
			}
		}
	}

	// find the clouds that should strike right now
	for(auto i = LightningStorm::CloudsManifesting->Count - 1; i >= 0; --i) {
		if(auto const pAnim = LightningStorm::CloudsManifesting->Items[i]) {
			if(pAnim->Animation.Value >= pAnim->Type->GetImage()->Frames / 2) {
				auto const crdStrike = pAnim->GetCoords();
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
		for(auto i = LightningStorm::CloudsPresent->Count - 1; i >= 0; --i) {
			if(auto const pAnim = LightningStorm::CloudsPresent->Items[i]) {
				auto pAnimImage = pAnim->Type->GetImage();
				if(pAnim->Animation.Value >= pAnimImage->Frames - 1) {
					LightningStorm::CloudsPresent->RemoveItem(i);
				}
			}
		}
	}

	// check for presence of Ares SW
	auto const pSuper = SW_LightningStorm::CurrentLightningStorm;

	if(!pSuper) {
		// still support old logic for triggers
		return Legacy;
	}

	auto const coords = LightningStorm::Coords;

	auto const pType = pSuper->Type;
	auto const pExt = SWTypeExt::ExtMap.Find(pType);

	// is inactive
	if(!LightningStorm::Active || LightningStorm::TimeToEnd) {
		auto deferment = LightningStorm::Deferment;

		// still counting down?
		if(deferment > 0) {
			--deferment;
			LightningStorm::Deferment = deferment;

			// still waiting
			if(deferment) {
				if(deferment % 225 == 0) {
					if(pExt->Weather_PrintText.Get(
						RulesClass::Instance->LightningPrintText))
					{
						pExt->PrintMessage(pExt->Message_Launch, pSuper->Owner);
					}
				}
			} else {
				// launch the storm
				LightningStorm::Start(
					LightningStorm::Duration, 0, coords, LightningStorm::Owner);
			}
		}

		return Handled;
	}

	// does this Lightning Storm go on?
	auto const duration = LightningStorm::Duration;
	if(duration != -1 && duration + LightningStorm::StartTime < currentFrame) {
		// it's over already
		LightningStorm::TimeToEnd = true;
		return Handled;
	}

	// deterministic damage. the very target cell.
	auto const hitDelay = pExt->Weather_HitDelay.Get(
		RulesClass::Instance->LightningHitDelay);
	if(hitDelay > 0 && currentFrame % hitDelay == 0) {
		LightningStorm::Strike(coords);
	}

	// random damage. somewhere in range.
	auto const scatterDelay = pExt->Weather_ScatterDelay.Get(
		RulesClass::Instance->LightningScatterDelay);
	if(scatterDelay > 0 && currentFrame % scatterDelay == 0) {
		auto const range = pExt->GetRange();
		auto const isRectangle = (range.height() <= 0);
		auto const width = range.width();
		auto const height = isRectangle ? width : range.height();

		auto const GetRandomCoords = [=]() {
			auto& Random = ScenarioClass::Instance->Random;
			auto const offsetX = Random.RandomRanged(-width / 2, width / 2);
			auto const offsetY = Random.RandomRanged(-height / 2, height / 2);
			auto const ret = coords + CellStruct{
				static_cast<short>(offsetX), static_cast<short>(offsetY) };

			// don't even try if this is invalid
			if(!MapClass::Instance->CellExists(ret)) {
				return CellStruct::Empty;
			}

			// out of range?
			if(!isRectangle && ret.DistanceFrom(coords) > range.WidthOrRange) {
				return CellStruct::Empty;
			}

			// if we respect lightning rods, start looking for one.
			if(!pExt->Weather_IgnoreLightningRod) {
				// if, by coincidence, this is a rod, hit it.
				auto const pCell = MapClass::Instance->GetCellAt(ret);
				auto const pCellBld = pCell->GetBuilding();

				if(pCellBld && pCellBld->Type->LightningRod) {
					return ret;
				}

				// if a lightning rod is next to this, hit that instead. naive.
				if(auto const pObj = pCell->FindTechnoNearestTo(
					Point2D::Empty, false, pCellBld))
				{
					if(auto const pBld = specific_cast<BuildingClass*>(pObj)) {
						if(pBld->Type->LightningRod) {
							return pBld->GetMapCoords();
						}
					}
				}
			}

			// is this spot far away from another cloud?
			auto const separation = pExt->Weather_Separation.Get(
				RulesClass::Instance->LightningSeparation);
			if(separation > 0) {
				// assume success and disprove.
				for(auto const& pCloud : *LightningStorm::CloudsPresent) {
					auto const cellCloud = pCloud->GetMapCoords();
					auto const dist = std::abs(cellCloud.X - ret.X)
						+ std::abs(cellCloud.Y - ret.Y);

					if(dist < separation) {
						return CellStruct::Empty;
					}
				}
			}

			return ret;
		};

		// generate a new place to strike
		if(height > 0 && width > 0 && MapClass::Instance->CellExists(coords)) {
			for(int k = pExt->Weather_ScatterCount; k > 0; --k) {
				auto const cell = GetRandomCoords();
				if(cell != CellStruct::Empty) {
					// found a valid position. strike there.
					LightningStorm::Strike(cell);
					break;
				}
			}
		}
	}

	// jump over everything
	return Handled;
}

// create a cloud.
DEFINE_HOOK(53A140, LightningStorm_Strike, 7) {
	if(auto const pSuper = SW_LightningStorm::CurrentLightningStorm) {
		GET_STACK(CellStruct const, cell, 0x4);

		auto const pType = pSuper->Type;
		auto const pExt = SWTypeExt::ExtMap.Find(pType);

		// get center of cell coords
		auto const pCell = MapClass::Instance->GetCellAt(cell);
		auto coords = pCell->GetCoordsWithBridge();

		// create a cloud animation
		if(coords != CoordStruct::Empty) {
			// select the anim
			auto const itClouds = pExt->Weather_Clouds.GetElements(
				RulesClass::Instance->WeatherConClouds);
			auto const pAnimType = itClouds.at(
				ScenarioClass::Instance->Random.Random() % itClouds.size());

			// infer the height this thing will be drawn at.
			if(pExt->Weather_CloudHeight < 0) {
				if(auto const itBolts = pExt->Weather_Bolts.GetElements(
					RulesClass::Instance->WeatherConBolts))
				{
					auto const pBoltAnim = itBolts.at(0);
					pExt->Weather_CloudHeight = Game::F2I(
						((pBoltAnim->GetImage()->Height / 2) - 0.5)
						* LightningStorm::CloudHeightFactor);
				}
			}
			coords.Z += pExt->Weather_CloudHeight;

			// create the cloud and do some book keeping.
			if(auto const pAnim = GameCreate<AnimClass>(pAnimType, coords)) {
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
	auto const pSuper = SW_LightningStorm::CurrentLightningStorm;

	if(!pSuper) {
		// legacy way for triggers
		return 0;
	}

	REF_STACK(CoordStruct const, refCoords, 0x4);

	auto const pType = pSuper->Type;
	auto const pData = SWTypeExt::ExtMap.Find(pType);

	// get center of cell coords
	auto const pCell = MapClass::Instance->GetCellAt(refCoords);
	auto const coords = pCell->GetCoordsWithBridge();

	if(coords != CoordStruct::Empty) {

		// create a bolt animation
		if(auto it = pData->Weather_Bolts.GetElements(
			RulesClass::Instance->WeatherConBolts))
		{
			auto const rnd = ScenarioClass::Instance->Random.Random();
			auto const pAnimType = it.at(rnd % it.size());

			if(auto const pAnim = GameCreate<AnimClass>(pAnimType, coords)) {
				LightningStorm::BoltsPresent->AddItem(pAnim);
			}
		}

		// play lightning sound
		if(auto const it = pData->Weather_Sounds.GetElements(
			RulesClass::Instance->LightningSounds))
		{
			auto const rnd = ScenarioClass::Instance->Random.Random();
			VocClass::PlayAt(it.at(rnd % it.size()), coords, nullptr);
		}

		auto debris = false;
		auto const pBld = pCell->GetBuilding();

		auto const& empty = Point2D::Empty;
		auto const pObj = pCell->FindTechnoNearestTo(empty, false, nullptr);
		auto const isInfantry = abstract_cast<InfantryClass*>(pObj) != nullptr;

		// empty cell action
		if(!pBld && !pObj) {
			debris = Helpers::Alex::is_any_of(
				pCell->LandType,
				LandType::Road,
				LandType::Rock,
				LandType::Wall,
				LandType::Weeds);
		}

		// account for lightning rods
		auto damage = pData->GetDamage();
		if(!pData->Weather_IgnoreLightningRod) {
			if(auto const pBldObj = abstract_cast<BuildingClass*>(pObj)) {
				auto const pBldType = pBldObj->Type;
				if(pBldType->LightningRod) {
					// multiply the damage, but never go below zero.
					auto const pBldExt = BuildingTypeExt::ExtMap.Find(pBldType);
					damage = Math::max(static_cast<int>(
						damage * pBldExt->LightningRod_Modifier), 0);
				}
			}
		}

		// cause mayhem
		if(damage) {
			auto const pWarhead = pData->GetWarhead();
			MapClass::FlashbangWarheadAt(
				damage, pWarhead, coords, false, SpotlightFlags::None);
			MapClass::DamageArea(
				coords, damage, nullptr, pWarhead, true, pSuper->Owner);

			// fancy stuff if damage is dealt
			auto const pAnimType = MapClass::SelectDamageAnimation(
				damage, pWarhead, pCell->LandType, coords);
			GameCreate<AnimClass>(pAnimType, coords);
		}

		// has the last target been destroyed?
		if(pObj != pCell->FindTechnoNearestTo(empty, false, nullptr)) {
			debris = true;
		}

		// create some debris
		if(auto const it = pData->Weather_Debris.GetElements(
			RulesClass::Instance->MetallicDebris))
		{
			// dead infantry never generates debris
			if(!isInfantry && debris) {
				auto const count = ScenarioClass::Instance->Random.RandomRanged(
					pData->Weather_DebrisMin, pData->Weather_DebrisMax);

				for(int i = 0; i < count; ++i) {
					auto const rnd = ScenarioClass::Instance->Random.Random();
					auto const pAnimType = it.at(rnd % it.size());

					GameCreate<AnimClass>(pAnimType, coords);
				}
			}
		}
	}

	return 0x53A69A;
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
	auto flashDuration = 0;

	// this is a bullet launched by a super weapon
	if(pExt->NukeSW && !pThis->WH->NukeMaker) {
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
		flashDuration = 30;
	}

	// does this create a flash?
	auto const duration = pWarheadExt->NukeFlashDuration.Get(flashDuration);

	if(allowFlash && duration > 0) {
		// replaces call to NukeFlash::FadeIn

		// manual light stuff
		NukeFlash::Status = NukeFlashStatus::FadeIn;
		ScenarioClass::Instance->AmbientTimer.Start(1);

		// enable the nuke flash
		NukeFlash::StartTime = Unsorted::CurrentFrame;
		NukeFlash::Duration = duration;

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
	GET(BuildingClass* const, pThis, ESI);

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const pSuperTarget = pExt->SuperTarget;

	// added this check so this is mutually exclusive
	if(pThis->Type->EMPulseCannon) {
		if(pSuperTarget) {
			// support multiple simultaneously firing super weapons by
			// re-setting the house-global coords for this super weapon
			auto const cell = CellClass::Coord2Cell(pSuperTarget->GetCoords());
			pThis->Owner->EMPTarget = cell;
		}
		return 0x44CD18;
	}

	// originally, this part was related to chem missiles
	auto const pTarget = pSuperTarget ? pSuperTarget
		: MapClass::Instance->GetCellAt(pThis->Owner->NukeTarget);

	pThis->Fire(pTarget, 0);
	pThis->QueueMission(Mission::Guard, false);

	R->EAX(1);
	return 0x44D599;
}
