#include <AnimClass.h>
#include <BulletClass.h>
#include <BuildingClass.h>
#include <CellClass.h>

#include "Body.h"
#include "..\TechnoType\Body.h"

/*
A_FINE_HOOK(4666F7, BulletClass_Update_FSW, 6)
{
	GET(BulletClass *, Bullet, EBP);

	CellClass *C = Bullet->GetCell();
	if(BuildingClass * B = C->GetBuilding()) {
		BuildingTypeClass *BT = B->Type;
		if(!Bullet->Owner || Bullet->Owner->Owner->IsAlliedWith(B->Owner)) {
			return 0;
		}
		if(BT->FirestormWall && B->Owner->FirestormActive && !Bullet->Type->IgnoresFirestorm) {
			if(AnimTypeClass *FSAnim = AnimTypeClass::Find(Bullet->IsInAir() ? "FSAIR" : "FSGRND")) {
				CoordStruct XYZ;
				Bullet->GetCoords(&XYZ);
				new AnimClass(FSAnim, &XYZ);
			}
			Bullet->Release();
		}
	}

	return 0;
}
*/

DEFINE_HOOK(6FF008, TechnoClass_Fire_FSW, 8)
DEFINE_HOOK_AGAIN(6FF860, TechnoClass_Fire_FSW, 8)
{
	CoordStruct src = *((CoordStruct *)R->lea_StackVar(0x44));
	CoordStruct tgt = *((CoordStruct *)R->lea_StackVar(0x88));

//	check the path of the projectile to see if there are any firestormed cells along the way
//	if so, redirect the proj to the nearest one so it crashes
//	this is only necessary for invisible projectiles which don't move to the target and thus
//	BulletClass::Update hook above won't work

	return 0;
}

DEFINE_HOOK(4DA53E, FootClass_Update, 6)
{
	GET(FootClass *, F, ESI);

	CellClass *C = F->GetCell();
	BuildingClass * B = C->GetBuilding();
	if(B) {
		BuildingTypeClass *BT = B->Type;
		HouseClass *H = B->Owner;
		if(BT->FirestormWall && H->FirestormActive) {
			int Damage = F->Health;
			F->ReceiveDamage(&Damage, 0, RulesClass::Global()->C4Warhead, 0, 1, 0, H);
			if(AnimTypeClass *FSAnim = AnimTypeClass::Find(F->IsInAir() ? "FSAIR" : "FSGRND")) {
				CoordStruct XYZ;
				F->GetCoords(&XYZ);
				new AnimClass(FSAnim, &XYZ);
			}
		}
	}

	return 0;
}

DEFINE_HOOK(4F8C97, HouseClass_Update_FSW_LowPower, 6)
{
	GET(HouseClass *, H, ESI);
	H->FirestormActive = 0;
	return 0;
}

DEFINE_HOOK(442230, BuildingClass_ReceiveDamage_FSW, 6)
{
	GET(BuildingClass *, pThis, ECX);
	GET_STACK(int *, Damage, 0x4);

	if(pThis->Type->FirestormWall && pThis->Owner->FirestormActive) {
		*Damage = 0;
		return 0x442C14;
	}

	return 0;
}

DEFINE_HOOK(43FC39, BuildingClass_Update_FSW, 6)
{
	GET(BuildingClass*, B, ESI);
	BuildingTypeClass *BT = B->Type;
	HouseClass *H = B->Owner;

	if(!BT->FirestormWall) {
		return 0;
	}

	bool FS = H->FirestormActive;

	int FWFrame = B->GetFWFlags();
	if(FS) {
		FWFrame += 32;
	}

	B->FirestormWallFrame = FWFrame;
	B->GetCell()->Setup(0xFFFFFFFF);
	B->SetLayer(lyr_Ground); // HACK - repaints properly

	if(!H->FirestormActive) {
		return 0;
	}

	CoordStruct XYZ;
	if(!(Unsorted::CurrentFrame % 7) && ScenarioClass::Global()->get_Random()->RandomRanged(0, 15) == 1) {
		AnimClass *IdleAnim = B->FirestormAnim;
		if(IdleAnim) {
			delete IdleAnim;
		}
		B->GetCoords(&XYZ);
		XYZ.X -= 768;
		XYZ.Y -= 768;
		if(AnimTypeClass *FSA = AnimTypeClass::Find("FSIDLE")) {
			B->FirestormAnim = new AnimClass(FSA, &XYZ);
		}
	}

	CellClass *C = B->GetCell();
	for(ObjectClass *O = C->GetContent(); O; O = O->NextObject) {
		O->GetCoords(&XYZ);
		if(((O->AbstractFlags & ABSFLAGS_ISTECHNO) != 0) && O != B) {
			int Damage = O->Health;
			O->ReceiveDamage(&Damage, 0, RulesClass::Global()->C4Warhead, 0, 1, 0, H);
		}
	}

	return 0;
}

