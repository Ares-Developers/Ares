#include <BulletClass.h>
#include <BuildingClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include "../BulletType/Body.h"
#include "../BuildingType/Body.h"

// forced solid buildings - implement selection depending on projectile/building settings and heightmaps
DEFINE_HOOK(4666F7, BulletClass_Update, 6)
{
	GET(BulletClass *, Bullet, EBP);

	CellClass *MyCell = Bullet->GetCell();
	BulletTypeExt::ExtData *pBulletData = BulletTypeExt::ExtMap.Find(Bullet->Type);
	BuildingClass *BuildingInIt = MyCell->GetBuilding();
	if(pBulletData->SubjectToSolid && BuildingInIt && Bullet->Owner != BuildingInIt) {
		BuildingTypeExt::ExtData *pBuildingData = BuildingTypeExt::ExtMap.Find(BuildingInIt->Type);
		CoordStruct MyXYZ;
		MyCell->GetCoords(&MyXYZ);

		// use this delta offset to pick specific foundation cell's height from height map when it's implemented
		CellStruct MyXY, BldXY, DeltaXY;
		Bullet->GetMapCoords(&MyXY);
		BuildingInIt->GetMapCoords(&BldXY);
		DeltaXY = MyXY - BldXY;

		int MyHeight = Bullet->get_Location()->Z;
		int BldHeight = MapClass::Global()->GetCellFloorHeight(&MyXYZ) + pBuildingData->Solid_Height * 256;
		if(MyHeight <= BldHeight) {
//			Debug::Log("Bullet at %d hits building of height %d == boom\n", MyHeight, BldHeight);
			Bullet->SetTarget((ObjectClass *)MyCell);
			Bullet->set_SpawnNextAnim(1);
			Bullet->set_NextAnim(NULL);
		}
	}

	return 0;
}
