#include "Body.h"
#include "../Building/Body.h"
#include "../HouseType/Body.h"
#include "../Rules/Body.h"
#include "../Techno/Body.h"
#include "../TechnoType/Body.h"

#include <StringTable.h>
#include <GameModeOptionsClass.h>
#include <BuildingClass.h>
#include <AircraftClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>
#include <ArrayClasses.h>
#include <Helpers/Template.h>
#include <CRT.h>
#include <VoxClass.h>
#include <TiberiumClass.h>
#include <MessageListClass.h>
#include <ScenarioClass.h>

// =============================
// other hooks

DEFINE_HOOK(4F7870, HouseClass_CanBuild, 7)
{
	// int (TechnoTypeClass *item, bool BuildLimitOnly, bool includeQueued)
	/* return
		 1 - cameo shown
		 0 - cameo not shown
		-1 - cameo greyed out
	 */

	GET(HouseClass const* const, pThis, ECX);
	GET_STACK(TechnoTypeClass const* const, pItem, 0x4);
	GET_STACK(bool const, buildLimitOnly, 0x8);
	GET_STACK(bool const, includeInProduction, 0xC);

	R->EAX(HouseExt::PrereqValidate(pThis, pItem, buildLimitOnly, includeInProduction));
	return 0x4F8361;
}


DEFINE_HOOK(505360, HouseClass_PrerequisitesForTechnoTypeAreListed, 5)
{
	//GET(HouseClass *, pHouse, ECX);

	GET_STACK(TechnoTypeClass *, pItem, 0x4);
	GET_STACK(DynamicVectorClass<BuildingTypeClass *> *, pBuildingsToCheck, 0x8);
	GET_STACK(int, pListCount, 0xC);

	auto it = Prereqs::BTypeIter(pBuildingsToCheck->Items, pListCount);
	R->EAX(HouseExt::PrerequisitesListed(it, pItem));

	return 0x505486;
}


/*
 * Attention: This is a rewrite of the "is this house defeated yet?" check that should clear up the
 * "On rare occasions, a player may lose every single unit they have but will not be 'defeated'." issues
 * But! It does so by detecting if you have any objects that are not in limbo, and defeating you if you don't.
 * An exception was added for parasites - they will count as eligible even when in limbo...
 */

DEFINE_HOOK(4F8EBD, HouseClass_Update_HasBeenDefeated, 0)
{
	GET(HouseClass*, pThis, ESI);

	if(pThis->OwnedBuildings) {
		return 0x4F8F87;
	}

	auto Eligible = [pThis](FootClass* pFoot) {
		if(pFoot->Owner != pThis) {
			return false;
		}
		if(!pFoot->InLimbo) {
			return true;
		}
		return pFoot->ParasiteImUsing != nullptr;
	};

	if(GameModeOptionsClass::Instance->ShortGame) {
		for(auto pBaseUnit : RulesClass::Instance->BaseUnit) {
			if(pThis->OwnedUnitTypes[pBaseUnit->ArrayIndex]) {
				return 0x4F8F87;
			}
		}
	} else {
		if(pThis->OwnedUnitTypes1.Total) {
			for(auto pTechno :*UnitClass::Array) {
				if(Eligible(pTechno)) {
					return 0x4F8F87;
				}
			}
		}

		if(pThis->OwnedInfantryTypes1.Total) {
			for(auto pTechno : *InfantryClass::Array) {
				if(Eligible(pTechno)) {
					return 0x4F8F87;
				}
			}
		}

		if(pThis->OwnedAircraftTypes1.Total) {
			for(auto pTechno : *AircraftClass::Array) {
				if(Eligible(pTechno)) {
					return 0x4F8F87;
				}
			}
		}
	}

	pThis->DestroyAll();
	pThis->AcceptDefeat();

	return 0x4F8F87;
}

DEFINE_HOOK(4F645F, HouseClass_CTOR_FixSideIndices, 5)
{
	GET(HouseClass *, pHouse, EBP);
	if(HouseTypeClass * pCountry = pHouse->Type) {
		if(strcmp(pCountry->ID, "Neutral") && strcmp(pCountry->ID, "Special")) {
			pHouse->SideIndex = pCountry->SideIndex;
		}
	}
	return 0x4F6490;
}

