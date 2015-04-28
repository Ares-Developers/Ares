#include <ArrayClasses.h>
#include "../Ares.h"
#include "Prerequisites.h"

#include "../Misc/SavegameDef.h"

Enumerable<GenericPrerequisite>::container_t Enumerable<GenericPrerequisite>::Array;

const char * Enumerable<GenericPrerequisite>::GetMainSection()
{
	return "GenericPrerequisites";
}

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
}

void GenericPrerequisite::LoadFromStream(AresStreamReader &Stm)
{
	Stm.Process(this->Prereqs);
}

void GenericPrerequisite::SaveToStream(AresStreamWriter &Stm)
{
	Stm.Process(this->Prereqs);
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

	// helper funcs

bool Prereqs::HouseOwnsGeneric(HouseClass const* const pHouse, signed int const Index)
{
	// hack - POWER is -1 , this way converts to 0, and onwards
	auto const idxPrereq = static_cast<unsigned int>(-1 - Index);
	if(idxPrereq < GenericPrerequisite::Array.size()) {
		const auto& dvc = GenericPrerequisite::Array[idxPrereq]->Prereqs;
		for(const auto& index : dvc) {
			if(HouseOwnsSpecific(pHouse, index)) {
				return true;
			}
		}
		if(idxPrereq == 5) { // PROC alternate, man I hate the special cases
			if(auto ProcAlt = RulesClass::Instance->PrerequisiteProcAlternate) {
				if(pHouse->OwnedUnitTypes.GetItemCount(ProcAlt->GetArrayIndex())) {
					return true;
				}
			}
		}
		return false;
	}
	return false;
}

bool Prereqs::HouseOwnsSpecific(HouseClass const* const pHouse, int const Index)
{
	auto BType = BuildingTypeClass::Array->GetItem(Index);
	const char* powerup = BType->PowersUpBuilding;
	if(*powerup) {
		auto BCore = BuildingTypeClass::Find(powerup);
		if(!BCore || pHouse->OwnedBuildingTypes1.GetItemCount(BCore->GetArrayIndex()) < 1) {
			return false;
		}
		for(const auto& Bld : pHouse->Buildings) {
			if(Bld->Type != BCore) {
				continue;
			}
			for(const auto& Upgrade : Bld->Upgrades) {
				if(Upgrade == BType) {
					return true;
				}
			}
		}
		return false;
	} else {
		return pHouse->OwnedBuildingTypes1.GetItemCount(Index) > 0;
	}
}

bool Prereqs::HouseOwnsPrereq(HouseClass const* const pHouse, signed int const Index)
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

bool Prereqs::ListContainsSpecific(const BTypeIter &List, signed int Index)
{
	auto Target = BuildingTypeClass::Array->GetItem(Index);
	return List.contains(Target);
}

bool Prereqs::ListContainsGeneric(const BTypeIter &List, signed int const Index)
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

bool Prereqs::ListContainsPrereq(const BTypeIter &List, signed int Index)
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
