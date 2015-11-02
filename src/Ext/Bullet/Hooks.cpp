#include <BulletClass.h>
#include <BuildingClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include "../BulletType/Body.h"
#include "../Building/Body.h"
#include "../BuildingType/Body.h"
#include "../House/Body.h"
#include "../WeaponType/Body.h"
#include <ScenarioClass.h>
#include <YRMath.h>
#include <Helpers/Iterators.h>

#include "../../Misc/TrajectoryHelper.h"

DEFINE_HOOK(468BE2, BulletClass_ShouldDetonate_Obstacle, 6)
{
	GET(BulletClass* const, pThis, ESI);
	GET(CoordStruct* const, pOutCoords, EDI);

	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);

	if(AresTrajectoryHelper::SubjectToAnything(pThis->Type, pTypeExt)) {
		auto const Map = MapClass::Instance;
		auto const pCellSource = Map->GetCellAt(pThis->SourceCoords);
		auto const pCellTarget = Map->GetCellAt(pThis->TargetCoords);
		auto const pCellLast = Map->GetCellAt(pThis->LastMapCoords);

		auto const pOwner = pThis->Owner ? pThis->Owner->Owner : nullptr;

		if(AresTrajectoryHelper::GetObstacle(
			pCellSource, pCellTarget, pThis->Owner, pThis->Target, pCellLast,
			*pOutCoords, pThis->Type, pTypeExt, pOwner))
		{
			return 0x468C76;
		}
	}

	return 0x468C86;
}

DEFINE_HOOK(46867F, BulletClass_SetMovement_Parachute, 5)
{
	GET(CoordStruct *, XYZ, EAX);
	GET(BulletClass *, Bullet, ECX);
//	GET_BASE(BulletVelocity *, Trajectory, 0xC);

	R->EBX<BulletClass *>(Bullet);

	BulletTypeExt::ExtData *pBulletData = BulletTypeExt::ExtMap.Find(Bullet->Type);

//	Debug::Log("Bullet [%s] is parachuted (%d)\n", Bullet->Type->get_ID(), pBulletData->Parachuted.Get());

	bool result = false;
	if(pBulletData->Parachuted) {
		result = Bullet->SpawnParachuted(*XYZ);
//		Debug::Log("Bullet trajectory is (%lf, %lf, %lf)\n", *Trajectory);
		Bullet->IsABomb = true;
	} else {
		result = Bullet->Put(*XYZ, 0);
	}

	R->EAX(result);
	return 0x468689;
}

DEFINE_HOOK(4688BD, BulletClass_SetMovement_Obstacle, 6)
{
	GET(BulletClass* const, pThis, EBX);
	GET(CoordStruct const* const, pLocation, EDI);
	REF_STACK(CoordStruct const, dest, STACK_OFFS(0x54, 0x10));

	auto const pOwner = pThis->Owner ? pThis->Owner->Owner : nullptr;

	// code must use pLocation because it has FlakScatter applied
	auto crdFirestorm = MapClass::Instance->FindFirstFirestorm(
		*pLocation, dest, pOwner);

	if(crdFirestorm != CoordStruct::Empty) {
		crdFirestorm.Z = MapClass::Instance->GetCellFloorHeight(crdFirestorm);
		pThis->SetLocation(crdFirestorm);

		auto const pCell = MapClass::Instance->GetCellAt(crdFirestorm);
		auto const pBld = pCell->GetBuilding();
		auto const pExt = BuildingExt::ExtMap.Find(pBld);
		pExt->ImmolateVictim(pThis, false);
		pThis->UnInit();

	} else {
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);
		auto const pCell = AresTrajectoryHelper::FindFirstObstacle(
			*pLocation, dest, pThis->Owner, pThis->Target, pThis->Type,
			pTypeExt, pOwner);

		pThis->SetLocation(pCell ? pCell->GetCoords() : dest);
		pThis->Speed = 0;
		pThis->Velocity = BulletVelocity::Empty;
	}

	return 0x468A3F;
}

