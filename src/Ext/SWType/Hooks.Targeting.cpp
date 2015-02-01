#include "Body.h"

#include "../Building/Body.h"
#include "../House/Body.h"
#include "../Techno/Body.h"
#include "../../Misc/SWTypes.h"

#include <CellSpread.h>
#include <FactoryClass.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <SuperClass.h>

#include <DiscreteDistributionClass.h>
#include <DiscreteSelectionClass.h>

#pragma region Support

enum class SWTargetFlags {
	DisallowEmpty,
	AllowEmpty
};

struct TargetResult {
	CellStruct Target;
	SWTargetFlags Flags;
};

struct TargetingInfo {
	TargetingInfo(SuperClass* pSuper) :
		Super(pSuper),
		Owner(pSuper->Owner),
		TypeExt(SWTypeExt::ExtMap.Find(pSuper->Type)),
		NewType(TypeExt->GetNewSWType())
	{ }

	bool CanFireAt(const CellStruct &cell) const {
		return this->NewType->CanFireAt(this->TypeExt, this->Owner, cell, false);
	}

	SuperClass* Super;
	HouseClass* Owner;
	SWTypeExt::ExtData* TypeExt;
	NewSWType* NewType;
};

CellStruct ConvertToCell(const CellStruct& result) {
	return result;
}

template <typename T>
CellStruct ConvertToCell(const T& result) {
	if(result) {
		return result->GetMapCoords();
	} else {
		return CellStruct::Empty;
	}
}

// check for blocking things, check preferred, else select something manually
template<typename CanFire, typename Prefer, typename Selector>
CellStruct GetTarget(const TargetingInfo& info, CanFire canFire, Prefer prefer, Selector selector) {
	CellStruct ret = CellStruct::Empty;

	// check whether SW can fire and whether it should do so
	// on the preferred target
	if(canFire(info)) {
		if(!prefer(info, ret)) {
			// delegate finding a target, convert to cell
			ret = ConvertToCell(selector(info));
		}
	}

	return ret;
}

template<typename It, typename Valuator>
ObjectClass* GetTargetFirstMax(It first, It last, Valuator value) {
	ObjectClass* pTarget = nullptr;
	int maxValue = 0;

	for(auto it = first; it < last; ++it) {
		if(auto pItem = *it) {
			auto curValue = value(pItem);

			if(curValue > maxValue) {
				pTarget = pItem;
				maxValue = curValue;
			}
		}
	}

	return pTarget;
}

template<typename It, typename Valuator>
ObjectClass* GetTargetAnyMax(It first, It last, Valuator value) {
	DiscreteSelectionClass<ObjectClass*> targets;

	for(auto it = first; it < last; ++it) {
		if(auto pItem = *it) {
			auto curValue = value(pItem);
			targets.Add(pItem, curValue);
		}
	}

	return targets.Select(ScenarioClass::Instance->Random);
}

template<typename It, typename Valuator>
ObjectClass* GetTargetShareAny(It first, It last, Valuator value) {
	DiscreteDistributionClass<ObjectClass*> targets;

	for(auto it = first; it < last; ++it) {
		if(auto pItem = *it) {
			auto curValue = value(pItem);
			targets.Add(pItem, curValue);
		}
	}

	return targets.Select(ScenarioClass::Instance->Random);
}

#pragma endregion

#pragma region CanFire functors

struct CanFireAlways {
	bool operator()(const TargetingInfo& info) const {
		// no restriction
		return true;
	}
};

struct CanFireRequiresEnemy {
	bool operator()(const TargetingInfo& info) const {
		// if we need an enemy, make sure there is one
		return info.Owner->EnemyHouseIndex != -1;
	}
};

#pragma endregion

#pragma region Prefer functors

struct PreferNothing {
	bool operator()(const TargetingInfo& info, CellStruct& target) const {
		// no preferred target
		return false;
	}
};

