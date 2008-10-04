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

