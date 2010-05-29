#include <AnimClass.h>
#include <WeaponTypeClass.h>

#include "../../Misc/Applicators.h"

#include "Body.h"
#include "../BuildingType/Body.h"
#include "../House/Body.h"
#include "../Techno/Body.h"

DEFINE_HOOK(4FB257, HouseClass_UnitFromFactory_Firewall, 6)
{
	GET(BuildingClass *, B, ESI);
	GET(HouseClass *, H, EBP);
	GET_STACK(CellStruct, CenterPos, 0x4C);

	//BuildingExt::ExtendFirewall(B, CenterPos, H);
	BuildingExt::buildLines(B, CenterPos, H);

	return 0;
}


DEFINE_HOOK(445355, BuildingClass_KickOutUnit_Firewall, 6)
{
	GET(BuildingClass *, Factory, ESI);

	GET(BuildingClass *, B, EDI);
	GET_STACK(CellStruct, CenterPos, 0x20);

	//BuildingExt::ExtendFirewall(B, CenterPos, Factory->Owner);
	BuildingExt::buildLines(B, CenterPos, Factory->Owner);

	return 0;
}

// placement linking
DEFINE_HOOK(6D5455, sub_6D5030, 6)
{
	GET(BuildingTypeClass *, BT, EAX);
	BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(BT);

	return pTypeData->IsLinkable()
	 ? 0x6D545F
	 : 0x6D54A9;
}

// placement linking
DEFINE_HOOK(6D5A5C, sub_6D59D0, 6)
{
	GET(BuildingTypeClass *, BT, EDX);
	BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(BT);

	return pTypeData->IsLinkable()
	 ? 0x6D5A66
	 : 0x6D5A75;
}

// frame to draw
DEFINE_HOOK(43EFB3, BuildingClass_GetStaticImageFrame, 6)
{
	GET(BuildingClass *, B, ESI);

	if(B->GetCurrentMission() != mission_Construction) {
		signed int FrameIdx = BuildingExt::GetImageFrameIndex(B);

		if(FrameIdx != -1) {
			R->EAX<signed int>(FrameIdx);
			return 0x43EFC3;
		}
	}
	return 0x43EFC6;
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

	BuildingExt::ExtData * pData = BuildingExt::ExtMap.Find(B);
	pData->UpdateFirewall();

	return 0;
}

// pathfinding 1
DEFINE_HOOK(483D8E, CellClass_Setup_Slave, 6)
{
	GET(BuildingClass *, B, ESI);
	BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(B->Type);
	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(B->Owner);

	R->EAX<BuildingTypeClass *>(B->Type);

	if(pTypeData->Firewall_Is) {
		R->EAX<HouseClass *>(B->Owner);
		return pHouseData->FirewallActive
			 ? 0x483D6B
			 : 0x483DCD
		;
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
			: 0x51BD7D
		;
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
			: 0x51C70F
		;
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
		R->EAX<HouseClass *>(B->Owner);
		return pHouseData->FirewallActive
			? 0x73FCD0
			: 0x73FA87
		;
	} else {
		return 0x73F7D3;
	}
}

// pathfinding 5
DEFINE_HOOK(58819F, MapClass_SomePathfinding_1, 6)
{
	GET(BuildingClass *, B, EAX);
	BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(B->Type);
	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(B->Owner);

	R->EAX<HouseClass *>(B->Owner);
	return (pTypeData->Firewall_Is && pHouseData->FirewallActive)
		? 0x5881BF
		: 0x5881C4
	;
}

// pathfinding 6
DEFINE_HOOK(58828C, MapClass_SomePathfinding_2, 6)
{
	GET(BuildingClass *, B, EAX);
	BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(B->Type);
	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(B->Owner);

	R->EAX<HouseClass *>(B->Owner);
	return (pTypeData->Firewall_Is && pHouseData->FirewallActive)
		? 0x5882AC
		: 0x5882B1
	;
}

// pathfinding 7
DEFINE_HOOK(5884A4, MapClass_SomePathfinding_3, 6)
{
	GET(BuildingClass *, B, EAX);
	BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(B->Type);
	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(B->Owner);

	R->EAX<HouseClass *>(B->Owner);
	return (pTypeData->Firewall_Is && pHouseData->FirewallActive)
		? 0x5884C4
		: 0x5884C9
	;
}

// targeting state
DEFINE_HOOK(6FC0C5, TechnoClass_GetObjectActivityState_Firewall, 6)
{
	GET(TechnoClass *, Tgt, EBX);
	if(BuildingClass *B = specific_cast<BuildingClass*>(Tgt)) {
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

	CellSequence Path(&Src->Location, &Tgt->Location);

	Path.Apply(FireFinder);

	if(FireFinder.found) {
		Src->ShouldLoseTargetNow = 1;
		TechnoExt::FiringStateCache = FireError::ILLEGAL;
	} else {
		TechnoExt::FiringStateCache = -1;
	}
	return 0;
}

DEFINE_HOOK(6FCD23, TechnoClass_GetObjectActivityState_OverrideFirewall, 6)
{
	if(TechnoExt::FiringStateCache != -1) {
		R->EAX(TechnoExt::FiringStateCache);
		TechnoExt::FiringStateCache = -1;
	}

	return 0;
}

DEFINE_HOOK(6F64CB, TechnoClass_DrawHealthBar, 6)
{
	GET(BuildingClass *, B, ESI);
	BuildingTypeExt::ExtData * pData = BuildingTypeExt::ExtMap.Find(B->Type);
	return (pData->Firewall_Is)
		? 0x6F6832
		: 0
	;
}

DEFINE_HOOK(71B126, TemporalClass_Fire, 7)
{
	GET(BuildingClass *, B, EDI);
	BuildingTypeExt::ExtData * pData = BuildingTypeExt::ExtMap.Find(B->Type);
	HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(B->Owner);

	if(pData->Firewall_Is && pHouseData->FirewallActive) {
		bool found = false;
		for(int i = 0; i < B->Owner->Buildings.Count; ++i) {
			BuildingClass * pBuilding = B->Owner->Buildings[i];
			if(pBuilding->Type == B->Type && pBuilding != B) {
				if(!pBuilding->InLimbo && pBuilding->IsAlive && pBuilding->Health) {
					found = true;
					break;
				}
			}
		}
		if(!found) {
			pHouseData->SetFirestormState(0);
		}
	}

	return 0;
}

DEFINE_HOOK(4DA53E, FootClass_Update, 6)
{
	GET(FootClass *, F, ESI);

	CellClass *C = F->GetCell();
	if(BuildingClass * B = C->GetBuilding()) {
		BuildingTypeExt::ExtData* pTypeData = BuildingTypeExt::ExtMap.Find(B->Type);
		HouseExt::ExtData *pHouseData = HouseExt::ExtMap.Find(B->Owner);
		if(pTypeData->Firewall_Is && pHouseData->FirewallActive) {
			BuildingExt::ExtData * pData = BuildingExt::ExtMap.Find(B);
			pData->ImmolateVictim(F);
		}
	}

	return 0;
}
