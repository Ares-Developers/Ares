#include <AnimClass.h>
#include <WeaponTypeClass.h>

#include "..\..\Misc\Applicators.h"

#include "Body.h"
#include "..\BuildingType\Body.h"
#include "..\House\Body.h"
#include "..\Techno\Body.h"

DEFINE_HOOK(4FB257, HouseClass_UnitFromFactory_Firewall, 6)
{
	GET(BuildingClass *, B, ESI);
	GET(HouseClass *, H, EBP);
	GET_STACK(CellStruct, CenterPos, 0x4C);

	BuildingExt::ExtendFirewall(B, CenterPos, H);

	return 0;
}


DEFINE_HOOK(445355, BuildingClass_KickOutUnit_Firewall, 6)
{
	GET(BuildingClass *, Factory, ESI);

	GET(BuildingClass *, B, EDI);
	GET_STACK(CellStruct, CenterPos, 0x2C);

	BuildingExt::ExtendFirewall(B, CenterPos, Factory->Owner);

	return 0;
}

// placement linking
DEFINE_HOOK(6D5455, sub_6D5030, 6)
{
	GET(BuildingTypeClass *, BT, EAX);
	BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(BT);

	return pTypeData->Firewall_Is
	 ? 0x6D545F
	 : 0x6D54A9;
}

// placement linking
DEFINE_HOOK(6D5A5C, sub_6D59D0, 6)
{
	GET(BuildingTypeClass *, BT, EDX);
	BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(BT);

	return pTypeData->Firewall_Is
	 ? 0x6D5A66
	 : 0x6D5A75;
}

// frame to draw
DEFINE_HOOK(43EFB3, BuildingClass_GetAnimLengths, 6)
{
	GET(BuildingTypeClass *, BT, EAX);
	BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(BT);

	return pTypeData->Firewall_Is
	 ? 0x43EFBD
	 : 0x43EFC6;
}

// ignore damage
DEFINE_HOOK(442230, BuildingClass_ReceiveDamage_FSW, 6)
{
	GET(BuildingClass *, pThis, ECX);
	GET_STACK(int *, Damage, 0x4);

	BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(pThis->Type);
	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(pThis->Owner); 
	if(pTypeData->Firewall_Is && pHouseData->FirewallActive) {
		*Damage = 0;
		return 0x442C14;
	}

	return 0;
}

// main update
DEFINE_HOOK(43FC39, BuildingClass_Update_FSW, 6)
{
	GET(BuildingClass*, B, ESI);
	BuildingTypeClass *BT = B->Type;
	HouseClass *H = B->Owner;
	BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(BT);

	if(!pTypeData->Firewall_Is) {
		return 0;
	}

	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(H); 
	bool FS = pHouseData->FirewallActive;

	DWORD FWFrame = BuildingExt::GetFirewallFlags(B);
	if(FS) {
		FWFrame += 32;
	}

	B->FirestormWallFrame = FWFrame;
	B->GetCell()->Setup(0xFFFFFFFF);
	B->SetLayer(lyr_Ground); // HACK - repaints properly

	if(!pHouseData->FirewallActive) {
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
		if(((O->AbstractFlags & ABSFLAGS_ISTECHNO) != 0) && O != B && !O->InLimbo && O->IsAlive) {
			int Damage = O->Health;
			O->ReceiveDamage(&Damage, 0, RulesClass::Global()->C4Warhead, 0, 1, 0, H);
		}
	}

	return 0;
}

// pathfinding 1
DEFINE_HOOK(483D8E, CellClass_Setup_Slave, 6)
{
	GET(BuildingClass *, B, ESI);
	R->set_EAX((DWORD)B->Type);
	BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(B->Type);
	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(B->Owner); 

	if(pTypeData->Firewall_Is) {
		R->set_EBP(pHouseData->FirewallActive ? 6 : 0);
		return 0x483D6B;
	} else {
		return 0x483DB0;
	}
}

// pathfinding 2
DEFINE_HOOK(51BD4C, InfantryClass_Update, 6)
{
	GET(BuildingClass *, B, EDI);
	BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(B->Type);
	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(B->Owner); 

	if(pTypeData->Firewall_Is) {
		return pHouseData->FirewallActive
		 ? 0x51BD7F
		 : 0x51BD7D;
	} else {
		return 0x51BD68;
	}
}

// pathfinding 3
DEFINE_HOOK(51C4C8, InfantryClass_IsCellOccupied, 6)
{
	GET(BuildingClass *, B, ESI);
	BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(B->Type);
	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(B->Owner); 

	if(pTypeData->Firewall_Is) {
		return pHouseData->FirewallActive
		 ? 0x51C7D0
		 : 0x51C70F;
	} else {
		return 0x51C4EB;
	}
}

// pathfinding 4
DEFINE_HOOK(73F7B0, UnitClass_IsCellOccupied, 6)
{
	GET(BuildingClass *, B, ESI);
	BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(B->Type);
	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(B->Owner); 

	if(pTypeData->Firewall_Is) {
		return pHouseData->FirewallActive
		 ? 0x73FCD0
		 : 0x73FA87;
	} else {
		return 0x73F7D3;
	}
}

// targeting state
DEFINE_HOOK(6FC0C5, TechnoClass_GetObjectActivityState_Firewall, 6)
{
	GET(TechnoClass *, Tgt, EBX);
	if(Tgt->WhatAmI() == abs_Building) {
		BuildingClass *B = reinterpret_cast<BuildingClass*>(Tgt);
		if(BuildingTypeExt::ExtMap.Find(B->Type)->Firewall_Is) {
			if(HouseExt::ExtMap.Find(B->Owner)->FirewallActive) {
				return 0x6FC86A;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(6FCD1D, TechnoClass_GetObjectActivityState_CanTargetFirewall, 5)
{
	GET(TechnoClass *, Src, ESI);
	GET_STACK(TechnoClass *, Tgt, 0x24);
	GET_STACK(int, idxWeapon, 0x28);

	WeaponTypeClass *Weapon = Src->GetWeapon(idxWeapon)->WeaponType;
	if(!Weapon || !Weapon->Projectile) {
		return 0;
	}

	BulletTypeExt::ExtData *pBulletData = BulletTypeExt::ExtMap.Find(Weapon->Projectile);
	if(!pBulletData->SubjectToFirewall) {
		return 0;
	}

	FirestormFinderApplicator FireFinder(Src->Owner);

	CellSequence Path(Src->get_Location(), Tgt->get_Location());

	Path.Apply(FireFinder);

	if(FireFinder.found) {
		Src->ShouldLoseTargetNow = 1;
		TechnoExt::FiringStateCache = 4; //fs_OutOfRange;
	} else {
		TechnoExt::FiringStateCache = -1;
	}
	return 0;
}

DEFINE_HOOK(6FCD23, TechnoClass_GetObjectActivityState_OverrideFirewall, 6)
{
	if(TechnoExt::FiringStateCache != -1) {
		R->set_EAX(TechnoExt::FiringStateCache);
		TechnoExt::FiringStateCache = -1;
	}

	return 0;
}