DEFINE_HOOK(500CC5, HouseClass_InitFromINI_FixBufferLimits, 6)
{
	GET(HouseClass *, H, EBX);

	if(H->UINameString[0]) {
		const wchar_t *str = StringTable::LoadString(H->UINameString);
		AresCRT::wstrCopy(H->UIName, str);
	} else {
		AresCRT::wstrCopy(H->UIName, H->Type->UIName);
	}

	//dropping this here, should be fine
	auto pHouseExt = HouseExt::ExtMap.Find(H);
	if(auto pType = H->Type->FindParentCountry()) {
		pHouseExt->FactoryOwners_GatheredPlansOf.push_back(pType);
	} else {
		pHouseExt->FactoryOwners_GatheredPlansOf.push_back(H->Type);
	}

	return 0x500D0D;
}

DEFINE_HOOK(4F62FF, HouseClass_CTOR_FixNameOverflow, 6)
{
	GET(HouseClass *, H, EBP);
	GET_STACK(HouseTypeClass *, Country, 0x48);

	AresCRT::wstrCopy(H->UIName, Country->UIName);

	return 0x4F6312;
}

// this is checked right before the TeamClass is instantiated -
// it does not mean the AI will abandon this team if another team wants BuildLimit'ed units at the same time
DEFINE_HOOK(50965E, HouseClass_CanInstantiateTeam, 5)
{
	GET(DWORD, ptrTask, EAX);
	GET(DWORD, ptrOffset, ECX);

	ptrTask += (ptrOffset - 4); // pointer math!
	TaskForceEntryStruct * ptrEntry = reinterpret_cast<TaskForceEntryStruct *>(ptrTask); // evil! but works, don't ask me why

	GET(HouseClass *, Owner, EBP);
	enum { BuildLimitAllows = 0x5096BD, TryToRecruit = 0x509671, NoWay = 0x5096F1} CanBuild = NoWay;
	if(TechnoTypeClass * Type = ptrEntry->Type) {
		if(Type->FindFactory(true, true, false, Owner)) {
			if(Ares::GlobalControls::AllowBypassBuildLimit[Owner->GetAIDifficultyIndex()]) {
				CanBuild = BuildLimitAllows;
			} else {
				int remainLimit = HouseExt::BuildLimitRemaining(Owner, Type);
				if(remainLimit >= ptrEntry->Amount) {
					CanBuild = BuildLimitAllows;
				} else {
					CanBuild = TryToRecruit;
				}
			}
		} else {
			CanBuild = TryToRecruit;
		}
	}
	return CanBuild;
}

DEFINE_HOOK(508EBC, HouseClass_Radar_Update_CheckEligible, 6)
{
	enum {Eligible = 0, Jammed = 0x508F08};
	GET(BuildingClass *, Radar, EAX);
	BuildingExt::ExtData* TheBuildingExt = BuildingExt::ExtMap.Find(Radar);

	return (!TheBuildingExt->RegisteredJammers.empty()
					|| Radar->EMPLockRemaining)
		? Jammed
		: Eligible
	;
}

DEFINE_HOOK(508F91, HouseClass_SpySat_Update_CheckEligible, 6)
{
	enum {Eligible = 0, Jammed = 0x508FF6};
	GET(BuildingClass *, SpySat, ECX);
	BuildingExt::ExtData* TheBuildingExt = BuildingExt::ExtMap.Find(SpySat);

	return (!TheBuildingExt->RegisteredJammers.empty()
					|| SpySat->EMPLockRemaining)
		? Jammed
		: Eligible
	;
}

DEFINE_HOOK(4F8B08, HouseClass_Update_DamageDelay, 6)
{
	GET(HouseClass* const, pThis, ESI);

	// timer used unconditionally to trigger checks
	if(pThis->DamageDelayTimer.Completed()) {
		auto const pType = pThis->Type;
		auto const pTypeExt = HouseTypeExt::ExtMap.Find(pType);
		auto const type_degrades = pTypeExt->Degrades.Get(!pType->MultiplayPassive);

		auto const pExt = HouseExt::ExtMap.Find(pThis);
		auto const degrades = pExt->Degrades.Get(type_degrades);

		auto const pRules = RulesClass::Instance;
		pThis->DamageDelayTimer.Start(static_cast<int>(pRules->DamageDelay * 900));

		// damage is only applied conditionally
		auto const pRulesExt = RulesExt::Global();
		if(degrades && pRulesExt->DegradeEnabled && pThis->HasLowPower()) {
			auto const defaultPercentage = pRulesExt->DegradePercentage.Get(pRules->ConditionYellow);

			for(auto const& pBld : pThis->Buildings) {
				auto const pBldType = pBld->Type;
				auto const pBldTypeExt = BuildingTypeExt::ExtMap.Find(pBldType);

				// get the default amount for this building
				auto const& defaultAmount = pBldType->PowerDrain ?
					pRulesExt->DegradeAmountConsumer : pRulesExt->DegradeAmountNormal;

				// get the damage that applies to this building
				auto damage = pBldTypeExt->DegradeAmount.Get(defaultAmount);
				auto const percentage = pBldTypeExt->DegradePercentage.Get(defaultPercentage);

				if(damage > 0 && pBld->GetHealthPercentage() > percentage) {
					pBld->ReceiveDamage(&damage, 0, pRules->C4Warhead, nullptr, false, false, nullptr);
				}
			}
		}
	}

	// recreate the replaced instructions
	return pThis->IsPlayer() ? 0x4F8B14u : 0x4F8DB1u;
}

