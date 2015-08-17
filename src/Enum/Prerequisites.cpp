#include "Prerequisites.h"

#include "../Ares.h"
#include "../Misc/SavegameDef.h"

#include <ArrayClasses.h>
#include <BuildingTypeClass.h>
#include <HouseClass.h>

Enumerable<GenericPrerequisite>::container_t Enumerable<GenericPrerequisite>::Array;

const char * Enumerable<GenericPrerequisite>::GetMainSection()
{
	return "GenericPrerequisites";
}

GenericPrerequisite::GenericPrerequisite(const char* const pTitle)
	: Enumerable<GenericPrerequisite>(pTitle)
{ }

GenericPrerequisite::~GenericPrerequisite() = default;

void GenericPrerequisite::LoadFromINI(CCINIClass *pINI)
{
	const char *section = GenericPrerequisite::GetMainSection();

	char generalbuf[0x80];

	char name[0x80];
	strcpy_s(name, this->Name);

	_strlwr_s(name);
	name[0] &= ~0x20; // LOL HACK to uppercase a letter

	_snprintf_s(generalbuf, _TRUNCATE, "Prerequisite%s", name);
	Prereqs::Parse(pINI, "General", generalbuf, this->Prereqs);

	Prereqs::Parse(pINI, section, this->Name, this->Prereqs);

	_snprintf_s(generalbuf, _TRUNCATE, "Prerequisite%sAlternate", name);
	Prereqs::ParseAlternate(pINI, "General", generalbuf, this->Alternates);
}

void GenericPrerequisite::LoadFromStream(AresStreamReader &Stm)
{
	Stm
		.Process(this->Prereqs)
		.Process(this->Alternates);
}

void GenericPrerequisite::SaveToStream(AresStreamWriter &Stm)
{
	Stm
		.Process(this->Prereqs)
		.Process(this->Alternates);
}

void GenericPrerequisite::AddDefaults()
{
	FindOrAllocate("POWER");
	FindOrAllocate("FACTORY");
	FindOrAllocate("BARRACKS");
	FindOrAllocate("RADAR");
	FindOrAllocate("TECH");
	FindOrAllocate("PROC");
}

void Prereqs::Parse(CCINIClass *pINI, const char *section, const char *key, DynamicVectorClass<int> &Vec)
{
	if(pINI->ReadString(section, key, "", Ares::readBuffer)) {
		Vec.Clear();

		char* context = nullptr;
		for(char *cur = strtok_s(Ares::readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context)) {
			int idx = BuildingTypeClass::FindIndex(cur);
			if(idx > -1) {
				Vec.AddItem(idx);
			} else {
				idx = GenericPrerequisite::FindIndex(cur);
				if(idx > -1) {
					Vec.AddItem(-1 - idx);
				} else {
					Debug::INIParseFailed(section, key, cur);
				}
			}
		}
	}
}

void Prereqs::ParseAlternate(CCINIClass *pINI, const char *section, const char *key, DynamicVectorClass<TechnoTypeClass*> &Vec)
{
	if(pINI->ReadString(section, key, "", Ares::readBuffer)) {
		Vec.Clear();

		char* context = nullptr;
		for(auto cur = strtok_s(Ares::readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context)) {
			if(auto const pType = TechnoTypeClass::Find(cur)) {
				Vec.AddItem(pType);
			} else if(!INIClass::IsBlank(cur)) {
				Debug::INIParseFailed(section, key, cur);
			}
		}
	}
}

	// helper funcs

bool Prereqs::HouseOwnsGeneric(HouseClass const* const pHouse, int const Index)
{
	// hack - POWER is -1 , this way converts to 0, and onwards
	auto const idxPrereq = static_cast<unsigned int>(-1 - Index);
	if(idxPrereq < GenericPrerequisite::Array.size()) {
		auto const& Prereq = GenericPrerequisite::Array[idxPrereq];
		for(const auto& index : Prereq->Prereqs) {
			if(HouseOwnsSpecific(pHouse, index)) {
				return true;
			}
		}
		for(const auto& pType : Prereq->Alternates) {
			if(pHouse->CountOwnedNow(pType)) {
				return true;
			}
		}
	}
	return false;
}

bool Prereqs::HouseOwnsSpecific(HouseClass const* const pHouse, int const Index)
{
	auto const pType = BuildingTypeClass::Array->Items[Index];
	auto const pPowerup = pType->PowersUpBuilding;
	if(*pPowerup) {
		auto const pCore = BuildingTypeClass::Find(pPowerup);
		if(!pCore || pHouse->OwnedBuildingTypes1.GetItemCount(pCore->ArrayIndex) < 1) {
			return false;
		}
		for(auto const& pBld : pHouse->Buildings) {
			if(pBld->Type != pCore) {
				continue;
			}
			for(const auto& pUpgrade : pBld->Upgrades) {
				if(pUpgrade == pType) {
					return true;
				}
			}
		}
		return false;
	} else {
		return pHouse->OwnedBuildingTypes1.GetItemCount(Index) > 0;
	}
}

bool Prereqs::HouseOwnsPrereq(HouseClass const* const pHouse, int const Index)
{
	return Index < 0
		? HouseOwnsGeneric(pHouse, Index)
		: HouseOwnsSpecific(pHouse, Index)
	;
}

bool Prereqs::HouseOwnsAll(HouseClass const* const pHouse, const DynamicVectorClass<int> &list)
{
	for(const auto& index : list) {
		if(!HouseOwnsPrereq(pHouse, index)) {
			return false;
		}
	}
	return true;
}

bool Prereqs::HouseOwnsAny(HouseClass const* const pHouse, const DynamicVectorClass<int> &list)
{
	for(const auto& index : list) {
		if(HouseOwnsPrereq(pHouse, index)) {
			return true;
		}
	}
	return false;
}

bool Prereqs::ListContainsSpecific(const BTypeIter &List, int const Index)
{
	auto const pItem = BuildingTypeClass::Array->Items[Index];
	return List.contains(pItem);
}

bool Prereqs::ListContainsGeneric(const BTypeIter &List, int const Index)
{
	// hack - POWER is -1 , this way converts to 0, and onwards
	auto const idxPrereq = static_cast<unsigned int>(-1 - Index);
	if(idxPrereq < GenericPrerequisite::Array.size()) {
		const auto& dvc = GenericPrerequisite::Array[idxPrereq]->Prereqs;
		for(const auto& index : dvc) {
			if(ListContainsSpecific(List, index)) {
				return true;
			}
		}
	}
	return false;
}

bool Prereqs::ListContainsPrereq(const BTypeIter &List, int Index)
{
	return Index < 0
		? ListContainsGeneric(List, Index)
		: ListContainsSpecific(List, Index)
	;
}

bool Prereqs::ListContainsAll(const BTypeIter &List, const DynamicVectorClass<int> &Requirements)
{
	for(const auto& index : Requirements) {
		if(!ListContainsPrereq(List, index)) {
			return false;
		}
	}
	return true;
}

bool Prereqs::ListContainsAny(const BTypeIter &List, const DynamicVectorClass<int> &Requirements)
{
	for(const auto& index : Requirements) {
		if(ListContainsPrereq(List, index)) {
			return true;
		}
	}
	return false;
}
