#include "RulesExt.h"
#include "Prerequisites.h"
#include "ArmorTypes.h"

RulesClassExt::RulesClassData* RulesClassExt::Data;

/*
EXT_CTOR(RulesClass)
{
	RulesClassExt::Global()->Data_Initialized = 0;
}

EXT_DTOR(RulesClass)
{
	delete RulesClassExt::Global();
}
*/

void RulesClassExt::Load(IStream *pStm)
{
	ULONG out;
	pStm->Read(RulesClassExt::Global(), sizeof(RulesClassExt::RulesClassData), &out);
}

void RulesClassExt::Save(IStream *pStm)
{
	ULONG out;
	pStm->Write(RulesClassExt::Global(), sizeof(RulesClassExt::RulesClassData), &out);
}

void RulesClassExt::RulesClassData::Initialize()
{
//	RulesClassExt::RulesClassData *pData = RulesClassExt::Global();
//	RulesClass * pRules = RulesClass::Global();

	GenericPrerequisite::FindOrAllocate("POWER");
	GenericPrerequisite::FindOrAllocate("FACTORY");
	GenericPrerequisite::FindOrAllocate("BARRACKS");
	GenericPrerequisite::FindOrAllocate("RADAR");
	GenericPrerequisite::FindOrAllocate("TECH");
	GenericPrerequisite::FindOrAllocate("PROC");

	ArmorType::FindOrAllocate("none");
	ArmorType::FindOrAllocate("flak");
	ArmorType::FindOrAllocate("plate");
	ArmorType::FindOrAllocate("light");
	ArmorType::FindOrAllocate("medium");
	ArmorType::FindOrAllocate("heavy");
	ArmorType::FindOrAllocate("wood");
	ArmorType::FindOrAllocate("steel");
	ArmorType::FindOrAllocate("concrete");
	ArmorType::FindOrAllocate("special_1");
	ArmorType::FindOrAllocate("special_2");

	this->Data_Initialized = 1;
}

void _stdcall RulesClassExt::Addition(CCINIClass* pINI)
{
}

void _stdcall RulesClassExt::TypeData(CCINIClass* pINI)
{
	RulesClassExt::Global()->Initialize();

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
}
