#include <ArrayClasses.h>
#include "../Ares.h"
#include "Prerequisites.h"

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
	strcpy(name, this->Name);

	_strlwr(name);
	name[0] &= ~0x20; // LOL HACK to uppercase a letter

	DynamicVectorClass<int> *dvc = &this->Prereqs;

	_snprintf(generalbuf, 0x80, "Prerequisite%s", name);
	Prereqs::Parse(pINI, "General", generalbuf, dvc);

	Prereqs::Parse(pINI, section, this->Name, dvc);
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

void Prereqs::Parse(CCINIClass *pINI, const char *section, const char *key, DynamicVectorClass<int> *vec)
{
	if(pINI->ReadString(section, key, "", Ares::readBuffer, Ares::readLength)) {
		vec->Clear();

		char* context = nullptr;
		for(char *cur = strtok_s(Ares::readBuffer, ",", &context); cur; cur = strtok_s(nullptr, ",", &context)) {
			int idx = BuildingTypeClass::FindIndex(cur);
			if(idx > -1) {
				vec->AddItem(idx);
			} else {
				idx = GenericPrerequisite::FindIndex(cur);
				if(idx > -1) {
					vec->AddItem(-1 - idx);
				} else {
					Debug::INIParseFailed(section, key, cur);
				}
			}
		}
	}
}

	// helper funcs

bool Prereqs::HouseOwnsGeneric(HouseClass *pHouse, signed int Index)
{
	Index = - 1 - Index; // hack - POWER is -1 , this way converts to 0, and onwards
	if(Index < GenericPrerequisite::Array.Count) {
		DynamicVectorClass<int> *dvc = &GenericPrerequisite::Array.GetItem(Index)->Prereqs;
		for(int i = 0; i < dvc->Count; ++i) {
			if(HouseOwnsSpecific(pHouse, dvc->GetItem(i))) {
				return true;
			}
		}
		if(Index == 5) { // PROC alternate, man I hate the special cases
			if(UnitTypeClass *ProcAlt = RulesClass::Instance->PrerequisiteProcAlternate) {
				if(pHouse->OwnedUnitTypes.GetItemCount(ProcAlt->GetArrayIndex())) {
					return true;
				}
			}
		}
		return false;
	}
	return false;
}

bool Prereqs::HouseOwnsSpecific(HouseClass *pHouse, int Index)
{
	BuildingTypeClass *BType = BuildingTypeClass::Array->GetItem(Index);
	char *powerup = BType->PowersUpBuilding;
	if(*powerup) {
		BuildingTypeClass *BCore = BuildingTypeClass::Find(powerup);
		if(pHouse->OwnedBuildingTypes1.GetItemCount(BCore->GetArrayIndex()) < 1) {
			return false;
		}
		for(int i = 0; i < pHouse->Buildings.Count; ++i) {
			BuildingClass *Bld = pHouse->Buildings.GetItem(i);
			if(Bld->Type != BCore) {
				continue;
			}
			for(int j = 0; j < 3; ++j) {
				BuildingTypeClass *Upgrade = Bld->Upgrades[j];
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

bool Prereqs::HouseOwnsPrereq(HouseClass *pHouse, signed int Index)
{
	return Index < 0
		? HouseOwnsGeneric(pHouse, Index)
		: HouseOwnsSpecific(pHouse, Index)
	;
}

bool Prereqs::HouseOwnsAll(HouseClass *pHouse, DynamicVectorClass<int> *list)
{
	for(int i = 0; i < list->Count; ++i) {
		if(!HouseOwnsPrereq(pHouse, list->GetItem(i))) {
			return false;
		}
	}
	return true;
}

bool Prereqs::HouseOwnsAny(HouseClass *pHouse, DynamicVectorClass<int> *list)
{
	for(int i = 0; i < list->Count; ++i) {
		if(HouseOwnsPrereq(pHouse, list->GetItem(i))) {
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

bool Prereqs::ListContainsGeneric(const BTypeIter &List, signed int Index)
{
	Index = - 1 - Index; // hack - POWER is -1 , this way converts to 0, and onwards
	if(Index < GenericPrerequisite::Array.Count) {
		DynamicVectorClass<int> *dvc = &GenericPrerequisite::Array.GetItem(Index)->Prereqs;
		for(int i = 0; i < dvc->Count; ++i) {
			if(ListContainsSpecific(List, dvc->GetItem(i))) {
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

bool Prereqs::ListContainsAll(const BTypeIter &List, DynamicVectorClass<int> *Requirements)
{
	for(int i = 0; i < Requirements->Count; ++i) {
		if(!ListContainsPrereq(List, Requirements->GetItem(i))) {
			return false;
		}
	}
	return true;
}

bool Prereqs::ListContainsAny(const BTypeIter &List, DynamicVectorClass<int> *Requirements)
{
	for(int i = 0; i < Requirements->Count; ++i) {
		if(ListContainsPrereq(List, Requirements->GetItem(i))) {
			return true;
		}
	}
	return false;
}