// set the weapon type when spawning bullets. at least
// Ivan Bombs need those
DEFINE_HOOK(46A5B2, BulletClass_Shrapnel_WeaponType1, 6)
{
	GET(BulletClass*, pShrapnel, EAX);
	GET(WeaponTypeClass*, pWeapon, ESI);

	pShrapnel->SetWeaponType(pWeapon);

	return 0;
}

DEFINE_HOOK(46AA27, BulletClass_Shrapnel_WeaponType2, 9)
{
	GET(BulletClass*, pShrapnel, EAX);
	GET(WeaponTypeClass*, pWeapon, ESI);

	pShrapnel->SetWeaponType(pWeapon);

	return 0;
}

DEFINE_HOOK(469EBA, BulletClass_DetonateAt_Splits, 6)
{
	GET(BulletClass*, pThis, ESI);
	auto pType = pThis->Type;
	auto pExt = BulletTypeExt::ExtMap.Find(pType);

	if(pExt->HasSplitBehavior()) {
		auto &random = ScenarioClass::Instance->Random;

		// some defaults
		int cluster = pType->Cluster;

		// get final target coords and cell
		CoordStruct crdDest = pExt->AroundTarget.Get(pExt->Splits)
			? pThis->GetTargetCoords() : pThis->GetCoords();
		CellStruct cellDest = CellClass::Coord2Cell(crdDest);

		// create a list of cluster targets
		DynamicVectorClass<AbstractClass*> targets;

		if(!pExt->Splits) {
			// default hardcoded YR way: hit each cell around the destination once

			// fill target list with cells around the target
			CellRangeIterator<CellClass>{}(cellDest, pExt->AirburstSpread,
				[&targets](CellClass* pCell) -> bool
			{
				targets.AddItem(pCell); return true;
			});

			// we want as many as we get, not more, not less
			cluster = targets.Count;

		} else {
			// fill with technos in range
			for(auto pTechno : *TechnoClass::Array) {
				if(pTechno->IsInPlayfield && pTechno->IsOnMap && pTechno->Health > 0) {
					CoordStruct crdTechno = pTechno->GetCoords();

					if(crdDest.DistanceFrom(crdTechno) < 0x500) {
						targets.AddItem(pTechno);
					}
				}
			}

			// fill up the list to cluster count with random cells around destination
			const int range = 3;
			while(targets.Count < cluster) {
				int x = random.RandomRanged(-range, range);
				int y = random.RandomRanged(-range, range);

				CellStruct cell = {static_cast<short>(cellDest.X + x), static_cast<short>(cellDest.Y + y)};
				CellClass* pCell = MapClass::Instance->GetCellAt(cell);

				targets.AddItem(pCell);
			}
		}

		// let it rain warheads
		for(int i = 0; i < cluster; ++i) {
			AbstractClass* pTarget = pThis->Target;

			if(!pExt->Splits) {
				// simple iteration
				pTarget = targets.GetItem(i);

			} else if(!pTarget || pExt->RetargetAccuracy < random.RandomDouble()) {
				// select another target randomly
				int index = random.RandomRanged(0, targets.Count - 1);
				pTarget = targets.GetItem(index);

				// firer would hit itself
				if(pTarget == pThis->Owner) {
					if(random.RandomDouble() > 0.5) {
						index = random.RandomRanged(0, targets.Count - 1);
						pTarget = targets.GetItem(index);
					}
				}

				// remove this target from the list
				targets.RemoveItem(index);
			}

			// create a new bullet
			WeaponTypeClass* pWeapon = pType->AirburstWeapon;
			if(pTarget && pWeapon) {
				auto pSplitExt = BulletTypeExt::ExtMap.Find(pWeapon->Projectile);

				if(auto pBullet = pSplitExt->CreateBullet(pTarget, pThis->Owner, pWeapon)) {
					DirStruct const dir(5, static_cast<short>(random.RandomRanged(0, 31)));
					auto const radians = dir.radians();

					auto const sin_rad = Math::sin(radians);
					auto const cos_rad = Math::cos(radians);

					//auto const almostDown = 1.5 * Math::Pi * 1.00001;
					auto const cos_factor = -2.44921270764e-16; // Math::cos(almostDown);
					auto const flatSpeed = cos_factor * pBullet->Speed;

					BulletVelocity velocity;
					velocity.X = cos_rad * flatSpeed;
					velocity.Y = sin_rad * flatSpeed;
					velocity.Z = -pBullet->Speed;

					pBullet->MoveTo(pThis->Location, velocity);
				}
			}
		}
	}

	return 0x46A290;
}

