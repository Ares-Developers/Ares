#include "ArmorTypes.h"

DynamicVectorClass<ArmorType*> Enumerable<ArmorType>::Array;

const char * Enumerable<ArmorType>::GetMainSection()
{
	return "ArmorTypes";
}

void Enumerable<ArmorType>::LoadFromINIList(CCINIClass *pINI)
{
	const char *section = Enumerable<ArmorType>::GetMainSection();
	int len = pINI->GetKeyCount(section);
	char buffer[0x40];
	for(int i = 0; i < len; ++i)
	{
		const char *Key = pINI->GetKeyName(section, i);
		pINI->ReadString(section, Key, "", buffer, 0x40);

		FindOrAllocate(Key)->DefaultIndex = ArmorType::FindIndex(buffer);
		FindOrAllocate(Key)->DefaultVerses = Conversions::Str2Armor(buffer);
	}
}

void ArmorType::LoadForWarhead(CCINIClass *pINI, WarheadTypeClass* pWH)
{
	WarheadTypeExt::ExtData *pData = WarheadTypeExt::ExtMap.Find(pWH);
	if(!pData) {
		return;
	}

	while(pData->Verses.Count < Array.Count) {
		ArmorType *pArmor = Array[pData->Verses.Count];
		int idx = pArmor->DefaultIndex;
		pData->Verses.AddItem(
			idx == -1
				? pArmor->DefaultVerses
				: pData->Verses[idx]
		);
	}

	char buffer[0x40];
	char ret[0x20];
	const char *section = pWH->get_ID();

	for(int i = 0; i < Array.Count; ++i) {
		_snprintf(buffer, 64, "Versus.%s", Array[i]->Name);
		if(pINI->ReadString(section, buffer, "", ret, 0x20)) {
			pData->Verses[i] = Conversions::Str2Armor(ret);
		}
	}
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
DEFINE_HOOK(4753F0, ArmorType_FindIndex, 0A)
{
	GET(CCINIClass *, pINI, ECX);
//	GET(ObjectTypeClass*, O, EBX);
//	DEBUGLOG("Mapping armor of %s (arrlen %d)\n", O->get_ID(), ArmorType::Array.get_Count());
	if(!ArmorType::Array.Count) {
		ArmorType::AddDefaults();
	}

	const char *Section = (const char *)R->get_StackVar32(0x4);
	const char *Key = (const char *)R->get_StackVar32(0x8);
	int fallback = R->get_StackVar32(0xC);

//	DEBUGLOG("[%s]%s (%d)\n", Section, Key, fallback);
	char buf[0x20];

	const char *curTitle = fallback < ArmorType::Array.Count
		? ArmorType::Array[fallback]->Name
		: "none";

	pINI->ReadString(Section, Key, curTitle, buf, 0x20);
//	DEBUGLOG("Read Armor=%s\n", buf);
	int idx = ArmorType::FindIndex(buf);
//	DEBUGLOG("Mapping armor %s to %d\n", buf, idx);
	R->set_EAX(idx == -1 ? 0 : idx);

	return 0x475430;
}

// 4B9A52, 5
DEFINE_HOOK(4B9A52, DropshipLoadout_PrintArmor, 5)
{
	R->set_StackVar32(0x4, (DWORD)ArmorType::Array[R->get_EDX()]);
	return 0;
}

