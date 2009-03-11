#include "..\Ares.h"
#include "Prerequisites.h"

DynamicVectorClass<GenericPrerequisite*> Enumerable<GenericPrerequisite>::Array;

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
	if(pINI->ReadString("General", generalbuf, "", Ares::readBuffer, Ares::readLength)) {
		Prereqs::Parse(Ares::readBuffer, dvc);
	}

	if(pINI->ReadString(section, this->Name, "", Ares::readBuffer, Ares::readLength)) {
		Prereqs::Parse(Ares::readBuffer, dvc);
	}
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

void Prereqs::Parse(char* buffer, DynamicVectorClass<int> *vec)
{
	vec->Clear();
	for(char *cur = strtok(buffer, ","); cur; cur = strtok(NULL, ",")) {
		int idx = BuildingTypeClass::FindIndex(cur);
		if(idx > -1) {
			vec->AddItem(idx);
		} else {
			idx = GenericPrerequisite::FindIndex(cur);
			if(idx > -1) {
				vec->AddItem(-1 - idx);
			}
		}
	}

}

EXPORT_FUNC(RulesClass_TypeData)
{
	CCINIClass *pINI = (CCINIClass *)R->get_StackVar32(0x4);
	GenericPrerequisite::LoadFromINIList(pINI);
	return 0;
}

	// helper funcs

bool Prereqs::HouseOwnsGeneric(HouseClass *pHouse, signed int Index)
{
	Index = - 1 - Index; // hack - POWER is -1 , this way converts to 0, and onwards
	if(Index < GenericPrerequisite::Array.Count) {
		DynamicVectorClass<int> *dvc = &GenericPrerequisite::Array.GetItem(Index)->Prereqs;
		for(int i = 0; i < dvc->Count; ++i) {
			if(HouseOwnsBuilding(pHouse, dvc->GetItem(i))) {
				return true;
			}
		}
		if(Index == 5) { // PROC alternate, man I hate the special cases
			if(pHouse->get_OwnedUnitTypes()->GetItemCount(
				RulesClass::Global()->PrerequisiteProcAlternate->GetArrayIndex()
				))
			{
				return true;
			}
		}
		return false;
	}
	return false;
}

bool Prereqs::HouseOwnsBuilding(HouseClass *pHouse, int Index)
{
	BuildingTypeClass *BType = BuildingTypeClass::Array->GetItem(Index);
	char *powerup = BType->get_PowersUpBuilding();
	if(*powerup) {
		BuildingTypeClass *BCore = BuildingTypeClass::Find(powerup);
		if(pHouse->get_OwnedBuildingTypes1()->GetItemCount(BCore->GetArrayIndex()) < 1) {
			return false;
		}
		for(int i = 0; i < pHouse->get_Buildings()->Count; ++i) {
			BuildingClass *Bld = pHouse->get_Buildings()->GetItem(i);
			if(Bld->Type != BCore) {
				continue;
			}
			for(int j = 0; j < 3; ++j) {
				BuildingTypeClass *Upgrade = Bld->get_Upgrades(j);
				if(Upgrade == BType) {
					return true;
				}
			}
		}
		return false;
	} else {
		return pHouse->get_OwnedBuildingTypes1()->GetItemCount(Index) > 0;
	}
}

bool Prereqs::HouseOwnsPrereq(HouseClass *pHouse, signed int Index)
{
	return Index < 0
	  ? HouseOwnsGeneric(pHouse, Index)
	  : HouseOwnsBuilding(pHouse, Index);
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