struct PreferHoldIfOffensive {
	bool operator()(const TargetingInfo& info, CellStruct& target) const {
		// if preferred cell set, don't use it, but don't look further
		if(info.Owner->PreferredTargetCell != CellStruct::Empty) {
			return true;
		}
		return false;
	}
};

struct PreferOffensive {
	bool operator()(const TargetingInfo& info, CellStruct& target) const {
		// if preferred cell set, use it
		if(info.Owner->PreferredTargetCell != CellStruct::Empty) {
			target = info.Owner->PreferredTargetCell;
			return true;
		}
		return false;
	}
};

struct PreferDefensive {
	bool operator()(const TargetingInfo& info, CellStruct& target) const {
		// if preferred cell set, use it
		if(info.Owner->PreferredDefensiveCell2 != CellStruct::Empty) {
			target = info.Owner->PreferredDefensiveCell2;
			return true;
		}
		return false;
	}
};

#pragma endregion

#pragma region Target picking functors

struct PickEmptyTarget {
	CellStruct operator()(const TargetingInfo& info) const {
		return CellStruct::Empty;
	}
};

struct PickIonCannonTarget {
	CellStruct operator()(const TargetingInfo& info) const {
		auto pEnemy = HouseClass::Array->GetItemOrDefault(info.Owner->EnemyHouseIndex);
		if(auto pResult = HouseExt::PickIonCannonTarget(info.Owner, pEnemy,
			info.Super->Type, HouseExt::IonCannonCloakOptions::IgnoreCloaked))
		{
			return pResult->GetMapCoords();
		}
		return CellStruct::Empty;
	}
};

struct PickPreferredTypeOrIonCannon {
	CellStruct operator()(const TargetingInfo& info) const {
		auto pOwner = info.Owner;
		auto type = pOwner->PreferredTargetType;
		if(type == TargetType::Anything) {
			return PickIonCannonTarget()(info);
		} else {
			return pOwner->PickTargetByType(type);
		}
	}
};

#pragma endregion

#pragma region Target Selectors

struct TargetSelector {
	//virtual TargetResult operator()(const TargetingInfo& info) const = 0;
	//virtual ~TargetSelector() = default;
};

struct NuclearMissileTargetSelector final : public TargetSelector {
	TargetResult operator()(const TargetingInfo& info) const {
		return{GetTarget(info, CanFireRequiresEnemy(), PreferOffensive(), PickPreferredTypeOrIonCannon()),
			SWTargetFlags::DisallowEmpty};
	}
};

struct LightningStormTargetSelector final : public TargetSelector {
	TargetResult operator()(const TargetingInfo& info) const {
		return{GetTarget(info, CanAutoFire, PreferOffensive(), PickPreferredTypeOrIonCannon()),
			SWTargetFlags::DisallowEmpty};
	}

private:
	static bool CanAutoFire(const TargetingInfo& info) {
		return !LightningStorm::Active && CanFireRequiresEnemy()(info);
	}
};

struct PsychicDominatorTargetSelector final : public TargetSelector {
	TargetResult operator()(const TargetingInfo& info) const {
		return{GetTarget(info, CanAutoFire, PreferHoldIfOffensive(), FindTargetItem),
			SWTargetFlags::DisallowEmpty};
	}

private:
	static bool CanAutoFire(const TargetingInfo& info) {
		return !PsyDom::Active() && CanFireRequiresEnemy()(info);
	}

	static ObjectClass* FindTargetItem(const TargetingInfo& info) {
		auto it = info.TypeExt->GetPotentialAITargets();
		return GetTargetFirstMax(it.begin(), it.end(), [&](TechnoClass* pTechno) {
			if(pTechno->InLimbo) {
				return -1;
			}

			auto cell = pTechno->GetCell()->MapCoords;

			// new check
			if(!info.CanFireAt(cell)) {
				return -1;
			}

			int value = 0;
			for(size_t i = 0; i < CellSpread::NumCells(3); ++i) {
				auto pCell = MapClass::Instance->GetCellAt(cell + CellSpread::GetCell(i));

				for(auto j = abstract_cast<FootClass*>(pCell->FirstObject); j; j = abstract_cast<FootClass*>(j->NextObject)) {
					if(!info.Owner->IsAlliedWith(j) && !j->IsInAir() && j->CanBePermaMindControlled()) {
						// original game does not consider cloak
						if(j->CloakState != CloakState::Cloaked) {
							++value;
						}
					}
				}
			}

			return value;
		});
	}
};