// play this annoying message every now and then
DEFINE_HOOK(4F8C23, HouseClass_Update_SilosNeededEVA, 5)
{
	GET(HouseClass* const, pThis, ESI);

	VoxClass::Play("EVA_SilosNeeded");

	if(const CSFText& Message = RulesExt::Global()->MessageSilosNeeded) {
		MessageListClass::Instance->PrintMessage(Message,
			RulesClass::Instance->MessageDelay,	pThis->ColorSchemeIndex);
	}

	return 0;
}

// restored from TS
DEFINE_HOOK(4F9610, HouseClass_GiveTiberium_Storage, A)
{
	GET(HouseClass* const, pThis, ECX);
	GET_STACK(float, amount, 0x4);
	GET_STACK(int const, idxType, 0x8);

	pThis->SiloMoney += Game::F2I(amount * 5.0);

	if(SessionClass::Instance->GameMode == GameMode::Campaign || pThis->CurrentPlayer) {
		// don't change, old values are needed for silo update
		const auto lastStorage = static_cast<int>(pThis->OwnedTiberium.GetTotalAmount());
		const auto lastTotalStorage = pThis->TotalStorage;

		// this is the upper limit for stored tiberium
		if(amount > lastTotalStorage - lastStorage) {
			amount = static_cast<float>(lastTotalStorage - lastStorage);
		}

		// go through all buildings and fill them up until all is in there
		for(auto const& pBuilding : pThis->Buildings) {
			if(amount <= 0.0) {
				break;
			}

			auto const storage = pBuilding->Type->Storage;
			if(pBuilding->IsOnMap && storage > 0) {
				// put as much tiberium into this silo
				auto freeSpace = storage - pBuilding->Tiberium.GetTotalAmount();
				if(freeSpace > 0.0) {
					if(freeSpace > amount) {
						freeSpace = amount;
					}

					pBuilding->Tiberium.AddAmount(freeSpace, idxType);
					pThis->OwnedTiberium.AddAmount(freeSpace, idxType);

					amount -= freeSpace;
				}
			}
		}

		// redraw silos
		pThis->UpdateAllSilos(lastStorage, lastTotalStorage);
	} else {
		// just add the money. this is the only original YR logic
		auto const pTib = TiberiumClass::Array->GetItem(idxType);
		pThis->Balance += Game::F2I(amount * pTib->Value * pThis->Type->IncomeMult);
	}

	return 0x4F9664;
}

// spread tiberium on building destruction. replaces the
// original code, made faster and spilling is now optional.
DEFINE_HOOK(441B30, BuildingClass_Destroy_Refinery, 6)
{
	GET(BuildingClass* const, pThis, ESI);
	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	auto &store = pThis->Tiberium;
	auto &total = pThis->Owner->OwnedTiberium;

	// remove the tiberium contained in this structure from the house's owned
	// tiberium. original code does this one bail at a time, we do bulk.
	if(store.GetTotalAmount() >= 1.0) {
		for(auto i = 0u; i < OwnedTiberiumStruct::Size; ++i) {
			auto const amount = std::ceil(store.GetAmount(i));
			if(amount > 0.0) {
				store.RemoveAmount(amount, i);
				total.RemoveAmount(amount, i);

				// spread bail by bail
				if(pExt->TiberiumSpill) {
					for(auto j = static_cast<int>(amount); j; --j) {
						auto const dist = ScenarioClass::Instance->Random.RandomRanged(256, 768);
						auto const crd = MapClass::GetRandomCoordsNear(pThis->Location, dist, true);

						auto const pCell = MapClass::Instance->GetCellAt(crd);
						pCell->IncreaseTiberium(i, 1);
					}
				}
			}
		}
	}

	return 0x441C0C;
}

