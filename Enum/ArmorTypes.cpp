#include "ArmorTypes.h"
#include <Helpers/Macro.h>

DynamicVectorClass<ArmorType*> Enumerable<ArmorType>::Array;

const char * Enumerable<ArmorType>::GetMainSection()
{
	return "ArmorTypes";
}

void ArmorType::LoadFromINI(CCINIClass *pINI)
{
	const char *section = Enumerable<ArmorType>::GetMainSection();

	char buffer[0x40];
	pINI->ReadString(section, this->Name, "", buffer, 0x40);

	this->DefaultIndex = ArmorType::FindIndex(buffer);
	DWORD specialFX;
	WarheadTypeExt::VersesData *VS = &this->DefaultVerses;
	VS->Verses = Conversions::Str2Armor(buffer, &specialFX);
	VS->ForceFire = ((specialFX & verses_ForceFire) == 0);
	VS->Retaliate = ((specialFX & verses_Retaliate) == 0);
	VS->PassiveAcquire = ((specialFX & verses_PassiveAcquire) == 0);
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

	char buffer[0x80];
	char ret[0x20];
	const char *section = pWH->get_ID();

	for(int i = 0; i < Array.Count; ++i) {
		_snprintf(buffer, 64, "Versus.%s", Array[i]->Name);
		if(pINI->ReadString(section, buffer, "", ret, 0x20)) {

			DWORD specialFX = 0x0;
			pData->Verses[i].Verses = Conversions::Str2Armor(ret, &specialFX);

			pData->Verses[i].ForceFire = ((specialFX & verses_ForceFire) != 0);
			pData->Verses[i].Retaliate = ((specialFX & verses_Retaliate) != 0);
			pData->Verses[i].PassiveAcquire = ((specialFX & verses_PassiveAcquire) != 0);
		}

		_snprintf(buffer, 128, "Versus.%s.ForceFire", Array[i]->Name);
		pData->Verses[i].ForceFire = pINI->ReadBool(section, buffer, pData->Verses[i].ForceFire);

		_snprintf(buffer, 128, "Versus.%s.Retaliate", Array[i]->Name);
		pData->Verses[i].Retaliate = pINI->ReadBool(section, buffer, pData->Verses[i].Retaliate);

		_snprintf(buffer, 128, "Versus.%s.PassiveAcquire", Array[i]->Name);
		pData->Verses[i].PassiveAcquire = pINI->ReadBool(section, buffer, pData->Verses[i].PassiveAcquire);
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
	int idx = ArmorType::FindIndex(buf);
	R->set_EAX(idx == -1 ? 0 : idx);

	return 0x475430;
}

// 4B9A52, 5
DEFINE_HOOK(4B9A52, DropshipLoadout_PrintArmor, 5)
{
	R->set_StackVar32(0x4, (DWORD)ArmorType::Array[R->get_EDX()]);
	return 0;
}

