#include "Body.h"
#include "../Building/Body.h"
//include "Side.h"
#include "../../Enum/Prerequisites.h"
#include "../../Misc/Debug.h"
#include "../Rules/Body.h"
#include "../../Utilities/TemplateDef.h"

#include <HouseTypeClass.h>

// =============================
// other hooks

DEFINE_HOOK(732D10, TacticalClass_CollectSelectedIDs, 5)
{
	// create in dll. all internal memory will be allocated on the game's heap
	// but as our vtable is used, the virtual destructor will free this instance
	// from the dll's heap.
	auto pNames = DLLCreate<DynamicVectorClass<const char*>>();

	auto Add = [pNames](TechnoTypeClass* pType) {
		if(auto pExt = TechnoTypeExt::ExtMap.Find(pType)) {
			const char* id = pExt->GetSelectionGroupID();

			if(std::none_of(pNames->begin(), pNames->end(), [id](const char* pID) {
				return !_strcmpi(pID, id);
			})) {
				pNames->AddItem(id);
			}
		}
	};

	bool useDeploy = RulesExt::Global()->TypeSelectUseDeploy;

	for(auto pObject : *ObjectClass::CurrentObjects) {
		// add this object's id used for grouping
		if(TechnoTypeClass* pType = pObject->GetTechnoType()) {
			Add(pType);

			// optionally do the same the original game does, but support the new grouping feature.
			if(useDeploy) {
				if(pType->DeploysInto) {
					Add(pType->DeploysInto);
				}
				if(pType->UndeploysInto) {
					Add(pType->UndeploysInto);
				}
			}
		}
	}

	R->EAX(pNames);
	return 0x732FE1;
}

DEFINE_HOOK(7327AA, TechnoClass_PlayerOwnedAliveAndNamed_GroupAs, 8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(const char*, pID, EDI);

	bool ret = TechnoTypeExt::HasSelectionGroupID(pThis->GetTechnoType(), pID);

	R->EAX<int>(ret);
	return 0x7327B2;
}

DEFINE_HOOK_AGAIN(4ABD9D, DisplayClass_LeftMouseButtonUp_GroupAs, A)
DEFINE_HOOK_AGAIN(4ABE58, DisplayClass_LeftMouseButtonUp_GroupAs, A)
DEFINE_HOOK(4ABD6C, DisplayClass_LeftMouseButtonUp_GroupAs, A)
{
	GET(ObjectClass*, pThis, ESI);
	R->EAX(TechnoTypeExt::GetSelectionGroupID(pThis->GetType()));
	return R->Origin() + 13;
}

DEFINE_HOOK(6DA665, sub_6DA5C0_GroupAs, A)
{
	GET(ObjectClass*, pThis, ESI);
	R->EAX(TechnoTypeExt::GetSelectionGroupID(pThis->GetType()));
	return R->Origin() + 13;
}

/*
A_FINE_HOOK(5F8480, ObjectTypeClass_Load3DArt, 6)
{
	GET(ObjectTypeClass *, O, ESI);
	if(O->WhatAmI() == abs_UnitType) {
		TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(reinterpret_cast<TechnoTypeClass*>(O));
		if(pData->WaterAlt) {
			return 0x5F848C;
		}
	}
	return 0;
}
*/

DEFINE_HOOK(715320, TechnoTypeClass_LoadFromINI_EarlyReader, 6)
{
	GET(CCINIClass *, pINI, EDI);
	GET(TechnoTypeClass *, pType, EBP);

	INI_EX exINI(pINI);
	TechnoTypeExt::ExtData *pData = TechnoTypeExt::ExtMap.Find(pType);

	pData->WaterImage.Read(exINI, pType->ID, "WaterImage");

	return 0;
}


DEFINE_HOOK(5F79B0, TechnoTypeClass_FindFactory, 6)
{
	enum { Eligible = 0x5F79C7, Ineligible = 0x5F7A57 };

	GET(TechnoTypeClass *, pProduction, EDI);
	GET(BuildingClass *, pFactory, ESI);

	auto pData = TechnoTypeExt::ExtMap.Find(pProduction);

	return (pData->CanBeBuiltAt(pFactory->Type))
		? Eligible
		: Ineligible
	;
}

DEFINE_HOOK(4444E2, BuildingClass_KickOutUnit_FindAlternateKickout, 6)
{
	GET(BuildingClass *, Src, ESI);
	GET(BuildingClass *, Tst, EBP);
	GET(TechnoClass *, Production, EDI);

	auto pData = TechnoTypeExt::ExtMap.Find(Production->GetTechnoType());

	if(Src != Tst
	 && Tst->GetCurrentMission() == Mission::Guard
	 && Tst->Type->Factory == Src->Type->Factory
	 && Tst->Type->Naval == Src->Type->Naval
	 && pData->CanBeBuiltAt(Tst->Type)
	 && !Tst->Factory)
	{
		return 0x44451F;
	}

	return 0x444508;
}

DEFINE_HOOK(444DBC, BuildingClass_KickOutUnit_Infantry, 5) {
	GET(TechnoClass*, Production, EDI);
	GET(BuildingClass*, Factory, ESI);

	// turn it off
	--Unsorted::IKnowWhatImDoing;

	auto pFactoryData = BuildingExt::ExtMap.Find(Factory);
	pFactoryData->KickOutClones(Production);

	// turn it back on so the game can turn it off again
	++Unsorted::IKnowWhatImDoing;

	return 0;
}

DEFINE_HOOK(4445F6, BuildingClass_KickOutUnit_Clone_NonNavalUnit, 5) {
	GET(TechnoClass*, Production, EDI);
	GET(BuildingClass*, Factory, ESI);

	// turn it off
	--Unsorted::IKnowWhatImDoing;

	auto pFactoryData = BuildingExt::ExtMap.Find(Factory);
	pFactoryData->KickOutClones(Production);

	// turn it back on so the game can turn it off again
	++Unsorted::IKnowWhatImDoing;

	return 0x444971;
}

DEFINE_HOOK(44441A, BuildingClass_KickOutUnit_Clone_NavalUnit, 6) {
	GET(TechnoClass*, Production, EDI);
	GET(BuildingClass*, Factory, ESI);

	auto pFactoryData = BuildingExt::ExtMap.Find(Factory);
	pFactoryData->KickOutClones(Production);

	return 0;
}

DEFINE_HOOK(4449DF, BuildingClass_KickOutUnit_PreventClone, 6)
{
	return 0x444A53;
}