struct GeneticMutatorTargetSelector final : public TargetSelector {
	TargetResult operator()(const TargetingInfo& info) const {
		return{GetTarget(info, CanFireAlways(), PreferHoldIfOffensive(), FindTargetItem),
			SWTargetFlags::DisallowEmpty};
	}

private:
	static ObjectClass* FindTargetItem(const TargetingInfo& info) {
		auto it = info.TypeExt->GetPotentialAITargets();
		return GetTargetFirstMax(it.begin(), it.end(), [&](TechnoClass* pTechno) {
			if(pTechno->InLimbo) {
				return -1;
			}

			auto cell = pTechno->GetCell()->MapCoords;

			// new check
			if(!info.CanFireAt(cell)) {
				return -1;
			}

			int value = 0;
			for(size_t i = 0; i < CellSpread::NumCells(1); ++i) {
				auto pCell = MapClass::Instance->GetCellAt(cell + CellSpread::GetCell(i));

				for(auto j = pCell->GetInfantry(pTechno->OnBridge); j; j = abstract_cast<InfantryClass*>(j->NextObject)) {
					if(!info.Owner->IsAlliedWith(j) && !j->IsInAir()) {
						// original game does not consider cloak
						if(j->CloakState != CloakState::Cloaked) {
							++value;
						}
					}
				}
			}

			return value;
		});
	}
};

struct ParaDropTargetSelector final : public TargetSelector {
	TargetResult operator()(const TargetingInfo& info) const {
		return{GetTarget(info, CanFireAlways(), PreferOffensive(), FindTargetCoords),
			SWTargetFlags::DisallowEmpty};
	}

private:
	static CellStruct FindTargetCoords(const TargetingInfo& info) {
		auto pOwner = info.Owner;

		static const int SpaceSize = 5;

		auto target = CellStruct::Empty;
		if(pOwner->PreferredTargetType == TargetType::Anything) {
			// if no enemy yet, reinforce own base
			auto pTargetPlayer = HouseClass::Array->GetItemOrDefault(pOwner->EnemyHouseIndex, info.Owner);

			target = MapClass::Instance->Pathfinding_Find(
				pTargetPlayer->GetBaseCenter(), SpeedType::Foot, -1,
				MovementZone::Normal, false, SpaceSize, SpaceSize, false,
				false, false, true, CellStruct::Empty, false, false);

			if(target != CellStruct::Empty) {
				target += CellStruct{SpaceSize / 2, SpaceSize / 2};
			}

		} else {
			target = pOwner->PickTargetByType(pOwner->PreferredTargetType);
		}

		// new check
		if(!info.CanFireAt(target)) {
			return CellStruct::Empty;
		}

		return target;
	}
};

struct ForceShieldTargetSelector final : public TargetSelector {
	TargetResult operator()(const TargetingInfo& info) const {
		return{GetTarget(info, CanFireAlways(), PreferDefensive(), FindTargetCoords),
			SWTargetFlags::DisallowEmpty};
	}

private:
	static CellStruct FindTargetCoords(const TargetingInfo& info) {
		auto pOwner = info.Owner;

		if(pOwner->PreferredDefensiveCell != CellStruct::Empty) {
			if(RulesClass::Instance->AISuperDefenseFrames + pOwner->PreferredDefensiveCellStartTime > Unsorted::CurrentFrame) {
				// new check
				if(info.CanFireAt(pOwner->PreferredDefensiveCell)) {
					return pOwner->PreferredDefensiveCell;
				}
			}
		}

		return CellStruct::Empty;
	}
};

