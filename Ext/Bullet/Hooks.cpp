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

DEFINE_HOOK(46867F, BulletClass_SetMovement_Parachute, 5)
{
	GET(CoordStruct *, XYZ, EAX);
	GET(BulletClass *, Bullet, ECX);
//	GET_BASE(BulletVelocity *, Trajectory, 0xC);

	R->SetEx_EBX<BulletClass *>(Bullet);
	
	BulletTypeExt::ExtData *pBulletData = BulletTypeExt::ExtMap.Find(Bullet->Type);

//	Debug::Log("Bullet [%s] is parachuted (%d)\n", Bullet->Type->get_ID(), pBulletData->Parachuted);

	byte result;
	if(pBulletData->Parachuted) {
		result = Bullet->SpawnParachuted(XYZ);
//		Debug::Log("Bullet trajectory is (%lf, %lf, %lf)\n", *Trajectory);
		Bullet->IsABomb = 1;
	} else {
		result = Bullet->Put(XYZ, 0);
	}

	R->set_EAX(result);
	return 0x468689;
}
