#include "Body.h"
#include "../Rules/Body.h"
#include "../TechnoType/Body.h"

#include <FlyLocomotionClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <YRMath.h>

DEFINE_HOOK(4CCB84, FlyLocomotionClass_ILocomotion_Process_HunterSeeker, 6)
{
	GET(ILocomotion* const, pThis, ESI);
	auto const pLoco = static_cast<FlyLocomotionClass*>(pThis);
	auto const pObject = pLoco->LinkedTo;
	auto const pType = pObject->GetTechnoType();

	if(pType->HunterSeeker) {
		if(!pObject->Target) {
			pLoco->Acquire_Hunter_Seeker_Target();

			if(pObject->Target) {
				pLoco->IsLanding = false;
				pLoco->FlightLevel = pType->GetFlightLevel();

				pObject->SendToFirstLink(RadioCommand::NotifyUnlink);
				pObject->QueueMission(Mission::Attack, false);
				pObject->NextMission();
			}
		}
	}

	return 0;
}

DEFINE_HOOK(4CE85A, FlyLocomotionClass_UpdateLanding, 8)
{
	GET(FlyLocomotionClass* const, pThis, ESI);
	auto const pObject = pThis->LinkedTo;
	auto const pType = pObject->GetTechnoType();

	if(pType->HunterSeeker) {
		if(!pObject->Target) {
			pThis->Acquire_Hunter_Seeker_Target();

			if(pObject->Target) {
				pThis->IsLanding = false;
				pThis->FlightLevel = pType->GetFlightLevel();

				pObject->SendToFirstLink(RadioCommand::NotifyUnlink);
				pObject->QueueMission(Mission::Attack, false);
				pObject->NextMission();
			}
		}

		// return 0
		R->EAX(0);
		return 0x4CE852;
	}

	return 0;
}

