#include "Body.h"
#include "../Rules/Body.h"
#include "../TechnoType/Body.h"

#include <FlyLocomotionClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <YRMath.h>

DEFINE_HOOK(4CCB84, FlyLocomotionClass_ILocomotion_Process_HunterSeeker, 6)
{
	GET(ILocomotion*, pThis, ESI);
	auto pLoco = static_cast<FlyLocomotionClass*>(pThis);
	auto pObject = pLoco->LinkedTo;
	auto pType = pObject->GetTechnoType();

	if(pType->HunterSeeker) {
		if(!pObject->Target) {
			pLoco->Acquire_Hunter_Seeker_Target();

			if(pObject->Target) {
				pLoco->IsLanding = false;
				pLoco->FlightLevel = pType->GetFlightLevel();

				pObject->SendToFirstLink(rc_Exit);
				pObject->QueueMission(Mission::Attack, false);
				pObject->NextMission();
			}
		}
	}

	return 0;
}

DEFINE_HOOK(4CE85A, FlyLocomotionClass_UpdateLanding, 8)
{
	GET(FlyLocomotionClass*, pThis, ESI);
	auto pObject = pThis->LinkedTo;
	auto pType = pObject->GetTechnoType();

	if(pType->HunterSeeker) {

		if(!pObject->Target) {
			pThis->Acquire_Hunter_Seeker_Target();

			if(pObject->Target) {
				pThis->IsLanding = false;
				pThis->FlightLevel = pType->GetFlightLevel();

				pObject->SendToFirstLink(rc_Exit);
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
	GET_STACK(FlyLocomotionClass*, pThis, 0x20);
	auto pObject = pThis->LinkedTo;
	auto pType = pObject->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	if(pType->HunterSeeker) {
		if(auto pTarget = pObject->Target) {
			int DetonateProximity = pExt->HunterSeekerDetonateProximity.Get(RulesExt::Global()->HunterSeekerDetonateProximity);
			int DescendProximity = pExt->HunterSeekerDescendProximity.Get(RulesExt::Global()->HunterSeekerDescendProximity);

			// get th difference of our position to the target,
			// disregarding the Z component.
			CoordStruct crd = pObject->GetCoords();
			crd -= pThis->MovingDestination;
			crd.Z = 0;

			int dist = Game::F2I(crd.Magnitude());

			if(dist >= DetonateProximity) {
				// not close enough to detonate, but we might start the decent
				if(dist < DescendProximity) {
					// the target's current height
					int z = pTarget->GetCoords().Z;

					// the hunter seeker's default flight level
					crd = pObject->GetCoords();
					int floor = MapClass::Instance->GetCellFloorHeight(crd);
					int height = floor + pType->GetFlightLevel();

					// linear interpolation between target's Z and normal flight level
					double ratio = dist / static_cast<double>(DescendProximity);
					double lerp = z * (1.0 - ratio) + height * ratio;

					// set the descending flight level
					int level = Game::F2I(lerp) - floor;
					if(level < 10) {
						level = 10;
					}

					pThis->FlightLevel = level;

					return 0x4CF4D2;
				}

				// project the next steps using the current speed
				// and facing. if there's a height difference, use
				// the highest value as the new flight level.
				int speed = pThis->Apparent_Speed();
				if(speed > 0) {
					double value = pObject->Facing.current().radians();
					double cos = Math::cos(value);
					double sin = Math::sin(value);

					int maxHeight = 0;
					int currentHeight = 0;
					crd = pObject->GetCoords();
					for(int i = 0; i < 11; ++i) {
						CellClass* pCell = MapClass::Instance->GetCellAt(crd);
						int z = pCell->GetCoordsWithBridge().Z;

						if(z > maxHeight) {
							maxHeight = z;
						}

						if(!i) {
							currentHeight = z;
						}

						// advance one step
						crd.X = Game::F2I(crd.X + cos * speed);
						crd.Y = Game::F2I(crd.Y - sin * speed);

						// result is never used in TS, but a break sounds
						// like a good idea.
						CellStruct cell = CellClass::Coord2Cell(crd);
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
				if(auto pTechno = abstract_cast<TechnoClass*>(pTarget)) {
					WeaponTypeClass* pWeapon = pObject->GetWeapon(0)->WeaponType;

					// damage the target
					int damage = pWeapon->Damage;
					pTechno->ReceiveDamage(&damage, 0, pWeapon->Warhead, pObject, true, true, nullptr);

					// damage the hunter seeker
					damage = pWeapon->Damage;
					pObject->ReceiveDamage(&damage, 0, pWeapon->Warhead, nullptr, true, true, nullptr);

					// damage the map
					crd = pObject->GetCoords();
					MapClass::FlashbangWarheadAt(pWeapon->Damage, RulesClass::Instance->C4Warhead, crd);
					MapClass::DamageArea(crd, pWeapon->Damage, pObject, pWeapon->Warhead, true, nullptr);

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
	GET(FlyLocomotionClass*, pThis, ESI);
	auto pObject = pThis->LinkedTo;
	auto pType = pObject->GetTechnoType();

	if(pType->HunterSeeker) {
		if(auto pTarget = pObject->Target) {

			// update the target's position, considering units in tunnels
			CoordStruct crd = pTarget->GetCoords();

			if(auto pFoot = abstract_cast<FootClass*>(pObject)) {
				AbstractType abs = pTarget->WhatAmI();
				if(abs == UnitClass::AbsID || abs == InfantryClass::AbsID) {
					if(pFoot->TubeIndex >= 0) {
						crd = pFoot->unknown_coords_568;
					}
				}
			}

			int height = MapClass::Instance->GetCellFloorHeight(crd);
			if(crd.Z < height) {
				crd.Z = height;
			}

			pThis->MovingDestination = crd;

			// update the facing
			crd = pObject->GetCoords();
			double value = Math::arctanfoo(crd.Y - pThis->MovingDestination.Y, pThis->MovingDestination.X - crd.X);
			DirStruct::value_type facing = static_cast<DirStruct::value_type>(Game::F2I((value - 1.570796326794897) * -10430.06004058427));

			DirStruct tmp(facing);
			pObject->Facing.set(tmp);
			pObject->TurretFacing.set(tmp);
		}
	}

	return 0;
}

DEFINE_HOOK(4CDE64, FlyLocomotionClass_sub_4CD600_HunterSeeker_Ascent, 6)
{
	GET(FlyLocomotionClass*, pThis, ESI);
	GET(int, unk, EDI);
	auto pObject = pThis->LinkedTo;
	auto pType = pObject->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	int ret = pThis->FlightLevel - unk;
	int max = 16;

	if(!pType->IsDropship) {
		if (!pType->HunterSeeker) {
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
	GET(FlyLocomotionClass*, pThis, ESI);
	GET(int, max, EDI);
	auto pObject = pThis->LinkedTo;
	auto pType = pObject->GetTechnoType();
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	if(pType->HunterSeeker) {
		int ret = pExt->HunterSeekerDescentSpeed.Get(RulesExt::Global()->HunterSeekerDescentSpeed);
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
	GET_STACK(ILocomotion*, pThis, 0x4);
	auto pLoco = static_cast<FlyLocomotionClass*>(pThis);
	auto pObject = pLoco->LinkedTo;
	auto pExt = TechnoExt::ExtMap.Find(pObject);

	// replace the entire function
	pExt->AcquireHunterSeekerTarget();

	return 0x4D016F;
}

DEFINE_HOOK(4D8D95, FootClass_UpdatePosition_HunterSeeker, A)
{
	GET(FootClass*, pThis, ESI);

	// ensure the target won't get away
	if(pThis->GetTechnoType()->HunterSeeker) {
		if(auto pTarget = abstract_cast<TechnoClass*>(pThis->Target)) {
			auto pWeapon = pThis->GetWeapon(0)->WeaponType;
			int damage = pWeapon->Damage;
			pTarget->ReceiveDamage(&damage, 0, pWeapon->Warhead, pThis, true, true, nullptr);
		}
	}

	return 0;
}
