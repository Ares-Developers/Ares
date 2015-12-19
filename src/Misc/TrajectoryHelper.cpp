#include "TrajectoryHelper.h"

#include "../Ext/BuildingType/Body.h"
#include "../Ext/Rules/Body.h"
#include "../Ext/WarheadType/Body.h"

#include <BulletTypeClass.h>
#include <CellClass.h>
#include <HouseClass.h>
#include <MapClass.h>
#include <OverlayTypeClass.h>
#include <WarheadTypeClass.h>

bool AresTrajectoryHelper::IsCliffHit(
	CellClass const* const pSource, CellClass const* const pBefore,
	CellClass const* const pAfter)
{
	auto const levelAfter = pAfter->GetLevel();
	return levelAfter - pBefore->GetLevel() >= CellClass::BridgeLevels
		&& levelAfter - pSource->GetLevel() > 0;
}

bool AresTrajectoryHelper::IsWallHit(
	CellClass const* const pSource, CellClass const* const pCheck,
	CellClass const* const pTarget, HouseClass const* const pOwner)
{
	if(pCheck != pTarget && pCheck->OverlayTypeIndex != -1) {
		if(OverlayTypeClass::Array->Items[pCheck->OverlayTypeIndex]->Wall) {
			if(pSource->Level <= pTarget->Level) {
				auto const& index = pCheck->WallOwnerIndex;
				return !RulesClass::Instance->AlliedWallTransparency
					|| !HouseClass::Array->Items[index]->IsAlliedWith(pOwner);
			}
		}
	}

	return false;
}

bool AresTrajectoryHelper::IsBuildingHit(
	AbstractClass const* const pSource, AbstractClass const* const pTarget,
	CoordStruct const& crdCur, HouseClass const* const pOwner)
{
	auto const pCellBullet = MapClass::Instance->GetCellAt(crdCur);

	if(auto const pBld = pCellBullet->GetBuilding()) {
		// source building and target buildings are always traversable.
		// this should fix the issue of aircraft being unable to hit buildings
		if(pBld == pSource || pBld == pTarget) {
			return false;
		}

		// does the building let allies through?
		auto const isTransparent = RulesExt::Global()->AlliedSolidTransparency
			&& pBld->Owner->IsAlliedWith(pOwner);

		if(isTransparent) {
			return false;
		}

		auto const pBldType = pBld->Type;

		// open gates are not hit: let it pass through
		if(pBldType->Gate && pBld->IsTraversable()) {
			return false;
		}

		auto const pBldTypeExt = BuildingTypeExt::ExtMap.Find(pBldType);
		if(int solidHeight = pBldTypeExt->Solid_Height) {
			if(solidHeight < 0) {
				solidHeight = pBldType->Height;
			}

			auto const floor = MapClass::Instance->GetCellFloorHeight(crdCur);
			return crdCur.Z <= floor + solidHeight * 256;
		}
	}

	return false;
}

Vector2D<int> AresTrajectoryHelper::AbsoluteDifference(const CoordStruct& coords) {
	return{ std::abs(coords.X), std::abs(coords.Y) };
}

Vector2D<int> AresTrajectoryHelper::AbsoluteDifference(const CellStruct& cell) {
	return{ std::abs(cell.X), std::abs(cell.Y) };
}

CellClass* AresTrajectoryHelper::GetObstacle(
	CellClass const* const pCellSource, CellClass const* const pCellTarget,
	AbstractClass const* const pSource, AbstractClass const* const pTarget,
	CellClass const* const pCellBullet, CoordStruct const& crdCur,
	BulletTypeClass const* const pType, BulletTypeExt::ExtData const* pTypeExt,
	HouseClass const* const pOwner)
{
	auto const cellCur = CellClass::Coord2Cell(crdCur);
	auto const pCellCur = MapClass::Instance->GetCellAt(cellCur);

	auto const IsCliffHit = [=]() {
		return pType->SubjectToCliffs
			&& AresTrajectoryHelper::IsCliffHit(pCellSource, pCellBullet, pCellCur);
	};

	auto const IsWallHit = [=]() {
		return pType->SubjectToWalls
			&& AresTrajectoryHelper::IsWallHit(pCellSource, pCellCur, pCellTarget, pOwner);
	};

	auto const IsBuildingHit = [=]() {
		return pTypeExt->SubjectToSolid
			&& AresTrajectoryHelper::IsBuildingHit(pSource, pTarget, crdCur, pOwner);
	};

	auto const isHit = IsCliffHit() || IsWallHit() || IsBuildingHit(); 

	return isHit ? pCellCur : nullptr;
}

