#include <AnimClass.h>
#include <WeaponTypeClass.h>

#include "../../Misc/Applicators.h"

#include "Body.h"
#include "../BuildingType/Body.h"
#include "../House/Body.h"
#include "../Techno/Body.h"
#include "../Tiberium/Body.h"
#include "../Rules/Body.h"

#include <Helpers/Enumerators.h>

DEFINE_HOOK(5880A0, MapClass_FindFirstFirestorm, 6)
{
	//GET(MapClass* const, pThis, ECX);
	GET_STACK(CoordStruct* const, pOutBuffer, STACK_OFFS(0x0, -0x4));
	GET_STACK(CoordStruct const* const, pStart, STACK_OFFS(0x0, -0x8));
	GET_STACK(CoordStruct const* const, pEnd, STACK_OFFS(0x0, -0xC));
	GET_STACK(HouseClass const* const, pOwner, STACK_OFFS(0x0, -0x10));

	*pOutBuffer = CoordStruct::Empty;

	if(HouseExt::IsAnyFirestormActive && *pStart != *pEnd) {
		auto const start = CellClass::Coord2Cell(*pStart);
		auto const end = CellClass::Coord2Cell(*pEnd);

		for(CellSequenceEnumerator it(start, end); it; ++it) {
			auto const pCell = MapClass::Instance->GetCellAt(*it);
			if(auto const pBld = pCell->GetBuilding()) {
				if(BuildingExt::IsActiveFirestormWall(pBld, pOwner)) {
					*pOutBuffer = CellClass::Cell2Coord(*it);
					break;
				}
			}
		}
	}

	R->EAX(pOutBuffer);
	return 0x58855E;
}

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
	GET(BuildingTypeClass* const, pType, EAX);
	auto const pExt = BuildingTypeExt::ExtMap.Find(pType);

	return pExt->IsLinkable() ? 0x6D545Fu : 0x6D54A9u;
}

// placement linking
DEFINE_HOOK(6D5A5C, sub_6D59D0, 6)
{
	GET(BuildingTypeClass* const, pType, EDX);
	auto const pExt = BuildingTypeExt::ExtMap.Find(pType);

	return pExt->IsLinkable() ? 0x6D5A66u : 0x6D5A75u;
}

// frame to draw
DEFINE_HOOK(43EFB3, BuildingClass_GetStaticImageFrame, 6)
{
	GET(BuildingClass*, pThis, ESI);

	if(pThis->GetCurrentMission() != Mission::Construction) {
		auto FrameIdx = BuildingExt::GetImageFrameIndex(pThis);

		if(FrameIdx != -1) {
			R->EAX(FrameIdx);
			return 0x43EFC3;
		}
	}
	return 0x43EFC6;
}

// ignore damage
DEFINE_HOOK(442230, BuildingClass_ReceiveDamage_FSW, 6)
{
	GET(BuildingClass* const, pThis, ECX);
	GET_STACK(int* const, pDamage, 0x4);

	if(BuildingExt::IsActiveFirestormWall(pThis)) {
		*pDamage = 0;
		return 0x442C14;
	}

	return 0;
}

// main update
DEFINE_HOOK(43FC39, BuildingClass_Update_FSW, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	auto const pData = BuildingExt::ExtMap.Find(pThis);

	pData->UpdateFirewall();

	return 0;
}

// pathfinding 1
DEFINE_HOOK(483D94, CellClass_Setup_Slave, 6)
{
	GET(BuildingClass* const, pBuilding, ESI);
	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);

	if(pTypeExt->Firewall_Is) {
		auto const pHouseExt = HouseExt::ExtMap.Find(pBuilding->Owner);
		return pHouseExt->FirewallActive ? 0x483D6Bu : 0x483DCDu;
	}

	return 0x483DB0;
}

// pathfinding 2
DEFINE_HOOK(51BD4C, InfantryClass_Update, 6)
{
	GET(BuildingClass* const, pBld, EDI);
	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

	enum {Impassable = 0x51BD7F, Ignore = 0x51BD7D, NoDecision = 0x51BD68};

	if(pTypeExt->IsPassable) {
		return Ignore;
	}

	if(pTypeExt->Firewall_Is) {
		auto const pHouseExt = HouseExt::ExtMap.Find(pBld->Owner);
		return pHouseExt->FirewallActive ? Impassable : Ignore;
	}

	return NoDecision;
}

// pathfinding 3
DEFINE_HOOK(51C4C8, InfantryClass_IsCellOccupied, 6)
{
	GET(BuildingClass* const, pBld, ESI);
	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

	enum {Impassable = 0x51C7D0, Ignore = 0x51C70F, NoDecision = 0x51C4EB};

	if(pTypeExt->IsPassable) {
		return Ignore;
	}

	if(pTypeExt->Firewall_Is) {
		auto const pHouseExt = HouseExt::ExtMap.Find(pBld->Owner);
		return pHouseExt->FirewallActive ? Impassable : Ignore;
	}

	return NoDecision;
}

