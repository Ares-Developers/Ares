#include "TrajectoryHelper.h"

#include <CellClass.h>
#include <HouseClass.h>
#include <MapClass.h>
#include <WarheadTypeClass.h>

Vector2D<int> AresTrajectoryHelper::AbsoluteDifference(const CoordStruct& coords) {
	return{ std::abs(coords.X), std::abs(coords.Y) };
}

Vector2D<int> AresTrajectoryHelper::AbsoluteDifference(const CellStruct& cell) {
	return{ std::abs(cell.X), std::abs(cell.Y) };
}

CellClass* AresTrajectoryHelper::GetObstacle(
	CellClass const* const pCellSource, CellClass const* const pCellTarget,
	CellClass const* const pCellBullet, CoordStruct const& crdCur,
	BulletTypeClass const* const pType, BulletTypeExt::ExtData const* pTypeExt,
	HouseClass const* const pOwner)
{
	auto const cellCur = CellClass::Coord2Cell(crdCur);
	auto const pCellCur = MapClass::Instance->GetCellAt(cellCur);

	auto IsCliffHit = [&]() {
		return pType->SubjectToCliffs
			&& TrajectoryHelper::IsCliffHit(pCellSource, pCellBullet, pCellCur);
	};

	auto IsWallHit = [&]() {
		return pType->SubjectToWalls
			&& TrajectoryHelper::IsWallHit(pCellSource, pCellCur, pCellTarget, pOwner);
	};

	auto const isHit = IsCliffHit() || IsWallHit(); 

	if(isHit) {
#ifdef SLOW_WITH_SAME_OUTCOME
		// NOTE: this path returns pCellCur regardless of anything that happen
		// in this block, because the complicated 'if' below folds down to
		// a constant and the magnitude calculation would return the same
		// value anyhow. this is here for reference -AlexB 2015-03-01
		auto const diff = crdCur - pCellCur->GetCoords();

		// Magnitude <= 85
		auto const UpperLimit = 86 * 86;
		if(diff.MagnitudeSquared() < UpperLimit) {
			return pCellCur;
		}

		auto const diff1 = AbsoluteDifference(
			pCellSource->GetCoords() - pCellTarget->GetCoords());
		auto const diff2 = AbsoluteDifference(
			crdCur - pCellCur->GetCoords());

		// original code, with isHit reused (and known to be true)
		// this is a well hidden compile time constant 'true'
		if((diff1.Y < diff1.X && diff2.Y > diff2.X && isHit)
			|| (diff1.Y <= diff1.X || diff2.Y >= diff2.X || isHit))
		{
			return pCellCur;
		}
#else
		return pCellCur;
#endif
	}

	return nullptr;
}

CellClass* AresTrajectoryHelper::FindFirstObstacle(
	CoordStruct const& crdSrc, CoordStruct const& crdTarget,
	BulletTypeClass const* const pType,
	BulletTypeExt::ExtData const* const pTypeExt,
	HouseClass const* const pOwner)
{
	if(AresTrajectoryHelper::SubjectToAnything(pType)) {
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
			if(auto const pCell = GetObstacle(pCellSrc, pCellTarget, pCellCur,
				crdCur, pType, pTypeExt, pOwner))
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
	WeaponTypeClass const* const pWeapon, HouseClass const* const pOwner)
{
	auto const pProjectile = pWeapon->Projectile;
	auto const pProjectileExt = BulletTypeExt::ExtMap.Find(pProjectile);

	if(auto const pCell = FindFirstObstacle(
		crdSrc, crdTarget, pProjectile, pProjectileExt,
		pOwner))
	{
		if(pCell->ConnectsToOverlay(-1, -1)) {
			if(pWeapon->Warhead->Wall) {
				return nullptr;
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
		pCellSource, pCellTarget, pCellBullet, crdCur, pType,
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
		*pSource, *pTarget, pType, pTypeExt, pOwner);

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
		*pSource, *pTarget, pWeapon, pOwner);

	R->EAX(ret);
	return 0x4CC357;
}
