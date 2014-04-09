#include <BulletClass.h>
#include <BuildingClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include "../BulletType/Body.h"
#include "../BuildingType/Body.h"
#include "../WeaponType/Body.h"
#include <ScenarioClass.h>
#include <YRMath.h>
#include <Helpers/Iterators.h>

// forced solid buildings - implement selection depending on projectile/building settings and heightmaps
DEFINE_HOOK(4666F7, BulletClass_Update, 6)
{
	GET(BulletClass *, Bullet, EBP);

	CellClass *MyCell = Bullet->GetCell();
	BulletTypeExt::ExtData *pBulletData = BulletTypeExt::ExtMap.Find(Bullet->Type);
	BuildingClass *BuildingInIt = MyCell->GetBuilding();
	if(pBulletData->SubjectToSolid && BuildingInIt && Bullet->Owner != BuildingInIt) {
		auto *pBuildingTypeData = BuildingTypeExt::ExtMap.Find(BuildingInIt->Type);
		if(int solidHeight = pBuildingTypeData->Solid_Height) {
			if(solidHeight < 0) {
				solidHeight = BuildingInIt->Type->Height * 256;
			} else {
				solidHeight *= 256;
			}
			CoordStruct MyXYZ = MyCell->GetCoords();

			// use this delta offset to pick specific foundation cell's height from height map when it's implemented
			auto MyXY = Bullet->GetMapCoords();
			auto BldXY = BuildingInIt->GetMapCoords();
			auto DeltaXY = MyXY - BldXY;

			int MyHeight = Bullet->Location.Z;
			int BldHeight = MapClass::Instance->GetCellFloorHeight(MyXYZ) + solidHeight;
			if(MyHeight <= BldHeight) {
				Bullet->SetTarget(MyCell);
				Bullet->SpawnNextAnim = 1;
				Bullet->NextAnim = nullptr;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(46867F, BulletClass_SetMovement_Parachute, 5)
{
	GET(CoordStruct *, XYZ, EAX);
	GET(BulletClass *, Bullet, ECX);
//	GET_BASE(BulletVelocity *, Trajectory, 0xC);

	R->EBX<BulletClass *>(Bullet);

	BulletTypeExt::ExtData *pBulletData = BulletTypeExt::ExtMap.Find(Bullet->Type);

//	Debug::Log("Bullet [%s] is parachuted (%d)\n", Bullet->Type->get_ID(), pBulletData->Parachuted);

	byte result;
	if(pBulletData->Parachuted) {
		result = Bullet->SpawnParachuted(*XYZ);
//		Debug::Log("Bullet trajectory is (%lf, %lf, %lf)\n", *Trajectory);
		Bullet->IsABomb = 1;
	} else {
		result = Bullet->Put(*XYZ, 0);
	}

	R->EAX(result);
	return 0x468689;
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

		// create a list of cluster targets
		DynamicVectorClass<AbstractClass*> targets;

		if(!pExt->Splits) {
			// default hardcoded YR way: hit each cell around the destination once
			auto pCell = pThis->GetCell();
			CellRangeIterator it(pCell->MapCoords, pExt->AirburstSpread);

			// fill target list with cells around the target
			auto collect = [&targets](CellClass* pCell) -> bool { targets.AddItem(pCell); return true; };
			it.apply<CellClass>(collect);

			// we want as many as we get, not more, not less
			cluster = targets.Count;

		} else {
			// get detonation coords and cell
			CoordStruct crdDest = pThis->GetTargetCoords();
			CellStruct cellDest = CellClass::Coord2Cell(crdDest);

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
					if(random.RandomDouble() > 0.5f) {
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
					const double doubleA = -0.00009587672516830327;
					const double doubleB = 4.712436918747274;

					int a = (random.RandomRanged(0, 32) << 8) - 16383;
					double b = a * doubleA;
					double sin_b = Math::sin(b);
					double cos_b = Math::cos(b);

					double sin_c = Math::sin(doubleB);
					double cos_c = Math::cos(doubleB);

					BulletVelocity velocity;
					velocity.X = cos_c * cos_b * pBullet->Speed;
					velocity.Y = cos_c * sin_b * pBullet->Speed;
					velocity.Z = sin_c * pBullet->Speed;

					pBullet->MoveTo(pThis->Location, velocity);
				}
			}
		}
	}

	return 0x46A290;
}

DEFINE_HOOK(468EB9, BulletClass_Fire_SplitsA, 6)
{
	GET(BulletTypeClass*, pType, EAX);
	auto pExt = BulletTypeExt::ExtMap.Find(pType);
	return !pExt->HasSplitBehavior() ? 0x468EC7 : 0x468FF4;
}

DEFINE_HOOK(468FFA, BulletClass_Fire_SplitsB, 6)
{
	GET(BulletTypeClass*, pType, EAX);
	auto pExt = BulletTypeExt::ExtMap.Find(pType);
	return pExt->HasSplitBehavior() ? 0x46909A : 0x469008;
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
	GET_BASE(AbstractClass*, pTarget, 0x8);

	// replace skipped instructions
	REF_STACK(int, Speed, 0x28);
	Speed = speed;

	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	auto pBulletExt = BulletTypeExt::ExtMap.Find(pWeapon->Projectile);

	// create a new bullet with projectile range
	auto ret = pBulletExt->CreateBullet(pTarget, pThis, pWeapon->Damage, pWeapon->Warhead,
		speed, pWeaponExt->GetProjectileRange(), pWeapon->Bright);

	R->EAX(ret);
	return 0x6FE562;
}