CellClass* AresTrajectoryHelper::FindFirstObstacle(
	CoordStruct const& crdSrc, CoordStruct const& crdTarget,
	AbstractClass const* const pSource, AbstractClass const* const pTarget,
	BulletTypeClass const* const pType,
	BulletTypeExt::ExtData const* const pTypeExt,
	HouseClass const* const pOwner)
{
	if(AresTrajectoryHelper::SubjectToAnything(pType, pTypeExt)) {
		auto const cellTarget = CellClass::Coord2Cell(crdTarget);
		auto const pCellTarget = MapClass::Instance->GetCellAt(cellTarget);

		auto const cellSrc = CellClass::Coord2Cell(crdSrc);
		auto const pCellSrc = MapClass::Instance->GetCellAt(cellSrc);

		auto const delta = AbsoluteDifference(cellSrc - cellTarget);
		auto const maxDelta = static_cast<size_t>(std::max(delta.X, delta.Y));

		auto const step = !maxDelta ? CoordStruct::Empty
			: (crdTarget - crdSrc) * (1.0 / maxDelta);

		auto crdCur = crdSrc;
		auto pCellCur = pCellSrc;
		for(size_t i = 0; i < maxDelta; ++i) {
			if(auto const pCell = GetObstacle(pCellSrc, pCellTarget, pSource,
				pTarget, pCellCur, crdCur, pType, pTypeExt, pOwner))
			{
				return pCell;
			}

			pCellCur = MapClass::Instance->GetCellAt(crdCur);
			crdCur += step;
		}
	}

	return nullptr;
}

CellClass* AresTrajectoryHelper::FindFirstImpenetrableObstacle(
	CoordStruct const& crdSrc, CoordStruct const& crdTarget,
	AbstractClass const* const pSource, AbstractClass const* const pTarget,
	WeaponTypeClass const* const pWeapon, HouseClass const* const pOwner)
{
	auto const pProjectile = pWeapon->Projectile;
	auto const pProjectileExt = BulletTypeExt::ExtMap.Find(pProjectile);

	if(auto const pCell = FindFirstObstacle(
		crdSrc, crdTarget, pSource, pTarget, pProjectile, pProjectileExt,
		pOwner))
	{
		if(pCell->ConnectsToOverlay(-1, -1)) {
			if(pWeapon->Warhead->Wall) {
				return nullptr;
			}
		} else if(auto const pBld = pCell->GetBuilding()) {
			// only willingfully fire through enemy buildings
			if(!pOwner->IsAlliedWith(pBld)) {
				auto const pBldTypeExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

				// penetrable if warhead level is at least equal to building level
				if(pProjectileExt->Solid_Level >= pBldTypeExt->Solid_Level) {
					return nullptr;
				}
			}
		}

		return pCell;
	}

	return nullptr;
}

DEFINE_HOOK(4CC360, TrajectoryHelper_GetObstacle, 5)
{
	GET(CellClass* const, pCellSource, ECX);
	GET(CellClass* const, pCellTarget, EDX);
	GET_STACK(CellClass* const, pCellBullet, 0x4);
	REF_STACK(CoordStruct const, crdCur, 0x8);
	GET_STACK(BulletTypeClass const* const, pType, 0x14);
	GET_STACK(HouseClass const* const, pOwner, 0x18);

	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pType);

	auto const ret = AresTrajectoryHelper::GetObstacle(
		pCellSource, pCellTarget, nullptr, nullptr, pCellBullet, crdCur, pType,
		pTypeExt, pOwner);

	R->EAX(ret);
	return 0x4CC671;
}

DEFINE_HOOK(4CC100, TrajectoryHelper_FindFirstObstacle, 7)
{
	GET(CoordStruct const* const, pSource, ECX);
	GET(CoordStruct const* const, pTarget, EDX);
	GET_STACK(BulletTypeClass const* const, pType, 0x4);
	GET_STACK(HouseClass* const, pOwner, 0x8);

	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pType);

	auto const ret = AresTrajectoryHelper::FindFirstObstacle(
		*pSource, *pTarget, nullptr, nullptr, pType, pTypeExt, pOwner);

	R->EAX(ret);
	return 0x4CC30B;
}

DEFINE_HOOK(4CC310, TrajectoryHelper_FindFirstImpenetrableObstacle, 5)
{
	GET(CoordStruct const* const, pSource, ECX);
	GET(CoordStruct const* const, pTarget, EDX);
	GET_STACK(WeaponTypeClass const* const, pWeapon, 0x4);
	GET_STACK(HouseClass* const, pOwner, 0x8);

	auto const ret = AresTrajectoryHelper::FindFirstImpenetrableObstacle(
		*pSource, *pTarget, nullptr, nullptr, pWeapon, pOwner);

	R->EAX(ret);
	return 0x4CC357;
}
