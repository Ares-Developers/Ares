#include "Body.h"
#include "../Building/Body.h"
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

// =============================
// other hooks

DEFINE_HOOK(4F7870, HouseClass_PrereqValidator, 7)
{
	// int (TechnoTypeClass *item, bool BuildLimitOnly, bool includeQueued)
	/* return
		 1 - cameo shown
		 0 - cameo not shown
		-1 - cameo greyed out
	 */

	GET(HouseClass *, pHouse, ECX);
	GET_STACK(TechnoTypeClass *, pItem, 0x4);
	GET_STACK(bool, BuildLimitOnly, 0x8);
	GET_STACK(bool, IncludeQueued, 0xC);

	R->EAX(HouseExt::PrereqValidate(pHouse, pItem, BuildLimitOnly, IncludeQueued));
	return 0x4F8361;
}


DEFINE_HOOK(505360, HouseClass_PrerequisitesForTechnoTypeAreListed, 5)
{
	GET(HouseClass *, pHouse, ECX);

	GET_STACK(TechnoTypeClass *, pItem, 0x4);
	GET_STACK(DynamicVectorClass<BuildingTypeClass *> *, pBuildingsToCheck, 0x8);

	R->EAX(HouseExt::PrerequisitesListed(pBuildingsToCheck, pItem));

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
	GET(HouseClass *, H, ESI);

	if(H->OwnedBuildings) {
		return 0x4F8F87;
	}

	struct EligibleObject {
		HouseClass *H;
		EligibleObject(HouseClass *pHouse) : H(pHouse) {};
		bool operator()(TechnoClass *T) {
			if(T->Owner != H) {
				return false;
			}
			if(!T->InLimbo) {
				return true;
			}
			if(FootClass *F = generic_cast<FootClass *>(T)) {
				return F->ParasiteImUsing != NULL;
			}
			return false;
		}
	} Eligible(H);

	if(GameModeOptionsClass::Instance->ShortGame) {
		for(int i = 0; i < RulesClass::Instance->BaseUnit.Count; ++i) {
			UnitTypeClass *U = RulesClass::Instance->BaseUnit[i];
			if(H->OwnedUnitTypes.GetItemCount(U->ArrayIndex)) {
				return 0x4F8F87;
			}
		}
	} else {
		if(H->OwnedUnitTypes1.Count) {
			for(int i = 0; i < UnitClass::Array->Count; ++i) {
				TechnoClass *T = UnitClass::Array->Items[i];
				if(Eligible(T)) {
					return 0x4F8F87;
				}
			}
		}

		if(H->OwnedInfantryTypes1.Count) {
			for(int i = 0; i < InfantryClass::Array->Count; ++i) {
				TechnoClass *T = InfantryClass::Array->Items[i];
				if(Eligible(T)) {
					return 0x4F8F87;
				}
			}
		}

		if(H->OwnedAircraftTypes1.Count) {
			for(int i = 0; i < AircraftClass::Array->Count; ++i) {
				TechnoClass *T = AircraftClass::Array->Items[i];
				if(Eligible(T)) {
					return 0x4F8F87;
				}
			}
		}

	}

	H->DestroyAll();
	H->AcceptDefeat();

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
		CRT::wcsncpy(H->UIName, str, 20);
	} else {
		CRT::wcsncpy(H->UIName, H->Type->UIName, 20);
	}

	H->UIName[20] = 0;
	return 0x500D0D;
}

DEFINE_HOOK(4F62FF, HouseClass_CTOR_FixNameOverflow, 6)
{
	GET(HouseClass *, H, EBP);
	GET_STACK(HouseTypeClass *, Country, 0x48);

	CRT::wcsncpy(H->UIName, Country->UIName, 20);

	H->UIName[20] = 0;

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
	enum { BuildLimitAllows = 0x5096BD, Absolutely = 0x509671, NoWay = 0x5096F1} CanBuild = NoWay;
	if(TechnoTypeClass * Type = ptrEntry->Type) {
		if(Type->GetFactoryType(true, true, false, Owner)) {
			if(Ares::GlobalControls::AllowBypassBuildLimit[Owner->AIDifficulty]) {
				CanBuild = BuildLimitAllows;
			} else {
				int remainLimit = HouseExt::BuildLimitRemaining(Owner, Type);
				if(remainLimit >= ptrEntry->Amount) {
					CanBuild = BuildLimitAllows;
				} else {
					CanBuild = NoWay;
				}
			}
		} else {
			CanBuild = BuildLimitAllows;
		}
	}
	return CanBuild;
}

DEFINE_HOOK(508EBC, HouseClass_Radar_Update_CheckJammed, 6)
{
	enum {Eligible = 0, Jammed = 0x508F08};
	GET(BuildingClass *, Radar, EAX);
	BuildingExt::ExtData* TheBuildingExt = BuildingExt::ExtMap.Find(Radar);

	return (!TheBuildingExt->RegisteredJammers.empty())
		? Jammed
		: Eligible
	;
}

DEFINE_HOOK(508F91, HouseClass_SpySat_Update_CheckJammed, 6)
{
	enum {Eligible = 0, Jammed = 0x508FF6};
	GET(BuildingClass *, SpySat, ECX);
	BuildingExt::ExtData* TheBuildingExt = BuildingExt::ExtMap.Find(SpySat);

	return (!TheBuildingExt->RegisteredJammers.empty())
		? Jammed
		: Eligible
	;
}
