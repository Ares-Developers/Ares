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
		pINI->ReadString(section, Key, "", buffer, 0x40);
//		DEBUGLOG("Reading armor type %s = %s\n", Key, buffer);

		FindOrAllocate(Key)->DefaultIndex = ArmorType::FindIndex(buffer);
		FindOrAllocate(Key)->DefaultVerses = Conversions::Str2Armor(buffer);
//		DEBUGLOG("\tDefaultIndex = %d\n", ArmorType::FindIndex(buffer));
	}
}

void ArmorType::LoadForWarhead(CCINIClass *pINI, WarheadTypeClass* pWH)
{
	WarheadTypeClassExt::WarheadTypeClassData *pData = WarheadTypeClassExt::Ext_p[pWH];

	DEBUGLOG("[%s]Verses: ", pWH->get_ID());
	for(int j = 0; j < pData->Verses.get_Count(); ++j)
	{
		DEBUGLOG("\t%9.6lf", pData->Verses[j]);
	}
	DEBUGLOG("\n ###   1   ###\n");

	while(pData->Verses.get_Count() < Array.get_Count())
	{
		ArmorType *pArmor = Array[pData->Verses.get_Count()];
		int idx = pArmor->DefaultIndex;
		pData->Verses.AddItem(
			idx == -1
				? pArmor->DefaultVerses
				: pData->Verses[idx]
		);
	}

	DEBUGLOG("[%s]Verses: ", pWH->get_ID());
	for(int j = 0; j < pData->Verses.get_Count(); ++j)
	{
		DEBUGLOG("\t%9.6lf", pData->Verses[j]);
	}
	DEBUGLOG("\n ###   2   ###\n");

	char buffer[0x40];
	char ret[0x20];
	const char *section = pWH->get_ID();

//	DEBUGLOG("Reading new verses of [%s]\n", section);
	for(int i = 0; i < Array.get_Count(); ++i)
	{
		sprintf(buffer, "Versus.%s", Array[i]->Title);
//		DEBUGLOG("Reading armor #%d verses (%s)\n", i, buffer);
		if(pINI->ReadString(section, buffer, "", ret, 0x20))
		{
			pData->Verses[i] = Conversions::Str2Armor(ret);
		}
//		DEBUGLOG("Got %lf\n", pData->Verses[i]);
	}

	DEBUGLOG("[%s]Verses: ", pWH->get_ID());
	for(int j = 0; j < pData->Verses.get_Count(); ++j)
	{
		DEBUGLOG("\t%9.6lf", pData->Verses[j]);
	}
	DEBUGLOG("\n ###   3   ###\n");
}

void ArmorType::AddDefaults()
{
	FindOrAllocate("none");
	FindOrAllocate("flak");
	FindOrAllocate("plate");
	FindOrAllocate("light");
	FindOrAllocate("medium");
	FindOrAllocate("heavy");
	FindOrAllocate("wood");
	FindOrAllocate("steel");
	FindOrAllocate("concrete");
	FindOrAllocate("special_1");
	FindOrAllocate("special_2");
}

// 4753F0, 0A
EXPORT_FUNC(ArmorType_FindIndex)
{
	GET(CCINIClass *, pINI, ECX);
	GET(ObjectTypeClass*, O, EBX);
//	DEBUGLOG("Mapping armor of %s (arrlen %d)\n", O->get_ID(), ArmorType::Array.get_Count());
	if(!ArmorType::Array.get_Count())
	{
		ArmorType::AddDefaults();
	}

	const char *Section = (const char *)R->get_StackVar32(0x4);
	const char *Key = (const char *)R->get_StackVar32(0x8);
	int fallback = R->get_StackVar32(0xC);

//	DEBUGLOG("[%s]%s (%d)\n", Section, Key, fallback);
	char buf[0x20];

	const char *curTitle = fallback < ArmorType::Array.get_Count()
		? ArmorType::Array[fallback]->Title
		: "none";

	pINI->ReadString(Section, Key, curTitle, buf, 0x20);
//	DEBUGLOG("Read Armor=%s\n", buf);
	int idx = ArmorType::FindIndex(buf);
//	DEBUGLOG("Mapping armor %s to %d\n", buf, idx);
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

