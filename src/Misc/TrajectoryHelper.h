#include "../Ext/BulletType/Body.h"

class CellClass;
class HouseClass;

class AresTrajectoryHelper
{
private:
	static Vector2D<int> AbsoluteDifference(const CoordStruct& coords);

	static Vector2D<int> AbsoluteDifference(const CellStruct& cell);

public:
	// gets whether collision checks are needed
	static bool SubjectToAnything(
		BulletTypeClass const* pType)
	{
		return pType->SubjectToCliffs
			|| pType->SubjectToWalls;
	}

	// gets the obstacle when moving from pCellBullet to crdCur
	static CellClass* GetObstacle(
		CellClass const* pCellSource, CellClass const* pCellTarget,
		CellClass const* pCellBullet, CoordStruct const& crdCur,
		BulletTypeClass const* pType,
		BulletTypeExt::ExtData const* pTypeExt, HouseClass const* pOwner);

	// gets the first obstacle when moving from crdSrc to crdTarget
	static CellClass* FindFirstObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		BulletTypeClass const* pType,
		BulletTypeExt::ExtData const* pTypeExt, HouseClass const* pOwner);

	// gets the first obstacle from crdSrc to crdTarget a weapon cannot destroy
	static CellClass* FindFirstImpenetrableObstacle(
		CoordStruct const& crdSrc, CoordStruct const& crdTarget,
		WeaponTypeClass const* pWeapon, HouseClass const* pOwner);
};
