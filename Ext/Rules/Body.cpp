#include "Body.h"
#include "..\Side\Body.h"
#include "..\HouseType\Body.h"
#include "..\..\Enum\Prerequisites.h"
#include "..\..\Enum\ArmorTypes.h"
#include "..\..\Enum\RadTypes.h"

const DWORD Extension<RulesClass>::Canary = 0x12341234;
RulesExt::ExtData * RulesExt::Data = NULL;

RulesExt::TT *Container<RulesExt>::SavingObject = NULL;
IStream *Container<RulesExt>::SavingStream = NULL;

void RulesExt::Allocate(RulesClass *pThis)
{
	if(Data) {
		Remove(pThis);
	}
	Data = new RulesExt::ExtData(RulesExt::ExtData::Canary);
}

void RulesExt::Remove(RulesClass *pThis)
{
	if(Data) {
		delete Data;
		Data = NULL;
	}
}

void RulesExt::LoadFromINI(RulesClass *pThis, CCINIClass *pINI)
{
	Data->LoadFromINI(pThis, pINI);
}

void RulesExt::LoadBeforeTypeData(RulesClass *pThis, CCINIClass *pINI)
{
	GenericPrerequisite::LoadFromINIList(pINI);
	ArmorType::LoadFromINIList(pINI);

	SideExt::ExtMap.LoadAllFromINI(pINI);
	HouseTypeExt::ExtMap.LoadAllFromINI(pINI);

	Data->LoadBeforeTypeData(pThis, pINI);
}

void RulesExt::LoadAfterTypeData(RulesClass *pThis, CCINIClass *pINI)
{
	Data->LoadAfterTypeData(pThis, pINI);
}


void RulesExt::ExtData::InitializeConstants(RulesClass *pThis)
{
	GenericPrerequisite::AddDefaults();
	ArmorType::AddDefaults();

	this->_Initialized = is_Constanted;
}

void RulesExt::ExtData::Initialize(RulesClass *pThis)
{
	this->_Initialized = is_Inited;
}

void RulesExt::ExtData::LoadFromINI(RulesClass *pThis, CCINIClass *pINI)
{
	RulesExt::ExtData *pData = RulesExt::Global();

	if(!pData) {
		return;
	}

	if(this->_Initialized == is_Blank) {
		this->InitializeConstants(pThis);
	}
	if(this->_Initialized == is_Constanted && RulesClass::Initialized) {
		this->InitializeRuled(pThis);
	}

	if(this->_Initialized == is_Ruled) {
		this->Initialize(pThis);
	}

	if(this->_Initialized != is_Inited) {
		return;
	}

	// this is where the main reading should happen
	const char section[] = "AudioVisual";

	PARSE_BUF();
	PARSE_ANIM("InfantryElectrocuted", pData->ElectricDeath);
}

void RulesExt::ExtData::LoadBeforeTypeData(RulesClass *pThis, CCINIClass *pINI)
{
	const char section[] = "WeaponTypes";

	int len = pINI->GetKeyCount(section);
	for(int i = 0; i < len; ++i) {
		const char *key = pINI->GetKeyName(section, i);
		if(pINI->ReadString(section, key, "", Ares::readBuffer, Ares::readLength)) {
			WeaponTypeClass::FindOrAllocate(Ares::readBuffer);
		}
	}
}

void RulesExt::ExtData::LoadAfterTypeData(RulesClass *pThis, CCINIClass *pINI)
{
	RulesExt::ExtData *pData = RulesExt::Global();

	if(!pData) {
		return;
	}

	if(this->_Initialized == is_Constanted && RulesClass::Initialized) {
		this->InitializeRuled(pThis);
	}

	if(this->_Initialized == is_Ruled) {
		this->Initialize(pThis);
	}

	if(this->_Initialized != is_Inited) {
		return;
	}

	for(int i = 0; i < WeaponTypeClass::Array->Count; ++i) {
		WeaponTypeClass::Array->Items[i]->LoadFromINI(pINI);
	}
}

// =============================
// container hooks

DEFINE_HOOK(667A1D, RulesClass_CTOR, 5)
{
	GET(RulesClass*, pItem, ESI);

	RulesExt::Allocate(pItem);
	return 0;
}

DEFINE_HOOK(667A30, RulesClass_DTOR, 5)
{
	GET(RulesClass*, pItem, ECX);

	RulesExt::Remove(pItem);
	return 0;
}