DEFINE_HOOK(468EB9, BulletClass_Fire_SplitsA, 6)
{
	GET(BulletTypeClass* const, pType, EAX);
	auto const pExt = BulletTypeExt::ExtMap.Find(pType);
	return !pExt->HasSplitBehavior() ? 0x468EC7u : 0x468FF4u;
}

DEFINE_HOOK(468FFA, BulletClass_Fire_SplitsB, 6)
{
	GET(BulletTypeClass* const, pType, EAX);
	auto const pExt = BulletTypeExt::ExtMap.Find(pType);
	return pExt->HasSplitBehavior() ? 0x46909Au : 0x469008u;
}

DEFINE_HOOK(467B94, BulletClass_Update_Ranged, 7)
{
	GET(BulletClass*, pThis, EBP);
	REF_STACK(bool, Destroy, 0x18);
	REF_STACK(CoordStruct, CrdNew, 0x24);

	// range check
	if(pThis->Type->Ranged) {
		CoordStruct crdOld = pThis->GetCoords();

		pThis->Range -= Game::F2I(CrdNew.DistanceFrom(crdOld));
		if(pThis->Range <= 0) {
			Destroy = true;
		}
	}

	// replicate replaced instruction
	pThis->SetLocation(CrdNew);

	// firestorm wall check
	if(HouseExt::IsAnyFirestormActive && !pThis->Type->IgnoresFirestorm) {
		auto const pCell = MapClass::Instance->GetCellAt(CrdNew);

		if(auto const pBld = pCell->GetBuilding()) {
			auto const pOwner = pThis->Owner ? pThis->Owner->Owner : nullptr;

			if(BuildingExt::IsActiveFirestormWall(pBld, pOwner)) {
				auto const pExt = BuildingExt::ExtMap.Find(pBld);
				pExt->ImmolateVictim(pThis, false);
				pThis->UnInit();
				return 0x467FBA;
			}
		}
	}

	return 0x467BA4;
}

DEFINE_HOOK(4664FB, BulletClass_Initialize_Ranged, 6)
{
	GET(BulletClass*, pThis, ECX);
	// conservative approach for legacy-initialized bullets
	pThis->Range = std::numeric_limits<int>::max();
	return 0;
}

DEFINE_HOOK(6FE53F, TechnoClass_Fire_CreateBullet, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET(int, speed, EAX);
	GET(int, damage, EDI);
	GET_BASE(AbstractClass*, pTarget, 0x8);

	// replace skipped instructions
	REF_STACK(int, Speed, 0x28);
	Speed = speed;

	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	auto pBulletExt = BulletTypeExt::ExtMap.Find(pWeapon->Projectile);

	// create a new bullet with projectile range
	auto ret = pBulletExt->CreateBullet(pTarget, pThis, damage, pWeapon->Warhead,
		speed, pWeaponExt->GetProjectileRange(), pWeapon->Bright);

	R->EAX(ret);
	return 0x6FE562;
}

DEFINE_HOOK(468000, BulletClass_GetAnimFrame, 6)
{
	GET(BulletClass*, pThis, ECX);
	auto pType = pThis->Type;

	int frame = 0;
	if(pType->AnimLow || pType->AnimHigh) {
		frame = pThis->AnimFrame;
	} else if(pType->Rotates()) {
		auto angle = Math::arctanfoo(-pThis->Velocity.Y, pThis->Velocity.X);
		DirStruct dir(angle);

		const auto ReverseFacing32 = *reinterpret_cast<const int(*)[8]>(0x7F4890);
		auto facing = ReverseFacing32[dir.value32()];

		auto pExt = BulletTypeExt::ExtMap.Find(pType);
		const int length = pExt->AnimLength;

		if(length > 1) {
			frame = facing * length + ((Unsorted::CurrentFrame / pType->AnimRate) % length);
		} else {
			frame = facing;
		}
	}

	R->EAX(frame);
	return 0x468088;
}