struct NoTargetTargetSelector final : public TargetSelector {
	TargetResult operator()(const TargetingInfo& info) const {
		return{CellStruct::Empty, SWTargetFlags::AllowEmpty};
	}
};

struct BaseTargetSelector final : public TargetSelector {
	TargetResult operator()(const TargetingInfo& info) const {
		// fire at the SW's owner's base cell
		auto cell = info.Owner->GetBaseCenter();

		// new check
		if(!info.CanFireAt(cell)) {
			cell = CellStruct::Empty;
		}

		return{cell, SWTargetFlags::DisallowEmpty};
	}
};

struct EnemyBaseTargetSelector final : public TargetSelector {
	TargetResult operator()(const TargetingInfo& info) const {
		// fire at the owner's enemy base cell
		return{GetTarget(info, CanFireRequiresEnemy(), PreferNothing(), FindTargetCoords),
			SWTargetFlags::DisallowEmpty};
	}

	static CellStruct FindTargetCoords(const TargetingInfo& info) {
		if(auto pEnemy = HouseClass::Array->GetItemOrDefault(info.Owner->EnemyHouseIndex)) {
			auto cell = pEnemy->GetBaseCenter();

			if(info.CanFireAt(cell)) {
				return cell;
			}
		}
		return CellStruct::Empty;
	}
};

struct OffensiveTargetSelector final : public TargetSelector {
	TargetResult operator()(const TargetingInfo& info) const {
		return{GetTarget(info, CanFireRequiresEnemy(), PreferNothing(), PickIonCannonTarget()),
			SWTargetFlags::DisallowEmpty};
	}
};

struct StealthTargetSelector final : public TargetSelector {
	TargetResult operator()(const TargetingInfo& info) const {
		return{GetTarget(info, CanFireAlways(), PreferNothing(), FindTargetItem),
			SWTargetFlags::DisallowEmpty};
	}

private:
	static ObjectClass* FindTargetItem(const TargetingInfo& info) {
		auto pEnemy = nullptr; //HouseClass::Array->GetItemOrDefault(info.Owner->EnemyHouseIndex);
		return HouseExt::PickIonCannonTarget(info.Owner, pEnemy, info.Super->Type,
			HouseExt::IonCannonCloakOptions::RequireCloaked);
	}
};

struct SelfTargetSelector final : public TargetSelector {
	TargetResult operator()(const TargetingInfo& info) const {
		return{GetTarget(info, CanFireAlways(), PreferNothing(), FindTargetCoords),
			SWTargetFlags::DisallowEmpty};
	}

private:
	static CellStruct FindTargetCoords(const TargetingInfo& info) {
		// find the first building providing super
		auto index = info.Super->Type->ArrayIndex;
		const auto& buildings = info.Owner->Buildings;

		// Ares < 0.9 didn't check power
		auto it = std::find_if(buildings.begin(), buildings.end(), [=](BuildingClass* pBld) {
			auto const pExt = BuildingExt::ExtMap.Find(pBld);
			if(pExt->HasSuperWeapon(index, true) && pBld->IsPowerOnline()) {
				auto cell = pBld->GetMapCoords();

				if(info.CanFireAt(cell)) {
					return true;
				}
			}
			return false;
		});

		if(it != buildings.end()) {
			auto pBld = *it;
			auto Offset = CellStruct{pBld->Type->GetFoundationWidth() / 2,
				pBld->Type->GetFoundationHeight(false) / 2};
			return pBld->GetCell()->MapCoords + Offset;
		}

		return CellStruct::Empty;
	}
};

