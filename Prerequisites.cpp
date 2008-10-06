#include <YRPP.h>
#include "Prerequisites.h"

DynamicVectorClass< GenericPrerequisite* > GenericPrerequisite::Array;

void GenericPrerequisite::LoadFromINIList(CCINIClass *pINI)
{
	char section[] = "GenericPrerequisites";
	int len = pINI->GetKeyCount(section);
	for(int i = 0; i < len; ++i)
	{
		const char *Key = pINI->GetKeyName(section, i);
		FindOrAllocate(Key)->LoadFromINI(pINI);
	}
}

void GenericPrerequisite::LoadFromINI(CCINIClass *pINI)
{
	char section[] = "GenericPrerequisites";

	char buffer[0x200];
	char generalbuf[0x80];

	char name[0x80];
	strcpy(name, this->Name);

	_strlwr(name);
	name[0] &= ~0x20; // LOL HACK to uppercase a letter

	DynamicVectorClass<int> *dvc = &this->Prereqs;

	if(pINI->ReadString("General", generalbuf, "", buffer, 0x200))
	{
		Prereqs::Parse(buffer, dvc);
	}

	if(pINI->ReadString(section, this->Name, "", buffer, 0x200))
	{
		Prereqs::Parse(buffer, dvc);
	}
}

void Prereqs::Parse(char* buffer, DynamicVectorClass<int> *vec)
	{
		vec->Clear();
		char *cur = strtok(buffer, ",");
		while(cur)
		{
			int idx = BuildingTypeClass::FindIndex(cur);
			if(idx > -1)
			{
				vec->AddItem(idx);
			}
			else
			{
				idx = GenericPrerequisite::FindIndex(cur);
				if(idx > -1)
				{
					vec->AddItem(-1 - idx);
				}
			}
			cur = strtok(NULL, ",");
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
	if(Index < GenericPrerequisite::Array.get_Count())
	{
		DynamicVectorClass<int> *dvc = &GenericPrerequisite::Array.GetItem(Index)->Prereqs;
		for(int i = 0; i < dvc->get_Count(); ++i)
		{
			if(HouseOwnsBuilding(pHouse, dvc->GetItem(i)))
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
	if(*powerup)
	{
		BType = BuildingTypeClass::Find(powerup);
		if(pHouse->get_OwnedBuildingTypes1()->GetItemCount(BType->GetArrayIndex()) < 1)
		{
			return false;
		}
		for(int i = 0; i < pHouse->get_Buildings()->get_Count(); ++i)
		{
			BuildingClass *Bld = pHouse->get_Buildings()->GetItem(i);
			for(int j = 0; j < 3; ++j)
			{
				BuildingTypeClass *Upgrade = Bld->get_Upgrades(j);
				if(Upgrade == BType)
				{
					return true;
				}
			}
		}
		return false;
	}
	else
	{
		return pHouse->get_OwnedBuildingTypes1()->GetItemCount(Index) > 0;
	}
}

bool Prereqs::HouseOwnsPrereq(HouseClass *pHouse, signed int Index)
{
	if(Index < 0)
	{
		return HouseOwnsGeneric(pHouse, Index);
	}
	else
	{
		return HouseOwnsBuilding(pHouse, Index);
	}
}

bool Prereqs::HouseOwnsAll(HouseClass *pHouse, DynamicVectorClass<int> *list)
{
	for(int i = 0; i < list->get_Count(); ++i)
	{
		if(!HouseOwnsPrereq(pHouse, list->GetItem(i)))
		{
			return false;
		}
	}
	return true;
}

bool Prereqs::HouseOwnsAny(HouseClass *pHouse, DynamicVectorClass<int> *list)
{
	for(int i = 0; i < list->get_Count(); ++i)
	{
		if(HouseOwnsPrereq(pHouse, list->GetItem(i)))
		{
			return true;
		}
	}
	return false;
}



