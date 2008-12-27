#include "RulesExt.h"
#include "Prerequisites.h"
#include "ArmorTypes.h"
#include "RadTypes.h"

RulesClassExt::Struct RulesClassExt::Data;

void RulesClassExt::Struct::Initialize()
{
	GenericPrerequisite::AddDefaults();
	ArmorType::AddDefaults();

	IsInitialized = true;
}

EXPORT RulesClassExt_Load(REGISTERS* R)
{
	IStream* pStm = (IStream*)R->get_StackVar32(0x4);

	ULONG out;
	pStm->Read(RulesClassExt::Global(), sizeof(RulesClassExt::Struct), &out);

	if(RulesClassExt::Global()->SavegameValidation != RULESEXT_VALIDATION)
		Debug::Log("SAVEGAME ERROR: RulesClassExt validation is faulty!\n");

	return 0;
}

EXPORT RulesClassExt_Save(REGISTERS* R)
{
	IStream* pStm = (IStream*)R->get_StackVar32(0x4);

	ULONG out;
	pStm->Write(RulesClassExt::Global(), sizeof(RulesClassExt::Struct), &out);
	return 0;
}

EXPORT RulesClassExt_Addition(REGISTERS* R)
{
	CCINIClass* pINI = (CCINIClass*)R->get_ESI();

// RulesClassExt::Global()->Initialize();

	return 0;
}

EXPORT RulesClassExt_PreAddition(REGISTERS* R)
{
	CCINIClass* pINI = (CCINIClass*)R->get_ESI();

	RulesClassExt::Global()->Initialize();

	return 0;
}

EXPORT RulesClassExt_TypeData(REGISTERS* R)
{
	CCINIClass* pINI = (CCINIClass*)R->get_ESI();

	char buffer[0x24];

	const char section[] = "WeaponTypes";

	int len = pINI->GetKeyCount(section);
	for(int i = 0; i < len; ++i)
	{
		const char *key = pINI->GetKeyName(section, i);
		if(pINI->ReadString(section, key, "", buffer, 0x20) > 0)
		{
			WeaponTypeClass::FindOrAllocate(buffer);
		}
	}

	GenericPrerequisite::LoadFromINIList(pINI);
	ArmorType::LoadFromINIList(pINI);

	RadType::LoadFromINIList(pINI);

	return 0;
}
