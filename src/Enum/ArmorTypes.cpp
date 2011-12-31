#include "ArmorTypes.h"

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
	WarheadTypeExt::VersesData *VS = &this->DefaultVerses;
	VS->Parse(buffer);
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

			pData->Verses[i].Parse(ret);
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

DEFINE_HOOK(0x4753F0, ArmorType_FindIndex, 0xA)
{
	GET(CCINIClass *, pINI, ECX);
	if(!ArmorType::Array.Count) {
		ArmorType::AddDefaults();
	}

	GET_STACK(const char *, Section, 0x4);
	GET_STACK(const char *, Key, 0x8);
	GET_STACK(int, fallback, 0xC);

	char buf[0x20];

	const char *curTitle = fallback < ArmorType::Array.Count
		? ArmorType::Array[fallback]->Name
		: "none";

	pINI->ReadString(Section, Key, curTitle, buf, 0x20);
	int idx = ArmorType::FindIndex(buf);
	R->EAX(idx == -1 ? 0 : idx);

	return 0x475430;
}

DEFINE_HOOK(0x4B9A52, DropshipLoadout_PrintArmor, 0x5)
{
	R->Stack(0x4, ArmorType::Array[R->EDX()]);
	return 0;
}