// pathfinding 4
DEFINE_HOOK(73F7B0, UnitClass_IsCellOccupied, 6)
{
	GET(BuildingClass* const, pBld, ESI);
	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

	enum {Impassable = 0x73FCD0, Ignore = 0x73FA87, NoDecision = 0x73F7D3};

	if(pTypeExt->IsPassable) {
		return Ignore;
	}

	if(pTypeExt->Firewall_Is) {
		auto const pHouseExt = HouseExt::ExtMap.Find(pBld->Owner);
		return pHouseExt->FirewallActive ? Impassable : Ignore;
	}

	return NoDecision;
}

// targeting state
DEFINE_HOOK(6FC0C5, TechnoClass_GetFireError_Firewall, 6)
{
	//GET(TechnoClass* const, pThis, ESI);
	GET(TechnoClass* const, pTarget, EBX);

	if(auto const pBld = abstract_cast<BuildingClass*>(pTarget)) {
		if(BuildingExt::IsActiveFirestormWall(pBld, nullptr)) {
			return 0x6FC86A;
		}
	}

	return 0;
}

DEFINE_HOOK(6FCD1D, TechnoClass_GetFireError_CanTargetFirewall, 5)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(AbstractClass* const, pTarget, 0x24);
	GET_STACK(int const, idxWeapon, 0x28);

	if(!HouseExt::IsAnyFirestormActive) {
		return 0;
	}

	auto const pWeapon = pThis->GetWeapon(idxWeapon)->WeaponType;
	if(!pWeapon || !pWeapon->Projectile) {
		return 0;
	}

	auto const pBulletData = BulletTypeExt::ExtMap.Find(pWeapon->Projectile);
	if(!pBulletData->SubjectToFirewall) {
		return 0;
	}

	auto const crdTgt = Tgt->GetCoords();

	FirestormFinderApplicator FireFinder(pThis->Owner);

	CellSequence Path(&pThis->Location, &crdTgt);

	Path.Apply(FireFinder);

	if(FireFinder.found) {
		pThis->ShouldLoseTargetNow = 1;
		TechnoExt::FiringStateCache = FireError::ILLEGAL;
	} else {
		TechnoExt::FiringStateCache = FireError::NONE;
	}
	return 0;
}

DEFINE_HOOK(6FCD23, TechnoClass_GetObjectActivityState_OverrideFirewall, 6)
{
	if(TechnoExt::FiringStateCache != FireError::NONE) {
		R->EAX(TechnoExt::FiringStateCache);
		TechnoExt::FiringStateCache = FireError::NONE;
	}

	return 0;
}

DEFINE_HOOK(6F64CB, TechnoClass_DrawHealthBar_FirestormWall, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	auto const pData = BuildingTypeExt::ExtMap.Find(pThis->Type);
	return pData->Firewall_Is ? 0x6F6832u : 0u;
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
	GET(FootClass* const, pThis, ESI);

	if(pThis->IsAlive) {
		auto const pCell = pThis->GetCell();
		if(auto const pBld = pCell->GetBuilding()) {
			if(BuildingExt::IsActiveFirestormWall(pBld)) {
				auto const pData = BuildingExt::ExtMap.Find(pBld);
				pData->ImmolateVictim(pThis);
			}
		}
	}

	// tiberium heal, as in Tiberian Sun, but customizable per Tiberium type
	if(pThis->IsAlive && RulesExt::Global()->Tiberium_HealEnabled
		&& pThis->GetHeight() <= RulesClass::Instance->HoverHeight)
	{
		auto const pType = pThis->GetTechnoType();
		if(pType->TiberiumHeal || pThis->HasAbility(Ability::TiberiumHeal)) {
			if(pThis->Health > 0 && pThis->Health < pType->Strength) {
				auto const pCell = pThis->GetCell();
				if(pCell->LandType == LandType::Tiberium) {
					auto delay = RulesClass::Instance->TiberiumHeal;
					auto health = pType->GetRepairStep();

					int idxTib = pCell->GetContainedTiberiumIndex();
					if(auto const pTib = TiberiumClass::Array->GetItemOrDefault(idxTib)) {
						auto pExt = TiberiumExt::ExtMap.Find(pTib);
						delay = pExt->GetHealDelay();
						health = pExt->GetHealStep(pThis);
					}

					if(!(Unsorted::CurrentFrame % Game::F2I(delay * 900.0))) {
						pThis->Health += health;
						if(pThis->Health > pType->Strength) {
							pThis->Health = pType->Strength;
						}
					}
				}
			}
		}
	}

	return 0;
}
