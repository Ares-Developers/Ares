#include "ArmorTypes.h"

#include "../Misc/SavegameDef.h"

#include <Conversions.h>

Enumerable<ArmorType>::container_t Enumerable<ArmorType>::Array;

const char * Enumerable<ArmorType>::GetMainSection()
{
	return "ArmorTypes";
}

ArmorType::ArmorType(const char* const pTitle)
	: Enumerable<ArmorType>(pTitle), DefaultIndex(-1)
{ }

ArmorType::~ArmorType() = default;

void ArmorType::LoadFromINI(CCINIClass *pINI)
{
	const char *section = Enumerable<ArmorType>::GetMainSection();

	char buffer[0x40];
	pINI->ReadString(section, this->Name, "", buffer);

	this->DefaultIndex = ArmorType::FindIndex(buffer);
	WarheadTypeExt::VersesData *VS = &this->DefaultVerses;
	VS->Parse(buffer);
}

void ArmorType::LoadFromStream(AresStreamReader &Stm)
{
	Stm
		.Process(this->DefaultIndex)
		.Process(this->DefaultVerses);
}

void ArmorType::SaveToStream(AresStreamWriter &Stm)
{
	Stm
		.Process(this->DefaultIndex)
		.Process(this->DefaultVerses);
}

void ArmorType::LoadForWarhead(CCINIClass *pINI, WarheadTypeClass* pWH)
{
	WarheadTypeExt::ExtData *pData = WarheadTypeExt::ExtMap.Find(pWH);
	if(!pData) {
		return;
	}

	pData->Verses.Reserve(Array.size());

	while(pData->Verses.Count < static_cast<int>(Array.size())) {
		auto& pArmor = Array[pData->Verses.Count];
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

	for(size_t i = 0; i < Array.size(); ++i) {
		_snprintf_s(buffer, _TRUNCATE, "Versus.%s", Array[i]->Name);
		if(pINI->ReadString(section, buffer, "", ret)) {
			pData->Verses[i].Parse(ret);
		}

		_snprintf_s(buffer, _TRUNCATE, "Versus.%s.ForceFire", Array[i]->Name);
		pData->Verses[i].ForceFire = pINI->ReadBool(section, buffer, pData->Verses[i].ForceFire);

		_snprintf_s(buffer, _TRUNCATE, "Versus.%s.Retaliate", Array[i]->Name);
		pData->Verses[i].Retaliate = pINI->ReadBool(section, buffer, pData->Verses[i].Retaliate);

		_snprintf_s(buffer, _TRUNCATE, "Versus.%s.PassiveAcquire", Array[i]->Name);
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

DEFINE_HOOK(4753F0, ArmorType_FindIndex, A)
{
	GET(CCINIClass *, pINI, ECX);
	if(ArmorType::Array.empty()) {
		ArmorType::AddDefaults();
	}

	GET_STACK(const char *, Section, 0x4);
	GET_STACK(const char *, Key, 0x8);
	GET_STACK(int, fallback, 0xC);

	char buf[0x20];

	const char *curTitle = fallback < static_cast<int>(ArmorType::Array.size())
		? ArmorType::Array[fallback]->Name
		: "none";

	pINI->ReadString(Section, Key, curTitle, buf);
	int idx = ArmorType::FindIndex(buf);

	if(idx == -1) {
		Debug::INIParseFailed(Section, Key, buf);
	}

	R->EAX(idx == -1 ? 0 : idx);

	return 0x475430;
}

DEFINE_HOOK(4B9A52, DropshipLoadout_PrintArmor, 5)
{
	R->Stack(0x4, ArmorType::Array[R->EDX()].get());
	return 0;
}
