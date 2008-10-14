#include "ArmorTypes.h"

DynamicVectorClass<ArmorType*> ArmorType::Array;
void ArmorType::LoadFromINIList(CCINIClass *pINI)
{
	char section[] = "ArmorTypes";
	int len = pINI->GetKeyCount(section);
	char buffer[0x40];
	for(int i = 0; i < len; ++i)
	{
		const char *Key = pINI->GetKeyName(section, i);

		pINI->ReadString(section, Key, buffer, "none", 0x40);
		FindOrAllocate(Key)->DefaultIndex = ArmorType::FindIndex(buffer);
	}
}

void ArmorType::LoadForWarhead(CCINIClass *pINI, WarheadTypeClass* pWH)
{
	WarheadTypeClassExt::WarheadTypeClassData *pData = WarheadTypeClassExt::Ext_p[pWH];

	char section[] = "ArmorTypes";
	while(pData->Verses.get_Count() < Array.get_Count())
	{
		int idx = Array[pData->Verses.get_Count()]->DefaultIndex;
		pData->Verses.AddItem(
			idx == -1
				? 1.0
				: pData->Verses[idx]
		);
	}

	char buffer[0x40];
	for(int i = 0; i < Array.get_Count(); ++i)
	{
		sprintf(buffer, "Versus.%s", Array[i]->Title);
		pData->Verses[i] = pINI->ReadDouble(section, buffer, pData->Verses[i]);
	}
}

// 4753F0, 0A
EXPORT_FUNC(ArmorType_FindIndex)
{
	GET(CCINIClass *, pINI, ECX);
	const char *Section = (const char *)R->get_StackVar32(0x4);
	const char *Key = (const char *)R->get_StackVar32(0x8);
	int fallback = R->get_StackVar32(0xC);

	char buf[0x20];

	pINI->ReadString(Section, Key, buf, ArmorType::Array[fallback]->Title, 0x80);
	int idx = ArmorType::FindIndex(buf);
	if(idx == -1)
	{
		R->set_EAX(0);
	}
	else
	{
		R->set_EAX(idx);
	}

	return 0x475430;
}

// 4B9A52, 5
EXPORT_FUNC(DropshipLoadout_PrintArmor)
{
	R->set_StackVar32(0x4, (DWORD)ArmorType::Array[R->get_EDX()]);
	return 0;
}