struct MultiMissileTargetSelector final : public TargetSelector {
	// classic TS code: fire at the enemy's most threatening building
	TargetResult operator()(const TargetingInfo& info) const {
		return{GetTarget(info, CanFireRequiresEnemy(), PreferOffensive(), FindTargetItem),
			SWTargetFlags::DisallowEmpty};
	}

private:
	static ObjectClass* FindTargetItem(const TargetingInfo& info) {
		auto pOwner = info.Owner;
		auto pTargetPlayer = HouseClass::Array->GetItem(pOwner->EnemyHouseIndex);

		auto it = info.TypeExt->GetPotentialAITargets(pTargetPlayer);
		return GetTargetFirstMax(it.begin(), it.end(), [&](TechnoClass* pTechno) {
			auto cell = pTechno->GetMapCoords();

			if(!info.CanFireAt(cell)) {
				return -1;
			}

			if(TechnoExt::IsCloaked(pTechno)) {
				return ScenarioClass::Instance->Random.RandomRanged(0, 100);
			}

			return MapClass::Instance->GetThreatPosed(cell, pOwner);
		});
	}
};

struct HunterSeekerTargetSelector final : public TargetSelector {
	// from TS: launch at empty coords only if a house has an enemy
	TargetResult operator()(const TargetingInfo& info) const {
		auto const hasEnemy = CanFireRequiresEnemy{}(info);
		return{CellStruct::Empty, hasEnemy ?
			SWTargetFlags::AllowEmpty : SWTargetFlags::DisallowEmpty};
	}
};

#pragma endregion

TargetResult PickSuperWeaponTarget(SuperClass* pSuper) {
	TargetingInfo info(pSuper);

	// all the different AI targeting modes
	switch(info.TypeExt->SW_AITargetingType) {
	case SuperWeaponAITargetingMode::Nuke:
		return NuclearMissileTargetSelector()(info);
	case SuperWeaponAITargetingMode::LightningStorm:
		return LightningStormTargetSelector()(info);
	case SuperWeaponAITargetingMode::PsychicDominator:
		return PsychicDominatorTargetSelector()(info);
	case SuperWeaponAITargetingMode::ParaDrop:
		return ParaDropTargetSelector()(info);
	case SuperWeaponAITargetingMode::GeneticMutator:
		return GeneticMutatorTargetSelector()(info);
	case SuperWeaponAITargetingMode::ForceShield:
		return ForceShieldTargetSelector()(info);
	case SuperWeaponAITargetingMode::NoTarget:
		return NoTargetTargetSelector()(info);
	case SuperWeaponAITargetingMode::Offensive:
		return OffensiveTargetSelector()(info);
	case SuperWeaponAITargetingMode::Stealth:
		return StealthTargetSelector()(info);
	case SuperWeaponAITargetingMode::Self:
		return SelfTargetSelector()(info);
	case SuperWeaponAITargetingMode::Base:
		return BaseTargetSelector()(info);
	case SuperWeaponAITargetingMode::MultiMissile:
		return MultiMissileTargetSelector()(info);
	case SuperWeaponAITargetingMode::HunterSeeker:
		return HunterSeekerTargetSelector()(info);
	case SuperWeaponAITargetingMode::EnemyBase:
		return EnemyBaseTargetSelector()(info);
	case SuperWeaponAITargetingMode::None:
	default:
		return{CellStruct::Empty, SWTargetFlags::DisallowEmpty};
	}
}

DEFINE_HOOK(5098F0, HouseClass_Update_AI_TryFireSW, 5) {
	GET(HouseClass*, pThis, ECX);

	// this method iterates over every available SW and checks
	// whether it should be fired automatically. the original
	// method would abort if this house is human-controlled.
	bool AIFire = !pThis->ControlledByHuman();

	for(const auto pSuper : pThis->Supers) {
		auto pExt = SWTypeExt::ExtMap.Find(pSuper->Type);

		// fire if this is AI owned or the SW has auto fire set.
		if(pSuper->IsCharged && (AIFire || pExt->SW_AutoFire)) {

			// don't try to fire if we obviously haven't enough money
			if(!pThis->CanTransactMoney(pExt->Money_Amount)) {
				continue;
			}

			auto result = PickSuperWeaponTarget(pSuper);
			if(result.Target != CellStruct::Empty || result.Flags == SWTargetFlags::AllowEmpty) {
				int idxSW = pThis->Supers.FindItemIndex(pSuper);
				pThis->Fire_SW(idxSW, result.Target);
			}
		}
	}

	return 0x509AE7;
}