DEFINE_HOOK(73E4A2, UnitClass_Mi_Unload_Storage, 6)
{
	// because a value gets pushed to the stack in an inconvenient
	// location, we do our stuff and then mess with the stack so
	// the original functions do nothing any more.
	GET(BuildingClass* const, pBld, EDI);
	GET(int const, idxTiberium, EBP);
	REF_STACK(float, amountRaw, 0x1C);
	REF_STACK(float, amountPurified, 0x34);

	auto const pExt = TechnoExt::ExtMap.Find(pBld);
	pExt->DepositTiberium(amountRaw, amountPurified, idxTiberium);
	amountPurified = amountRaw = 0.0f;

	return 0;
}

DEFINE_HOOK(522D75, InfantryClass_Slave_UnloadAt_Storage, 6)
{
	GET(TechnoClass* const, pBld, EAX);
	GET(int const, idxTiberium, ESI);
	GET(OwnedTiberiumStruct* const, pTiberium, EBP);

	// replaces the inner loop and stores
	// one tiberium type at a time
	auto const amount = pTiberium->GetAmount(idxTiberium);
	pTiberium->RemoveAmount(amount, idxTiberium);

	if(amount > 0.0) {
		auto const pExt = TechnoExt::ExtMap.Find(pBld);
		pExt->RefineTiberium(amount, idxTiberium);

		// register for refinery smoke
		R->BL(1);
	}

	return 0x522E38;
}

// drain affecting only the drained power plant
DEFINE_HOOK(508D32, HouseClass_UpdatePower_LocalDrain1, 5)
{
	GET(HouseClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);

	bool fullDrain = true;

	auto output = pBld->GetPowerOutput();

	if(output > 0) {
		auto pBldTypeExt = TechnoTypeExt::ExtMap.Find(pBld->Type);
		auto pDrainTypeExt = TechnoTypeExt::ExtMap.Find(pBld->DrainingMe->GetTechnoType());

		// local, if any of the participants in the drain is local
		if(pBldTypeExt->Drain_Local || pDrainTypeExt->Drain_Local) {
			fullDrain = false;

			// use the sign to select min or max.
			// 0 means no change (maximum of 0 and a positive value)
			auto limit = [](int value, int limit) {
				if(limit <= 0) {
					return std::max(value, -limit);
				} else {
					return std::min(value, limit);
				}
			};

			// drains the entire output of this building by default
			// (the local output). building has the last word though.
			auto drain = limit(output, pDrainTypeExt->Drain_Amount);
			drain = limit(drain, pBldTypeExt->Drain_Amount);

			if(drain > 0) {
				pThis->PowerOutput -= drain;
			}
		}
	}

	return fullDrain ? 0 : 0x508D37;
}

// sanitize the power output
DEFINE_HOOK(508D4A, HouseClass_UpdatePower_LocalDrain2, 6)
{
	GET(HouseClass*, pThis, ESI);
	if(pThis->PowerOutput < 0) {
		pThis->PowerOutput = 0;
	}
	return 0;
}

DEFINE_HOOK(4FC731, HouseClass_DestroyAll_ReturnStructures, 7)
{
	GET_STACK(HouseClass*, pThis, STACK_OFFS(0x18, 0x8));
	GET(TechnoClass*, pTechno, ESI);

	// do not return structures in campaigns
	if(SessionClass::Instance->GameMode == GameMode::Campaign) {
		return 0;
	}

	// check whether this is a building
	if(auto pBld = abstract_cast<BuildingClass*>(pTechno)) {
		auto pInitialOwner = pBld->InitialOwner;

		// was the building owned by a neutral country?
		if(!pInitialOwner || pInitialOwner->Type->MultiplayPassive) {
			auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);

			auto occupants = pBld->GetOccupantCount();
			auto canReturn = (pInitialOwner != pThis) || occupants > 0;

			if(canReturn && pExt->Returnable.Get(RulesExt::Global()->ReturnStructures)) {

				// this may change owner
				if(occupants) {
					pBld->KillOccupants(nullptr);
				}

				// don't do this when killing occupants already changed owner
				if(pBld->GetOwningHouse() == pThis) {

					// fallback to first civilian side house, same logic SlaveManager uses
					if(!pInitialOwner) {
						pInitialOwner = HouseClass::FindCivilianSide();
					}

					// give to other house and disable
					if(pInitialOwner && pBld->SetOwningHouse(pInitialOwner, false)) {
						pBld->Guard();

						if(pBld->Type->NeedsEngineer) {
							pBld->HasEngineer = false;
							pBld->DisableStuff();
						}

						return 0x4FC770;
					}
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(4F8440, HouseClass_Update_TogglePower, 5)
{
	GET(HouseClass* const, pThis, ECX);
	auto const pExt = HouseExt::ExtMap.Find(pThis);

	pExt->UpdateTogglePower();

	return 0;
}