DEFINE_HOOK(4CF3D0, FlyLocomotionClass_sub_4CEFB0_HunterSeeker, 7)
{
	GET_STACK(FlyLocomotionClass* const, pThis, 0x20);
	auto const pObject = pThis->LinkedTo;
	auto const pType = pObject->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	if(pType->HunterSeeker) {
		if(auto const pTarget = pObject->Target) {
			auto const DetonateProximity = pExt->HunterSeekerDetonateProximity.Get(RulesExt::Global()->HunterSeekerDetonateProximity);
			auto const DescendProximity = pExt->HunterSeekerDescendProximity.Get(RulesExt::Global()->HunterSeekerDescendProximity);

			// get th difference of our position to the target,
			// disregarding the Z component.
			auto crd = pObject->GetCoords();
			crd -= pThis->MovingDestination;
			crd.Z = 0;

			auto const dist = Game::F2I(crd.Magnitude());

			if(dist >= DetonateProximity) {
				// not close enough to detonate, but we might start the decent
				if(dist < DescendProximity) {
					// the target's current height
					auto const z = pTarget->GetCoords().Z;

					// the hunter seeker's default flight level
					crd = pObject->GetCoords();
					auto floor = MapClass::Instance->GetCellFloorHeight(crd);
					auto const height = floor + pType->GetFlightLevel();

					// linear interpolation between target's Z and normal flight level
					auto const ratio = dist / static_cast<double>(DescendProximity);
					auto const lerp = z * (1.0 - ratio) + height * ratio;

					// set the descending flight level
					auto level = Game::F2I(lerp) - floor;
					if(level < 10) {
						level = 10;
					}

					pThis->FlightLevel = level;

					return 0x4CF4D2;
				}

				// project the next steps using the current speed
				// and facing. if there's a height difference, use
				// the highest value as the new flight level.
				auto const speed = pThis->Apparent_Speed();
				if(speed > 0) {
					double const value = pObject->Facing.current().radians();
					double const cos = Math::cos(value);
					double const sin = Math::sin(value);

					int maxHeight = 0;
					int currentHeight = 0;
					auto crd2 = pObject->GetCoords();
					for(int i = 0; i < 11; ++i) {
						auto const pCell = MapClass::Instance->GetCellAt(crd2);
						auto const z = pCell->GetCoordsWithBridge().Z;

						if(z > maxHeight) {
							maxHeight = z;
						}

						if(!i) {
							currentHeight = z;
						}

						// advance one step
						crd2.X += Game::F2I(cos * speed);
						crd2.Y -= Game::F2I(sin * speed);

						// result is never used in TS, but a break sounds
						// like a good idea.
						auto const cell = CellClass::Coord2Cell(crd2);
						if(!MapClass::Instance->CoordinatesLegal(cell)) {
							break;
						}
					}

					// pull the old lady up
					if(maxHeight > currentHeight) {
						pThis->FlightLevel = pType->GetFlightLevel();
						return 0x4CF4D2;
					}
				}

			} else {
				// close enough to detonate
				if(auto const pTechno = abstract_cast<TechnoClass*>(pTarget)) {
					auto const pWeapon = pObject->GetWeapon(0)->WeaponType;

					// damage the target
					auto damage = pWeapon->Damage;
					pTechno->ReceiveDamage(&damage, 0, pWeapon->Warhead, pObject, true, true, nullptr);

					// damage the hunter seeker
					damage = pWeapon->Damage;
					pObject->ReceiveDamage(&damage, 0, pWeapon->Warhead, nullptr, true, true, nullptr);

					// damage the map
					auto const crd2 = pObject->GetCoords();
					MapClass::FlashbangWarheadAt(pWeapon->Damage, RulesClass::Instance->C4Warhead, crd2);
					MapClass::DamageArea(crd2, pWeapon->Damage, pObject, pWeapon->Warhead, true, nullptr);

					// return 0
					R->EBX(0);
					return 0x4CF5F2;
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(4CD9C8, FlyLocomotionClass_sub_4CD600_HunterSeeker_UpdateTarget, 6)
{
	GET(FlyLocomotionClass* const, pThis, ESI);
	auto const pObject = pThis->LinkedTo;
	auto const pType = pObject->GetTechnoType();

	if(pType->HunterSeeker) {
		if(auto const pTarget = pObject->Target) {

			// update the target's position, considering units in tunnels
			auto crd = pTarget->GetCoords();

			if(auto const pFoot = abstract_cast<FootClass*>(pObject)) {
				auto const abs = pTarget->WhatAmI();
				if(abs == UnitClass::AbsID || abs == InfantryClass::AbsID) {
					if(pFoot->TubeIndex >= 0) {
						crd = pFoot->unknown_coords_568;
					}
				}
			}

			auto const height = MapClass::Instance->GetCellFloorHeight(crd);
			if(crd.Z < height) {
				crd.Z = height;
			}

			pThis->MovingDestination = crd;

			// update the facing
			auto const crdSource = pObject->GetCoords();
			auto const value = Math::arctanfoo(crdSource.Y - crd.Y, crd.X - crdSource.X);

			DirStruct const tmp(value);
			pObject->Facing.set(tmp);
			pObject->TurretFacing.set(tmp);
		}
	}

	return 0;
}

DEFINE_HOOK(4CDE64, FlyLocomotionClass_sub_4CD600_HunterSeeker_Ascent, 6)
{
	GET(FlyLocomotionClass* const, pThis, ESI);
	GET(int const, unk, EDI);
	auto const pObject = pThis->LinkedTo;
	auto const pType = pObject->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	auto ret = pThis->FlightLevel - unk;
	auto max = 16;

	if(!pType->IsDropship) {
		if(!pType->HunterSeeker) {
			// ordinary aircraft
			max = (R->BL() != 0) ? 10 : 20;

		} else {
			// is hunter seeker
			if(pThis->IsTakingOff) {
				max = pExt->HunterSeekerEmergeSpeed.Get(RulesExt::Global()->HunterSeekerEmergeSpeed);
			} else {
				max = pExt->HunterSeekerAscentSpeed.Get(RulesExt::Global()->HunterSeekerAscentSpeed);
			}
		}
	}

	if(ret > max) {
		ret = max;
	}

	R->EAX(ret);
	return 0x4CDE8F;
}

DEFINE_HOOK(4CDF54, FlyLocomotionClass_sub_4CD600_HunterSeeker_Descent, 5)
{
	GET(FlyLocomotionClass* const, pThis, ESI);
	GET(int const, max, EDI);
	auto const pObject = pThis->LinkedTo;
	auto const pType = pObject->GetTechnoType();
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	if(pType->HunterSeeker) {
		auto ret = pExt->HunterSeekerDescentSpeed.Get(RulesExt::Global()->HunterSeekerDescentSpeed);
		if(max < ret) {
			ret = max;
		}

		R->ECX(ret);
		return 0x4CDF81;
	}

	return 0;
}

DEFINE_HOOK(4CFE80, FlyLocomotionClass_ILocomotion_AcquireHunterSeekerTarget, 5)
{
	GET_STACK(ILocomotion* const, pThis, 0x4);
	auto const pLoco = static_cast<FlyLocomotionClass*>(pThis);
	auto const pObject = pLoco->LinkedTo;
	auto const pExt = TechnoExt::ExtMap.Find(pObject);

	// replace the entire function
	pExt->AcquireHunterSeekerTarget();

	return 0x4D016F;
}

DEFINE_HOOK(4D8D95, FootClass_UpdatePosition_HunterSeeker, A)
{
	GET(FootClass* const, pThis, ESI);

	// ensure the target won't get away
	if(pThis->GetTechnoType()->HunterSeeker) {
		if(auto const pTarget = abstract_cast<TechnoClass*>(pThis->Target)) {
			auto const pWeapon = pThis->GetWeapon(0)->WeaponType;
			auto damage = pWeapon->Damage;
			pTarget->ReceiveDamage(&damage, 0, pWeapon->Warhead, pThis, true, true, nullptr);
		}
	}

	return 0;
}
