#include "../Ext/BulletType/Body.h"

class CellClass;
class HouseClass;

class AresTrajectoryHelper
{
private:
	static bool IsCliffHit(
		CellClass const* pSource, CellClass const* pBefore,
		CellClass const* pAfter);

	static bool IsWallHit(
		CellClass const* pSource, CellClass const* pCheck,
		CellClass const* pTarget, HouseClass const* pOwner);

	static bool IsBuildingHit(
		AbstractClass const* pSource, AbstractClass const* pTarget,
		CoordStruct const& crdCur, HouseClass const* pOwner);

	static Vector2D<int> AbsoluteDifference(const CoordStruct& coords);

	static Vector2D<int> AbsoluteDifference(const CellStruct& cell);

public:
	// gets whether collision checks are needed
	static bool SubjectToAnything(
		BulletTypeClass const* pType, BulletTypeExt::ExtData const* pTypeExt)
	{
		return pType->SubjectToCliffs
			|| pType->SubjectToWalls
			|| pTypeExt->SubjectToSolid;
	}

	// gets the obstacle when moving from pCellBullet to crdCur
	static CellClass* GetObstacle(
		CellClass const* pCellSource, CellClass const* pCellTarget,
		AbstractClass const* pSource, AbstractClass const* pTarget,
		CellClass const* pCellBullet, CoordStruct const& crdCur,
		BulletTypeClass const* pType,
		BulletTypeExt::ExtData const* pTypeExt, HouseClass const* pOwner);

	// gets the first obstacle when moving from crdSrc to crdTarget
	static CellClass* FindFirstObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		AbstractClass const* pSource, AbstractClass const* pTarget,
		BulletTypeClass const* pType,
		BulletTypeExt::ExtData const* pTypeExt, HouseClass const* pOwner);

	// gets the first obstacle from crdSrc to crdTarget a weapon cannot destroy
	static CellClass* FindFirstImpenetrableObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		AbstractClass const* pSource, AbstractClass const* pTarget,
		WeaponTypeClass const* pWeapon, HouseClass const* pOwner);
};
