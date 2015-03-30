#include <AnimClass.h>
#include <WeaponTypeClass.h>

#include "Body.h"
#include "../BuildingType/Body.h"
#include "../BulletType/Body.h"
#include "../House/Body.h"
#include "../Techno/Body.h"
#include "../Tiberium/Body.h"
#include "../Rules/Body.h"

#include "../../Misc/SWTypes/Firewall.h"

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
DEFINE_HOOK(4423E7, BuildingClass_ReceiveDamage_FSW, 5)
{
	GET(BuildingClass* const, pThis, ESI);
	GET_STACK(int* const, pDamage, 0xA0);

	if(BuildingExt::IsActiveFirestormWall(pThis)) {
		auto const pExt = RulesExt::Global();
		auto const& coefficient = pExt->DamageToFirestormDamageCoefficient;
		auto const amount = static_cast<int>(*pDamage * coefficient);

		if(amount > 0) {
			auto const index = SW_Firewall::FirewallType;
			if(auto const pSuper = pThis->Owner->FindSuperWeapon(index)) {
				auto const left = pSuper->RechargeTimer.GetTimeLeft();
				int const reduced = std::max(0, left - amount);
				pSuper->RechargeTimer.Start(reduced);
			}
		}
		return 0x4423B7;
	}

	return 0x4423F2;
}

// connect the newly built Firestorm Wall
DEFINE_HOOK(440D01, BuildingClass_Put_FirestormWall, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	//GET(CellStruct const*, pMapCoords, EBP);
	auto const pExt = BuildingExt::ExtMap.Find(pThis);

	pExt->UpdateFirewallLinks();

	return 0;
}

// disconnect the Firestorm Wall
DEFINE_HOOK(445DF4, BuildingClass_Remove_FirestormWall, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	auto const pExt = BuildingExt::ExtMap.Find(pThis);

	pExt->UpdateFirewallLinks();

	return 0;
}

// main update
DEFINE_HOOK(440378, BuildingClass_Update_FirestormWall, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	auto const pData = BuildingExt::ExtMap.Find(pThis);

	pData->UpdateFirewall(false);

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

DEFINE_HOOK(6F64CB, TechnoClass_DrawHealthBar_FirestormWall, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	auto const pData = BuildingTypeExt::ExtMap.Find(pThis->Type);
	return pData->Firewall_Is ? 0x6F6832u : 0u;
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
