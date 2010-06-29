#include "Body.h"
#include "../Side/Body.h"
#include "../HouseType/Body.h"
#include "../../Enum/Prerequisites.h"
#include "../../Enum/ArmorTypes.h"
#include "../../Enum/RadTypes.h"
#include <GameModeOptionsClass.h>

template<> const DWORD Extension<RulesClass>::Canary = 0x12341234;
RulesExt::ExtData * RulesExt::Data = NULL;

template<> RulesExt::TT *Container<RulesExt>::SavingObject = NULL;
template<> IStream *Container<RulesExt>::SavingStream = NULL;

void RulesExt::Allocate(RulesClass *pThis) {
	if (Data) {
		Remove(pThis);
	}
	Data = new RulesExt::ExtData(RulesExt::ExtData::Canary, pThis);
}

void RulesExt::Remove(RulesClass *pThis) {
	if (Data) {
		delete Data;
		Data = NULL;
	}
}

void RulesExt::LoadFromINIFile(RulesClass *pThis, CCINIClass *pINI) {
	Data->LoadFromINI(pThis, pINI);
}

void RulesExt::LoadBeforeTypeData(RulesClass *pThis, CCINIClass *pINI) {
	GenericPrerequisite::LoadFromINIList(pINI);
	ArmorType::LoadFromINIList(pINI);

	SideExt::ExtMap.LoadAllFromINI(pINI);
	HouseTypeExt::ExtMap.LoadAllFromINI(pINI);

	Data->LoadBeforeTypeData(pThis, pINI);
}

void RulesExt::LoadAfterTypeData(RulesClass *pThis, CCINIClass *pINI) {
	Data->LoadAfterTypeData(pThis, pINI);
}

void RulesExt::ExtData::InitializeConstants(RulesClass *pThis) {
	GenericPrerequisite::AddDefaults();
	ArmorType::AddDefaults();
}

void RulesExt::ExtData::LoadFromINIFile(RulesClass *pThis, CCINIClass *pINI) {
	// earliest loader - can't really do much because nothing else is initialized yet, so lookups won't work
}

void RulesExt::ExtData::LoadBeforeTypeData(RulesClass *pThis, CCINIClass *pINI) {
	const char section[] = "WeaponTypes";

	int len = pINI->GetKeyCount(section);
	for (int i = 0; i < len; ++i) {
		const char *key = pINI->GetKeyName(section, i);
		if (pINI->ReadString(section, key, "", Ares::readBuffer, Ares::readLength)) {
			WeaponTypeClass::FindOrAllocate(Ares::readBuffer);
		}
	}
}

// this should load everything that TypeData is not dependant on
// i.e. InfantryElectrocuted= can go here since nothing refers to it
// but [GenericPrerequisites] have to go earlier because they're used in parsing TypeData
void RulesExt::ExtData::LoadAfterTypeData(RulesClass *pThis, CCINIClass *pINI) {
	RulesExt::ExtData *pData = RulesExt::Global();

	if (!pData) {
		return;
	}

	INI_EX exINI(pINI);

	const char strAV[] = "AudioVisual";

	pData->ElectricDeath.Parse(&exINI, strAV, "InfantryElectrocuted");

	for (int i = 0; i < WeaponTypeClass::Array->Count; ++i) {
		WeaponTypeClass::Array->GetItem(i)->LoadFromINI(pINI);
	}

	pData->EngineerDamage = pINI->ReadDouble("General", "EngineerDamage", pData->EngineerDamage);
	pData->EngineerAlwaysCaptureTech = pINI->ReadBool("General", "EngineerAlwaysCaptureTech", pData->EngineerAlwaysCaptureTech);
	pData->EngineerDamageCursor.Read(&exINI, "General", "EngineerDamageCursor");

	pData->FirestormActiveAnim.Parse(&exINI, strAV, "FirestormActiveAnim");
	pData->FirestormIdleAnim.Parse(&exINI, strAV, "FirestormIdleAnim");
	pData->FirestormGroundAnim.Parse(&exINI, strAV, "FirestormGroundAnim");
	pData->FirestormAirAnim.Parse(&exINI, strAV, "FirestormAirAnim");

	pData->FirestormWH.Parse(&exINI, "General", "FirestormWH");
}

// =============================
// container hooks

DEFINE_HOOK(667A1D, RulesClass_CTOR, 5) {
	GET(RulesClass*, pItem, ESI);

	RulesExt::Allocate(pItem);
	return 0;
}

DEFINE_HOOK(667A30, RulesClass_DTOR, 5) {
	GET(RulesClass*, pItem, ECX);

	RulesExt::Remove(pItem);
	return 0;
}
